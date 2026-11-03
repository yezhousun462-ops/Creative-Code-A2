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


The following involves calling or drawing upon code written by others.
// Define pin numbers
const int trigPin = 9;
const int echoPin = 10;

// Variables for duration and distance
long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT); // Set the trigPin as an Output
  pinMode(echoPin, INPUT);  // Set the echoPin as an Input
  Serial.begin(9600);       // Start serial communication at 9600 baud
}

void loop() {
  // Clear the trigPin by setting it LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Set the trigPin HIGH for 10 microseconds to send out a sonic burst
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, returning the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance in cm (Speed of sound is ~0.034 cm/microsecond, divide by 2 for go-and-return)
  distance = duration * 0.034 / 2;

  // Print the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500); // Wait half a second before next reading
}


Breathing light
int ledPin = 9; // 将 LED 连接到支持 PWM 的数字引脚 9

void setup() {
  pinMode(ledPin, OUTPUT); // 设置引脚为输出模式
}

void loop() {
  // 渐亮过程（从暗到亮）
  for (int value = 0; value <= 255; value += 5) {
    analogWrite(ledPin, value); 
    delay(30); 
  }

  // 渐暗过程（从亮到暗）
  for (int value = 255; value >= 0; value -= 5) {
    analogWrite(ledPin, value); 
    delay(30); 
  }
}
