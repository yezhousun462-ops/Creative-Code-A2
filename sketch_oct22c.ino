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

long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

// 根据距离用 map() 计算颜色
// 红 -> 橙 -> 黄 -> 绿 -> 蓝（范围 2~50cm）
void updateColorByDistance(long distance) {
  long d = distance;
  if (d < NEAR_DISTANCE_CM) d = NEAR_DISTANCE_CM;
  if (d > FAR_DISTANCE_CM)  d = FAR_DISTANCE_CM;

  // 各档位距离边界（2~50cm 内均匀划分）
  const int D_RED_END    = 8;    // 2~8   红 -> 橙
  const int D_ORANGE_END = 16;   // 8~16  橙 -> 黄
  const int D_YELLOW_END = 26;   // 16~26 黄 -> 绿
  const int D_GREEN_END  = 38;   // 26~38 绿 -> 蓝
                                 // 38~50 纯蓝

  // 各档位颜色
  const int cRedR = 255,    cRedG = 0,   cRedB = 0;
  const int cOrangeR = 255, cOrangeG = 128, cOrangeB = 0;
  const int cYellowR = 255, cYellowG = 255, cYellowB = 0;
  const int cGreenR = 0,    cGreenG = 255, cGreenB = 0;
  const int cBlueR = 0,     cBlueG = 0,   cBlueB = 255;

  if (d <= D_RED_END) {
    // 红 -> 橙
    int t = map(d, NEAR_DISTANCE_CM, D_RED_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cRedR, cOrangeR);
    currentG = map(t, 0, 255, cRedG, cOrangeG);
    currentB = map(t, 0, 255, cRedB, cOrangeB);
  }
  else if (d <= D_ORANGE_END) {
    // 橙 -> 黄
    int t = map(d, D_RED_END, D_ORANGE_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cOrangeR, cYellowR);
    currentG = map(t, 0, 255, cOrangeG, cYellowG);
    currentB = map(t, 0, 255, cOrangeB, cYellowB);
  }
  else if (d <= D_YELLOW_END) {
    // 黄 -> 绿
    int t = map(d, D_ORANGE_END, D_YELLOW_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cYellowR, cGreenR);
    currentG = map(t, 0, 255, cYellowG, cGreenG);
    currentB = map(t, 0, 255, cYellowB, cGreenB);
  }
  else if (d <= D_GREEN_END) {
    // 绿 -> 蓝
    int t = map(d, D_YELLOW_END, D_GREEN_END, 0, 255);
    t = constrain(t, 0, 255);
    currentR = map(t, 0, 255, cGreenR, cBlueR);
    currentG = map(t, 0, 255, cGreenG, cBlueG);
    currentB = map(t, 0, 255, cGreenB, cBlueB);
  }
  else {
    // 最远段：纯蓝
    currentR = cBlueR;
    currentG = cBlueG;
    currentB = cBlueB;
  }

  currentR = constrain(currentR, 0, 255);
  currentG = constrain(currentG, 0, 255);
  currentB = constrain(currentB, 0, 255);
}

//根据当前颜色 + 当前亮度刷新所有 LED
void refreshLeds() {
  int r = (int)((long)currentR * brightness / 255);
  int g = (int)((long)currentG * brightness / 255);
  int b = (int)((long)currentB * brightness / 255);

  if (objectDetected == 0) {
    r = 0;
    g = 0;
    b = 0;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void loop() {
  unsigned long now = millis();

  // 任务1：超声波测量（每 0.5s）
  if (now - lastMeasureTime >= MEASURE_INTERVAL) {
    lastMeasureTime = now;

    long distance = measureDistance();
    lastDistance = distance;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance > 0 && distance < DETECT_DISTANCE_CM) {
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
