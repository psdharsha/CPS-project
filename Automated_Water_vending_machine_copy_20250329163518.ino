#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL5aGhqqoJY"
#define BLYNK_TEMPLATE_NAME "water vending"
#define BLYNK_AUTH_TOKEN "Njjz2Makm_grl8dB7yENnCSZdzrTHupm"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// WiFi Credentials
char auth[] = "Njjz2Makm_grl8dB7yENnCSZdzrTHupm";
char ssid[] = "Bunny";
char pass[] = "psdharsha";

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Stepper Motor Configuration
#define IN1 D1  // GPIO5
#define IN2 D2  // GPIO4
#define IN3 D3  // GPIO0
#define IN4 D4  // GPIO2

// 8-step sequence for 28BYJ-48
const int stepSequence[8][4] = {
  {1, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 1, 1}, {0, 0, 1, 0},
  {0, 1, 1, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}
};

// System Variables
int selectedBottles = 0;
bool selectionMade = false;
const int stepsPer360 = 4096;        // Steps for one full rotation
const int rotationsPerBottle = 20;    // 4 full rotations per bottle
const int stepsPerBottle = stepsPer360 * rotationsPerBottle; // 16384 steps
const int motorSpeedDelay = 800;     // Lower = faster (800-2000 recommended)

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  Wire.begin(D6, D5);  // SDA=D6 (GPIO12), SCL=D5 (GPIO14)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Water Vending");
  display.println("Machine Ready");
  display.display();
  delay(2000);

  // Initialize Stepper Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  resetDisplay();
}

void loop() {
  Blynk.run();
}

// Blynk Handlers
BLYNK_WRITE(V1) {
  selectedBottles = param.asInt();
  selectedBottles = constrain(selectedBottles, 1, 3);
  selectionMade = true;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Selected: ");
  display.print(selectedBottles);
  display.println(" bottles");
  display.print("Price: Rs ");
  display.println(selectedBottles * 100);
  display.display();
  delay(2000);
  
  displayWaitingForPayment();
}

BLYNK_WRITE(V3) {
  String cmd = param.asStr();
  if (selectionMade && cmd.equalsIgnoreCase("PAY")) {
    dispenseWater();
  }
}

// Motor Control Function (4 full rotations per bottle)
void rotateMotor(long steps) {
  int direction = (steps > 0) ? 1 : -1;
  steps = abs(steps);
  
  for (long i = 0; i < steps; i++) {
    int step = (direction > 0) ? (i % 8) : (7 - (i % 8));
    digitalWrite(IN1, stepSequence[step][0]);
    digitalWrite(IN2, stepSequence[step][1]);
    digitalWrite(IN3, stepSequence[step][2]);
    digitalWrite(IN4, stepSequence[step][3]);
    delayMicroseconds(motorSpeedDelay);
  }
}

// Dispensing Logic
void dispenseWater() {
  display.clearDisplay();
  display.println("Dispensing...");
  display.print(selectedBottles * rotationsPerBottle);
  display.println(" rotations");
  display.display();

  // Rotate 4 full turns per bottle (1440° total)
  long totalSteps = selectedBottles * stepsPerBottle;
  rotateMotor(totalSteps);

  display.clearDisplay();
  display.println("Thank You!");
  display.println("Visit Again");
  display.display();
  delay(3000);
  
  resetSystem();
}

// Display Functions
void displayWaitingForPayment() {
  display.clearDisplay();
  display.println("Waiting for");
  display.println("Payment...");
  display.println("Send 'PAY' to V3");
  display.display();
}

void resetDisplay() {
  display.clearDisplay();
  display.println("Select Bottles");
  display.println("via Blynk V1");
  display.println("(1-3)");
  display.display();
}

void resetSystem() {
  selectionMade = false;
  selectedBottles = 0;
  resetDisplay();
}