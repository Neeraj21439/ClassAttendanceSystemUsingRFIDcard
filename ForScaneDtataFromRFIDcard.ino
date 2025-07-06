#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// RFID and Buzzer Pins
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D2

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

int blockNum = 4;
byte bufferLen = 18;
byte readBlockData[18];

String studentName = "";
String rollNumber = "";

// Replace with your actual Google Apps Script Web App URL
const String sheet_url = "/*enter your sheet url here*/";

// Your WiFi credentials
#define WIFI_SSID "/* enter wi-fi name here*/"
#define WIFI_PASSWORD "/*enter wi-fi password here*/"

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(BUZZER, OUTPUT);
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n**Card Detected**");

  ReadDataFromBlock(blockNum, readBlockData);

  String cardData = String((char*)readBlockData);
  cardData.trim();

  Serial.println("✅ Block read successfully");
  Serial.print("Raw Block Data: ");
  Serial.println(cardData);

  int commaIndex = cardData.indexOf(',');
  if (commaIndex != -1) {
    studentName = cardData.substring(0, commaIndex);

    // Extract digits only from roll number part
    rollNumber = "";
    for (int i = commaIndex + 1; i < cardData.length(); i++) {
      char c = cardData[i];
      if (isDigit(c)) {
        rollNumber += c;
      } else {
        break;
      }
    }
  } else {
    Serial.println("❌ Error: Data format incorrect!");
    return;
  }

  Serial.println("Extracted Name: " + studentName);
  Serial.println("Extracted Roll Number: " + rollNumber);

  // Buzzer beep
  digitalWrite(BUZZER, HIGH); delay(150);
  digitalWrite(BUZZER, LOW);  delay(100);
  digitalWrite(BUZZER, HIGH); delay(150);
  digitalWrite(BUZZER, LOW);

  // Send to Google Sheets
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    String fullUrl = sheet_url + "name=" + studentName + "&rollno=" + rollNumber;
    fullUrl.trim();
    Serial.println(fullUrl);

    HTTPClient https;
    Serial.println("[HTTPS] begin...");

    if (https.begin(*client, fullUrl)) {
      Serial.println("[HTTPS] GET...");
      int httpCode = https.GET();

      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.println("[HTTPS] Unable to connect");
    }
  }

  delay(1000);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("❌ Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("❌ Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}
