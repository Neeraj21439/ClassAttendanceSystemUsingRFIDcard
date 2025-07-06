#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  D3
#define SS_PIN   D4

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

int blockNum = 4;
byte buffer[18];
String dataToWrite = "name,rollno"; //enter name or roll-no 

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Place your card to write...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  // Authentication key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Pad data
  dataToWrite.trim();
  while (dataToWrite.length() < 16) dataToWrite += " ";
  dataToWrite.getBytes(buffer, 18);

  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX); Serial.print(" ");
  }
  Serial.println();

  // Authenticate
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write
  status = mfrc522.MIFARE_Write(blockNum, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("✅ Data written successfully!");

  // Read Back
  byte readBuffer[18];
  byte len = 18;
  status = mfrc522.MIFARE_Read(blockNum, readBuffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.print("📦 Data in block ");
  Serial.print(blockNum);
  Serial.print(": ");
  for (byte i = 0; i < 16; i++) {
    Serial.write(readBuffer[i]);
  }
  Serial.println();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);
}
