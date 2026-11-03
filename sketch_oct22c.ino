#include <Adafruit_NeoPixel.h>  // Include the Adafruit NeoPixel library
#define LED_PIN    6  // Define the LED strip data pin as pin 6 on the Arduino
#define LED_COUNT  16  // Define the total number of LED beads as 16

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);  
// Create a NeoPixel object named 'strip':
// - LED_COUNT: Number of LEDs
// - LED_PIN: Data pin
// - NEO_GRB: Color order is GRB (most WS2812 LED strips use the GRB order)
// - NEO_KHZ800: Communication speed is 800kHz

const int trigPin = 9;  // Connect the ultrasonic module's Trig (trigger) pin to Arduino pin 9
const int echoPin = 10;  // Connect the Echo pin of the ultrasonic module to pin 10 on the Arduino

//颜色表（保留，用于参考/调试）
int colors[][3] = {
  {255, 0,   0  },   // Index 0: Red (R=255, G=0, B=0)
  {0,   255, 0  },   // Index 1: Green (R=0, G=255, B=0)
  {0,   0,   255},   // Index 2: Blue (R=0, G=0, B=255)
  {255, 255, 0  },   // Index 3: Yellow (R=255, G=255, B=0)
  {255, 0,   255},   // Index 4: Purple (R=255, G=0, B=255)
  {0,   255, 255},   // Index 5: Cyan (R=0, G=255, B=255)
  {255, 128, 0  },   // Index 6: Orange (R=255, G=128, B=0)
  {255, 105, 180}    // Index 7: Pink (R=255, G=105, B=180)
};
int colorCount = sizeof(colors) / sizeof(colors[0]);  // Calculate the number of colors in the color table (total number of bytes / bytes per row = 8)

int  brightness   = 0;      // Current brightness (0–255)   // Current LED brightness value; 0 means completely off, 255 means maximum brightness
int  breathDir    = 1;      // 1 = brightening, -1 = dimming // Breathing light direction indicator: 1 denotes increasing brightness, -1 denotes decreasing brightness
unsigned long lastBreathTime = 0;  // Timestamp of the last update to the breathing light brightness (in milliseconds)
const unsigned long BREATH_INTERVAL = 5;   // Breathing light update interval: Adjust brightness every 5 milliseconds to create a smooth breathing effect

int  objectDetected = 0;            // 0 = Not detected, 1 = Detected  //Whether an object has been detected within the detection range
const int DETECT_DISTANCE_CM = 50;  // Detection threshold: An object is considered detected if the distance is less than 50 cm.
const int NEAR_DISTANCE_CM   = 2;   // Minimum distance threshold for color mapping: 2 cm
const int FAR_DISTANCE_CM    = DETECT_DISTANCE_CM;  // Maximum distance boundary for color mapping: equal to the detection threshold of 50 cm

int currentR = 0;  // The red component of the color currently to be displayed (0–255)
int currentG = 0;  // The green component of the color currently to be displayed (0–255)
int currentB = 0;  // The blue component of the color currently to be displayed (0–255)

long lastDistance = 0;  // Store the previously measured distance value (cm)

unsigned long lastMeasureTime = 0;  // Record the timestamp of the previous ultrasonic measurement (in milliseconds)
const unsigned long MEASURE_INTERVAL = 500; // Ultrasonic measurement interval: measure distance every 500 milliseconds

void setup() {
  Serial.begin(9600);  // Initialize serial communication with a baud rate of 9600

  pinMode(trigPin, OUTPUT);  // Set the Trig pin to output mode
  pinMode(echoPin, INPUT);  // Set the Echo pin to input mode

  strip.begin();  // Initialize the NeoPixel LED strip
  strip.show();  // Refresh the LED strip immediately (defaults to all off)
}

long measureDistance() {  // Define the function measureDistance(), which returns the measured distance (in cm)
  digitalWrite(trigPin, LOW);  // First, pull the Trig pin low to ensure the trigger signal starts from a low level.
  delayMicroseconds(2);  // Hold the signal low for 2 microseconds to meet the sensor's timing requirements
  digitalWrite(trigPin, HIGH);  // Pull Trig high to start sending a trigger pulse
  delayMicroseconds(10);  // Maintain a high level for 10 microseconds
  digitalWrite(trigPin, LOW);  // Pull the Trig pin low to end the trigger pulse; the sensor then begins emitting ultrasonic waves.

  long duration = pulseIn(echoPin, HIGH);  // Wait for the Echo pin to go high and measure the duration for which it remains high (in microseconds)
  long distance = duration * 0.034 / 2;  
// Calculate distance:
// Speed ​​of sound is approximately 340 m/s = 0.034 cm/μs
// duration * 0.034 gives the round-trip distance (in cm); divide by 2 to get the one-way distance
  return distance;
}

// Calculate color based on distance using map()
// Red -> Orange -> Yellow -> Green -> Blue (range: 2–50 cm)
void updateColorByDistance(long distance) {   // Define a function: update the global color variables currentR, currentG, and currentB based on distance
  long d = distance;
  if (d < NEAR_DISTANCE_CM) d = NEAR_DISTANCE_CM;  // If the distance is less than the minimum boundary (2cm), force it to 2cm to prevent the map from going out of bounds.
  if (d > FAR_DISTANCE_CM)  d = FAR_DISTANCE_CM;  //If the distance exceeds the maximum boundary (50cm), it is capped at 50cm to prevent the map from going out of bounds.

  const int D_RED_END    = 8;    // End distance for the red-to-orange transition segment: 8 cm
  const int D_ORANGE_END = 16;   // End distance for the orange-to-yellow transition segment: 16cm
  const int D_YELLOW_END = 26;   // End distance for the yellow-to-green transition segment: 26cm
  const int D_GREEN_END  = 38;   // End distance for the green-to-blue transition segment: 38 cm; the range from 38 to 50 cm is solid blue.

  const int cRedR = 255,    cRedG = 0,   cRedB = 0;
  const int cOrangeR = 255, cOrangeG = 128, cOrangeB = 0;
  const int cYellowR = 255, cYellowG = 255, cYellowB = 0;
  const int cGreenR = 0,    cGreenG = 255, cGreenB = 0;
  const int cBlueR = 0,     cBlueG = 0,   cBlueB = 255;

  if (d <= D_RED_END) {  // Scenario 1: Distance is between 2 and 8 cm; color transitions from red to orange.
    int t = map(d, NEAR_DISTANCE_CM, D_RED_END, 0, 255);  // Linearly map the distance d from [2, 8] to [0, 255] to obtain the interpolation coefficient t
    t = constrain(t, 0, 255);  // Constrain t to the range 0–255 to prevent it from going out of bounds
    currentR = map(t, 0, 255, cRedR, cOrangeR);  // The red component is interpolated between red (255) and orange (255) (both are actually 255)
    currentG = map(t, 0, 255, cRedG, cOrangeG);  // The green component is interpolated between red (0) and orange (128), creating a gradient from red to orange.
    currentB = map(t, 0, 255, cRedB, cOrangeB);  // The blue component is interpolated between red (0) and orange (0) (both are actually 0)
  }
  else if (d <= D_ORANGE_END) {  // Scenario 2: Distance is between 8 and 16 cm; color transitions from orange to yellow
    int t = map(d, D_RED_END, D_ORANGE_END, 0, 255);  
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cOrangeR, cYellowR);
    currentG = map(t, 0, 255, cOrangeG, cYellowG);
    currentB = map(t, 0, 255, cOrangeB, cYellowB);
  }
  else if (d <= D_YELLOW_END) {
    int t = map(d, D_ORANGE_END, D_YELLOW_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cYellowR, cGreenR);
    currentG = map(t, 0, 255, cYellowG, cGreenG);
    currentB = map(t, 0, 255, cYellowB, cGreenB);
  }
  else if (d <= D_GREEN_END) {
    int t = map(d, D_YELLOW_END, D_GREEN_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cGreenR, cBlueR);
    currentG = map(t, 0, 255, cGreenG, cBlueG);
    currentB = map(t, 0, 255, cGreenB, cBlueB);
  }
  else {
    currentR = cBlueR;
    currentG = cBlueG;
    currentB = cBlueB;
  }

  currentR = constrain(currentR, 0, 255);  // Finally, ensure once more that the red component is within the valid range of 0–255
  currentG = constrain(currentG, 0, 255);  // Finally, ensure once more that the green component is within the valid range of 0–255.
  currentB = constrain(currentB, 0, 255);  // Finally, ensure once more that the blue component is within the valid range of 0–255.
}

void refreshLeds() {  // Define a function: Scale the current color by the current brightness, write it to all LED beads, and refresh the display.
  int r = (int)((long)currentR * brightness / 255);  // Calculate actual red brightness: current red component × breathing brightness / 255
  int g = (int)((long)currentG * brightness / 255);  // Use long multiplication to avoid int overflow, then convert back to int
  int b = (int)((long)currentB * brightness / 255);

  if (objectDetected == 0) {  // If no object is currently detected
    r = 0;
    g = 0;
    b = 0;
  }
  // In other words: the LED strip turns off completely when no object is detected.

  for (int i = 0; i < LED_COUNT; i++) {  // Iterate through each LED (from 0 to LED_COUNT-1)
    strip.setPixelColor(i, strip.Color(r, g, b));  // Set the color of the i-th LED to (r, g, b)
  }
  strip.show();  // Actually send all the configured colors to the LED strip and display them
}

void loop() {
  unsigned long now = millis();  // Get the current system uptime (in milliseconds)

  if (now - lastMeasureTime >= MEASURE_INTERVAL) {  // If more than 500ms has elapsed since the last measurement
    lastMeasureTime = now;  // Update the last measurement time to the current time

    long distance = measureDistance();  // Call measureDistance() to take a distance measurement
    lastDistance = distance;  // Save the current distance to lastDistance

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance > 0 && distance < DETECT_DISTANCE_CM) {  // If the distance is valid (greater than 0) and less than 50cm, an object is considered detected.
      objectDetected = 1;
      updateColorByDistance(distance);
    } else {
      objectDetected = 0;
    }
  }

  // 呼吸灯刷新（每 5ms）
  if (now - lastBreathTime >= BREATH_INTERVAL) {
    lastBreathTime = now;

    brightness += breathDir;

    if (brightness >= 255) {
      brightness = 255;
      breathDir = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      breathDir = 1;
    }

    refreshLeds();
  }
}
