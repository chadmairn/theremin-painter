/*
 * THEREMIN PAINTER - Arduino side
 * ===============================
 * Reads three controls and streams them to the web app over USB serial:
 *   - HC-SR04 ultrasonic sensor  -> hand distance in cm
 *   - Potentiometer              -> paint color (0-1023)
 *   - Push button                -> stamp trigger (0/1, debounced)
 *
 * It also drives the physical feedback on the breadboard:
 *   - RGB LED glows the currently selected paint color
 *   - LCD1602 shows the current musical note + color name
 *
 * Serial protocol (9600 baud, 10 lines/sec):
 *   D:42,P:512,B:0
 *   D = distance in cm (0 means "no hand detected")
 *   P = potentiometer 0-1023
 *   B = button 0/1
 *
 * Libraries needed: LiquidCrystal (comes built into the Arduino IDE,
 *                   no install required).
 */

#include <LiquidCrystal.h>

// ---------------------------------------------------------------------------
// PIN MAP - change here if you wire things differently, and keep the wiring
// guide + web app in sync with these numbers.
// ---------------------------------------------------------------------------
#define TRIG_PIN    9    // HC-SR04 trigger
#define ECHO_PIN    10   // HC-SR04 echo
#define POT_PIN     A0   // potentiometer wiper
#define BUTTON_PIN  2    // push button (uses external 10k pull-DOWN resistor)

#define LED_R       3    // RGB LED red   (PWM pin, through 220 ohm resistor)
#define LED_G       5    // RGB LED green (PWM pin, through 220 ohm resistor)
#define LED_B       6    // RGB LED blue  (PWM pin, through 220 ohm resistor)

#define LCD_RS      7    // LCD1602 RS
#define LCD_EN      8    // LCD1602 Enable
#define LCD_D4      11   // LCD1602 data bit 4
#define LCD_D5      12   // LCD1602 data bit 5
#define LCD_D6      13   // LCD1602 data bit 6
#define LCD_D7      4    // LCD1602 data bit 7
// NOTE: pins 0 and 1 are left alone - they carry the USB serial to the PC.

// ---------------------------------------------------------------------------
// MUSICAL MAP - C major pentatonic, two octaves, 10 steps.
// Distance 5..60 cm maps to steps 0..9. KEEP IN SYNC with theremin-painter.html
// (the SCALE_FREQS / SCALE_NAMES arrays there).
// ---------------------------------------------------------------------------
const float NOTE_FREQS[10] = {
  261.63,  // 0: C4
  293.66,  // 1: D4
  329.63,  // 2: E4
  392.00,  // 3: G4
  440.00,  // 4: A4
  523.25,  // 5: C5
  587.33,  // 6: D5
  659.25,  // 7: E5
  783.99,  // 8: G5
  880.00   // 9: A5
};
const char* NOTE_NAMES[10] = {
  "C4","D4","E4","G4","A4","C5","D5","E5","G5","A5"
};

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ---- timing / debounce state ----
unsigned long lastSendMs = 0;      // last serial line sent
unsigned long lastLcdMs  = 0;      // last LCD refresh
int  buttonState       = LOW;      // debounced button value we report
int  lastRawButton     = LOW;      // raw reading from the pin
unsigned long lastDebounceMs = 0;
const unsigned long DEBOUNCE_MS = 40;

// ---------------------------------------------------------------------------
// Read the HC-SR04 and return distance in cm. Returns 0 if nothing is in range.
// ---------------------------------------------------------------------------
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);     // 10 microsecond trigger pulse
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn waits for the echo; 30000 us timeout ~= 5 meters max
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 0;      // timed out -> no hand detected
  return duration / 58;             // sound travels ~58 us per cm (round trip)
}

// ---------------------------------------------------------------------------
// Map a distance to a pentatonic step 0..9, or -1 when the hand is out of range.
// ---------------------------------------------------------------------------
int distanceToStep(long cm) {
  if (cm < 4 || cm > 70) return -1;
  long c = constrain(cm, 5L, 60L);
  return (int)(((c - 5) * 9.0 / 55.0) + 0.5);
}

// ---------------------------------------------------------------------------
// Convert a hue (0-359) to RGB brightness (0-255 each). Full saturation.
// ---------------------------------------------------------------------------
void hueToRgb(int hue, int &r, int &g, int &b) {
  hue = hue % 360;
  if (hue < 0) hue += 360;
  int sector = hue / 60;                 // 0..5
  int frac   = (hue % 60) * 255 / 60;   // 0..255 inside the sector
  int q = 255 - frac;
  switch (sector) {
    case 0:  r = 255; g = frac; b = 0;   break;  // red -> yellow
    case 1:  r = q;   g = 255;  b = 0;   break;  // yellow -> green
    case 2:  r = 0;   g = 255;  b = frac; break; // green -> cyan
    case 3:  r = 0;   g = q;    b = 255;  break; // cyan -> blue
    case 4:  r = frac; g = 0;   b = 255;  break; // blue -> magenta
    default: r = 255; g = 0;    b = q;    break;  // magenta -> red
  }
}

// ---------------------------------------------------------------------------
// Friendly color name for the LCD. KEEP IN SYNC with colorName() in the HTML.
// ---------------------------------------------------------------------------
const char* colorName(int hue) {
  if (hue < 15  || hue >= 345) return "Red";
  if (hue < 45)                return "Orange";
  if (hue < 75)                return "Yellow";
  if (hue < 150)               return "Green";
  if (hue < 195)               return "Cyan";
  if (hue < 255)               return "Blue";
  if (hue < 285)               return "Purple";
  return "Pink";
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);   // external 10k pull-DOWN resistor on the breadboard
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Theremin Painter");
  lcd.setCursor(0, 1);
  lcd.print("warming up...");
  delay(1500);
  lcd.clear();
}

void loop() {
  // ---- debounced button ----
  int raw = digitalRead(BUTTON_PIN);
  if (raw != lastRawButton) {
    lastDebounceMs = millis();
    lastRawButton = raw;
  }
  if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
    buttonState = raw;
  }

  unsigned long now = millis();

  // ---- stream sensor values 10x per second ----
  if (now - lastSendMs >= 100) {
    lastSendMs = now;
    long dist = readDistanceCM();
    int  pot  = analogRead(POT_PIN);
    Serial.print("D:");
    Serial.print(dist);
    Serial.print(",P:");
    Serial.print(pot);
    Serial.print(",B:");
    Serial.println(buttonState);
  }

  // ---- physical feedback: RGB LED + LCD, refreshed 4x per second ----
  // (LCD is slow - updating it at full speed causes flicker.)
  if (now - lastLcdMs >= 250) {
    lastLcdMs = now;
    long dist = readDistanceCM();
    int  pot  = analogRead(POT_PIN);
    int  hue  = map(pot, 0, 1023, 0, 359);

    int r, g, b;
    hueToRgb(hue, r, g, b);
    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);

    int step = distanceToStep(dist);

    lcd.setCursor(0, 0);
    if (step >= 0) {
      lcd.print("Note: ");
      lcd.print(NOTE_NAMES[step]);
      lcd.print("        ");            // spaces wipe leftover characters
    } else {
      lcd.print("Note: --         ");
    }
    lcd.setCursor(0, 1);
    lcd.print("Color: ");
    const char* cn = colorName(hue);
    lcd.print(cn);
    // pad with spaces to exactly 16 columns so old text is erased
    // (printing past column 16 would wrap around and corrupt line 1!)
    int used = 7 + strlen(cn);
    for (int i = used; i < 16; i++) lcd.print(" ");
  }
}
