# Theremin Painter: Complete Lesson Plan

## The big idea

Students build an instrument they play without touching. An ultrasonic sensor reads hand distance and turns it into musical pitch, a knob controls paint color, and a button stamps notes onto a living canvas in the browser. Then the painting becomes a sequencer: it plays the stamped melody back while the student improvises over it. Every core Arduino concept appears in one project: digital and analog input, PWM output, serial communication, and mapping values between domains.

## Learning objectives

By the end, students can:

- Wire and read an ultrasonic distance sensor, explaining how echo timing becomes distance.
- Read analog (potentiometer) and digital (button) inputs, and explain why the button needs a pull-down resistor.
- Drive an RGB LED with PWM, explaining why PWM can fake an analog output.
- Explain how the Arduino streams data to a computer over serial, and how a web page parses it.
- Use the map() function to convert sensor ranges into musical and visual ranges.
- Debug a real circuit systematically: one leg per row, the resistor bridge rule.

## Materials per station

- Arduino UNO R3 + USB cable, solderless breadboard
- HC-SR04 ultrasonic sensor
- 10k potentiometer (trim pot or knob type)
- Push button, RGB LED (common cathode)
- LCD1602 (optional, second session)
- Resistors: 3x 220 ohm (LED), 1x 10k (button), 1x 1k (LCD contrast)
- About 20 jumper wires
- Laptop with Chrome or Edge (Web Serial), Arduino IDE

## Core concepts

### 1. Ultrasonic distance sensing (HC-SR04)

The sensor sends an inaudible click and listens for the echo. The Arduino measures how long the echo takes to return, then divides by two (there and back) to get distance. Key vocabulary: trigger, echo, speed of sound. Demo: wave a hand and watch the numbers change in the Serial Monitor before any music happens.

### 2. Analog input (potentiometer)

A potentiometer is a voltage divider. The wiper (middle leg) picks a voltage between 0 and 5V depending on knob position, and the Arduino's analog-to-digital converter turns that into a number from 0 to 1023. Ten bits, 1024 steps. This is the color control.

### 3. Digital input and the pull-down resistor (button)

A button connects 5V to a pin when pressed. But when released, the pin is connected to nothing, and it picks up electrical noise and reads randomly. The 10k resistor to ground "pulls down" the pin to a clean 0 when the button is open. This is the single most useful debugging concept in the project: a floating pin lies.

### 4. PWM output (RGB LED)

Arduino digital pins are on or off, but rapidly pulsing them (pulse width modulation) fakes a dimmer. analogWrite(pin, 0-255) sets the duty cycle. The RGB LED has three tiny LEDs inside (red, green, blue) sharing one ground leg (the longest). Mixing brightnesses mixes colors, the same way screens do.

### 5. Serial communication

The Arduino prints one line ten times a second: D:42,P:512,B:0 (distance, pot, button). The browser opens the USB serial port (Web Serial API, Chrome/Edge) and parses each line. Emphasize: only one program can hold the serial port at a time. If the web page says "failed to open serial port," close the Arduino Serial Monitor first.

### 6. Mapping values

The heart of the project. Distance in centimeters (5-60) maps to ten notes of a C major pentatonic scale. Potentiometer (0-1023) maps to hue (0-359). The map() function is the translator between the physical world and the musical/visual world. Change the mapping, change the instrument.

## Wiring (student version)

Power first: UNO 5V to red rail, GND to blue rail.

1. HC-SR04: VCC to red rail, GND to blue rail, Trig to D9, Echo to D10.
2. Potentiometer: outer legs to red and blue rails, middle leg to A0.
3. Button: one leg to red rail, other leg to D2, plus 10k resistor from the D2 row to blue rail.
4. RGB LED: four legs in four consecutive rows, one leg per row. Long leg's row: jumper to blue rail. Each other leg: 220 ohm resistor from its row to an empty row, jumper from there to D3 (red), D5 (green), D6 (blue).
5. LCD (optional): pin 1 GND, pin 2 5V, pin 3 through 1k to GND, pin 4 D7, pin 5 GND, pin 6 D8, pins 11-14 to D11/D12/D13/D4, pin 15 through 220 ohm to 5V, pin 16 GND. Pins 7-10 empty.

The golden rule, worth putting on the board: one component leg per row. A resistor must bridge two different rows. A resistor with both ends in the same row does nothing.

## Sketch walkthrough

Structure: setup() configures pins and starts serial at 9600 baud. loop() reads the three inputs, updates the LED and LCD, prints the data line, and waits 100ms (ten readings per second).

- Reading the HC-SR04: pulse Trig, measure Echo with pulseIn(), convert microseconds to centimeters.
- Playing zone: distances outside 5-60 cm read as "no hand," and the brush parks.
- Hue math: pot 0-1023 becomes hue 0-359, then hue-to-RGB sectors drive the three PWM pins.
- The data line: the sketch prints D:42,P:512,B:0. The web page splits on commas and colons.

## Web app walkthrough

Single HTML file, no server. p5.js draws the canvas, Web Serial reads the Arduino, Web Audio plays the notes.

- Parsing: each serial line is matched against D:digits,P:digits,B:0-or-1.
- Brush: distance sets vertical position and pitch; the brush sweeps left to right leaving a fading, glowing trail.
- Stamps: a button press draws a permanent glowing dot labeled with the note name, and records it for the sequencer.
- Sequencer: a playhead sweeps the canvas on an 8-second loop, playing each stamped note it crosses. The hand keeps playing live over the loop.
- Recording: the canvas and audio are captured to a .webm file for sharing.

## Suggested class flow

- Session 1: concepts 1-4, wire sensor + pot + button + LED, Serial Monitor smoke test (D/P/B lines respond).
- Session 2: the sketch's mapping code, web app connect and paint, musical scales discussion.
- Session 3: sequencer and recording, student performances, LCD wiring for early finishers.

## Troubleshooting: greatest hits from the bench

- "Failed to open serial port": close the Arduino Serial Monitor. Only one program holds the port.
- LED dark: check the long leg reaches ground; check each resistor bridges two different rows; confirm 220 ohm (red-red-brown), not 10k.
- LED wrong colors: the D3/D5/D6 jumpers may be swapped at the Arduino end.
- Button reads 1 constantly: the 10k pull-down resistor is missing or in the wrong row (floating pin).
- Potentiometer jumpy: some pots are 500k instead of 10k; they work but read noisier.
- Page freezes on camera enable: the camera is optional decoration; skip it and paint on the dark canvas.

## Assessment ideas

- Explain in plain words what the pull-down resistor does and what happens without it.
- Given a new sensor range, write the map() call to convert it to a 0-100 brightness.
- Remix: change the scale to minor pentatonic, change the trail length, or add a second button function. Present the remix and explain what changed.

## Extensions

- LCD: display the current note name and color on the breadboard.
- MIDI out: send notes to a DAW instead of the browser's synth.
- Second sensor: two-handed playing, one hand pitch, one hand volume.
- Save/load: store stamp patterns in the browser and reload them.
