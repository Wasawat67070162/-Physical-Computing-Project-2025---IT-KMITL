#include "ArduinoBLE.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin setup ---
const int buzzerPin = 3;
const int buttonPin = 2;

// RGB LED (Common Cathode)
const int redPin = 5;
const int greenPin = 6;
const int bluePin = 7;

bool mute = false;

// --- LCD setup ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // เปลี่ยน 0x27 เป็น 0x3F ถ้าไม่ขึ้น

// --- BLE setup ---
BLEService alertService("180F");
BLECharacteristic statusChar("2A19", BLERead | BLENotify | BLEWrite, 20);

void setup() {
  Serial.begin(9600);

  // --- Pin setup ---
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // --- LCD setup ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Anti-Lost Alarm");
  lcd.setCursor(0, 1);
  lcd.print("Status: Ready");

  digitalWrite(buzzerPin, HIGH);
  setLEDDisconnected();

  // --- BLE setup check---
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    lcd.clear();
    lcd.print("BLE Failed!");
    while (1);
  }

  BLE.setLocalName("ALAN");
  BLE.setAdvertisedService(alertService);
  alertService.addCharacteristic(statusChar);
  BLE.addService(alertService);

  statusChar.writeValue("Ready");
  BLE.advertise();

  Serial.println("✅ BLE device ready");
  setLEDConnected();
}

void loop() {
  BLEDevice connectDevice = BLE.central();

  readMuteButton();

  if (connectDevice) {
    Serial.print("Connected to: ");
    Serial.println(connectDevice.address());
    digitalWrite(buzzerPin, HIGH);
    if (!mute) setLEDConnected();
    sendStatus("Connected");

    lcd.clear();
    lcd.print("BLE: Connected");

    while (connectDevice.connected()) {
      readMuteButton();
      delay(50);
    }

    Serial.print("Disconnected from: ");
    Serial.println(connectDevice.address());
    sendStatus("Disconnected");

    lcd.clear();
    lcd.print("BLE: Lost");

    if (!mute) {
      lcd.setCursor(0, 1);
      lcd.print("Alarm Triggered!");

      for (int i = 0; i < 10; i++) {
        digitalWrite(buzzerPin, LOW);
        setLEDDisconnected();
        delay(300);
        digitalWrite(buzzerPin, HIGH);
        turnOffLED();
        delay(300);
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Mute ON, Safe");
      setLEDMuteON();
    }
  }
}

// --- อ่านปุ่ม mute ---
void readMuteButton() {
  static bool lastButton = HIGH;
  bool buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButton == HIGH) {
    mute = !mute;
    Serial.print(" Mute toggled: ");
    Serial.println(mute ? "ON" : "OFF");

    digitalWrite(buzzerPin, LOW);
    delay(100);
    digitalWrite(buzzerPin, HIGH);

    if (mute) {
      setLEDMuteON();
      sendStatus("Mute ON");
      lcd.clear();
      lcd.print("Mute: ON");
    } else {
      setLEDConnected();
      sendStatus("Mute OFF");
      lcd.clear();
      lcd.print("Mute: OFF");
    }

    delay(250);
  }

  lastButton = buttonState;
}

// --- ส่งข้อความ BLE ---
void sendStatus(const char* message) {
  statusChar.writeValue(message);
}

// --- LED Functions ---
void setLEDConnected() {
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void setLEDDisconnected() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
}

void setLEDMuteON() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
}

void turnOffLED() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}
