# Theremin Painter

An Arduino build for a class demo: wave your hand over an ultrasonic sensor to play notes, turn a knob to pick a color, press a button to stamp the note onto a canvas. The web app is the canvas, the sequencer, and the recorder.

## Hardware

- Arduino Uno (or any board with the same pins)
- HC-SR04 ultrasonic sensor
- Potentiometer (color)
- Push button (stamp)
- RGB LED, common cathode, with resistors
- Optional 16x2 LCD

## Files

- `theremin-painter.ino`: the sketch. Reads the sensor, the knob, and the button, and sends simple serial messages to the web app.
- `theremin-painter.html`: the web app. Open it in a browser (it loads p5.js from a CDN). Hand height plays notes from a C major pentatonic scale, closer is higher. The knob color washes across the canvas. Press the button on the board to stamp notes. Play stamps runs an 8 second loop of your stamps. Record saves a .webm clip with sound.
- `theremin-painter-lesson-plan.md`: the full lesson plan for the online course.
- `theremin-painter-demo-notes.md`: short demo notes for showing it in class.

## Run it

1. Upload the sketch to the board.
2. Open `theremin-painter.html` in Chrome or Edge.
3. Pick the board's serial port when the page asks, or just watch the canvas respond to the sensor.

Built for a basic Arduino class at St. Petersburg College.
