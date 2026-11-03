#include <Adafruit_NeoPixel.h>
#define LED_PIN    6
#define LED_COUNT  16

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//超声波引脚
const int trigPin = 9;
const int echoPin = 10;

//颜色表（保留，用于参考/调试）
int colors[][3] = {
  {255, 0,   0  },   // 0 红
  {0,   255, 0  },   // 1 绿
  {0,   0,   255},   // 2 蓝
  {255, 255, 0  },   // 3 黄
  {255, 0,   255},   // 4 紫
  {0,   255, 255},   // 5 青
  {255, 128, 0  },   // 6 橙
  {255, 105, 180}    // 7 粉
};
int colorCount = sizeof(colors) / sizeof(colors[0]);

//呼吸灯状态
int  brightness   = 0;      // 当前亮度 (0~255)
int  breathDir    = 1;      // 1 = 渐亮，-1 = 渐暗
unsigned long lastBreathTime = 0;
const unsigned long BREATH_INTERVAL = 5;   // 每 5ms 更新一次亮度

//超声波状态
int  objectDetected = 0;            // 0 = 没感应到，1 = 感应到
const int DETECT_DISTANCE_CM = 50;  // 阈值：50cm 内有物体算"感应到"
const int NEAR_DISTANCE_CM   = 2;   // 最近距离
const int FAR_DISTANCE_CM    = DETECT_DISTANCE_CM;

// 当前距离对应的颜色
int currentR = 0;
int currentG = 0;
int currentB = 0;

long lastDistance = 0;

unsigned long lastMeasureTime = 0;
const unsigned long MEASURE_INTERVAL = 500; // 每 0.5s 测一次

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  strip.begin();
  strip.show();
}

// 测一次距离（cm）
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