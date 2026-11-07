# Theremin Painter: Class Demo Notes (Beginner)

Keep it simple. The story is: your hand makes music, the knob makes color, the button makes art, and the painting plays itself back.

## The 30-second pitch

Wave your hand over a sensor to play notes like a theremin. Turn a knob to paint the music in color. Press a button to stamp each note onto the canvas. Press play, and your painting becomes a sequencer that performs the melody back while you improvise over it. One Arduino, one sensor, one web page.

## 1. The breadboard: how things are routed and why

Everything shares two rails: red is 5V, blue is ground. Every part connects to those rails for power, and to one Arduino pin for its signal.

- **HC-SR04 ultrasonic sensor:** Trig to D9, Echo to D10. It sends an inaudible click and listens for the echo. The longer the echo takes, the farther your hand is. Time becomes distance.
- **Potentiometer (the knob):** outer legs to 5V and ground, middle leg to A0. It is a voltage divider: the middle leg picks a voltage between 0 and 5V depending on knob position, and the Arduino reads it as a number from 0 to 1023. This is the color control.
- **Button:** one side to 5V, the other side to D2, plus a 10k resistor from the D2 row to ground. That resistor is the pull-down: when the button is open, it holds the pin at a clean 0 instead of letting it float and pick up noise. A floating pin lies.
- **RGB LED:** four legs in four consecutive rows, one leg per row. The long leg is ground, jumper to the blue rail. Each color leg gets its own 220 ohm resistor to an empty row, then a jumper to D3 (red), D5 (green), D6 (blue). The Arduino dims each color by pulsing it fast (PWM), and mixing the three makes any color.
- **LCD (optional):** shows the current note name and color. The code is already in the sketch; it is just 16 wires.

Golden rule for the board: one leg per row. A resistor must bridge two different rows, or it does nothing.

## 2. Arduino sketch highlights

The sketch does the same thing ten times a second:

1. Read the sensor (hand distance in cm).
2. Read the knob (0-1023).
3. Read the button (pressed or not).
4. Update the LED and the LCD.
5. Print one line to the computer: D:42,P:512,B:0 (distance, pot, button).

Two translations happen inside the sketch, both with the map() function:

- Distance 5 to 60 cm becomes ten notes of a C major pentatonic scale. Closer is higher.
- Knob 0 to 1023 becomes a color hue 0 to 359, the full rainbow.

That is the whole job of the sketch: read three inputs, translate them, drive the LED, and report the numbers.

## 3. The webpage: how it all connects

One HTML file, no server, no install. Open it in Chrome or Edge (Firefox and Safari cannot do this part). Three pieces of web tech:

- **Web Serial:** the page opens the USB serial port and reads the lines the Arduino prints. The sketch does not know the browser exists; it just prints, and the page listens. Only one program can hold the port at a time, so close the Arduino Serial Monitor before connecting.
- **p5.js:** draws the canvas. The brush moves up and down with your hand, sweeps left to right, and leaves a glowing trail in the knob's color.
- **Web Audio:** plays the notes. The pitch comes from your hand distance, the same number the LED color comes from.

How the features work:

- **Stamps:** when you press the button, the page draws a permanent glowing dot labeled with the note name, and saves that note in a list. The dot is art; the list is music.
- **Sequencer:** "Play stamps" sweeps a playhead across the canvas on an 8-second loop. Every stamped dot it crosses plays its note. Your hand keeps playing live over the loop, so you are improvising with your own painting.
- **Sounds:** two voices. The hand plays a triangle wave, the sequencer plays its own separate voice, so both can sound at once. Both go to the speakers and to the recorder.
- **Record:** captures the canvas as video and the audio as sound, and downloads one .webm file you can share. The camera was removed because it froze the page; the dark canvas is the painting surface.

## The one-line version of everything

The sensor reads your hand, the knob reads your color choice, the Arduino translates both into numbers, the numbers travel over USB to the web page, and the web page turns them into light, sound, and a painting that plays itself back.
