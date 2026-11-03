# Creative-Code-A2


Ultrasonic Distance-Reactive Breathing LED
An interactive lighting installation based on an Arduino, an HC-SR04 ultrasonic sensor, and a programmable LED strip. The LED strip changes color in real-time based on the distance of an object, while simultaneously incorporating a "breathing" light effect—creating an engaging, high-tech interactive experience.


Features
Real-time Distance Measurement: Uses an HC-SR04 ultrasonic module to measure distance every 0.5 seconds.
Distance-based Color Mapping: When an object is detected within 50 cm, the color shifts dynamically based on the distance—transitioning from red → orange → yellow → green → blue.
Breathing Light Effect: The brightness cycles smoothly between 0 and 255, with updates occurring every 5 ms.
Automatic Turn-off: The LED strip switches off automatically when no object is detected or when an object is beyond the detection range (>50 cm).
Serial Debugging: Outputs measured distance data in real time to facilitate debugging.


Hardware List
Arduino development board (UNO/Nano/Mega, etc.)	1	Main controller
HC-SR04 ultrasonic distance sensor module	      1	Distance detection
NeoPixel LED strip (16 LEDs)	                  1	Display
Jumper wires	                                  4	Connections


Wiring Instructions
Ultrasonic Module HC-SR04
HC-SR04 Pin	 Arduino Pin
VCC	         5V
GND	         GND
Trig	       D9
Echo	       D10

NeoPixel LED Strip
LED Strip Pin	  Arduino Pin
VCC 	          5V
GND	            GND
DIN (Data)	    D6


Software Dependencies
Install via the Library Manager in the Arduino IDE:
Adafruit NeoPixel by Adafruit


Instructions
Connect the hardware according to the wiring diagram.
Open the Arduino IDE and load the project's .ino file.
Select the correct board model and port.
Open the Serial Monitor to view real-time distance readings.


Color Mapping Rules
Detection range: 2 cm – 50 cm; color transitions are defined as follows:

Distance Range	Color Transition
2 – 8 cm	Red → Orange
8 – 16 cm	Orange → Yellow
16 – 26 cm	Yellow → Green
26 – 38 cm	Green → Blue
38 – 50 cm	Solid Blue
< 2 cm or > 50 cm	Off (No object detected)


Technical Description
Ranging Principle: Distance (cm) = Echo Time (μs) × 0.034 / 2
Color Interpolation: The `map()` function is used to linearly map the distance to an interpolation coefficient in the range 0–255.
Brightness Overlay: Final RGB = Current Color RGB × Brightness / 255; a `long` type cast is applied to prevent overflow during multiplication.
Non-blocking Timing: All timed tasks are managed based on the difference in `millis()` values; `delay()` is not used, ensuring multi-tasking capability.



