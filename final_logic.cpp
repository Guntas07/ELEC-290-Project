#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library

#define RST_PIN 8   // Connect RST to Digital Pin 8
#define SS_PIN  7   // Slave Select pin, connected to SDA on MFRC522
#define LED_PIN 13  // LED
#define PIR_PIN 2   // PIR Out pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create an instance of the class
int pirStat = 0;                   // PIR status
unsigned long lastMotionTime = 0;  // Timestamp of the last motion detection
unsigned long lastCardScanTime = 0; // Timestamp of last card scan
int rfidReady = false;             // Flag to indicate system is ready to read RFID
String authorizedUIDs[] = {"A3FDF04F", "33F0F002"};  // Array of authorized UIDs

unsigned long lastSerialUpdate = 0;  // Track last time serial output was updated
unsigned long serialUpdateInterval = 1000; // Update serial output every 1 second
unsigned long ledOnTime = 0;         // Time to turn off the LED after scanning

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(RST_PIN, OUTPUT);       // Set the digital pin as output for RST
    digitalWrite(RST_PIN, HIGH);    // Optional: Keep the RFID module active on boot
    Serial.begin(9600);             // Start serial communication at 9600 baud
    SPI.begin();                    // Initialize SPI bus
    mfrc522.PCD_Init();             // Initialize the MFRC522 card
    Serial.println("Serial Monitor connected");
}

bool checkUID(String readUID) {
    for (String uid : authorizedUIDs) {
        if (readUID.equals(uid)) {
            return true;
        }
    }
    return false;
}

void loop() {
    unsigned long currentTime = millis();
    
    // Create a JSON document
    StaticJsonDocument<200> doc;
    
    // Check motion sensor every second
    pirStat = digitalRead(PIR_PIN); 

    // Add motion detection status to JSON
    if (pirStat == HIGH) {
        doc["motionDetected"] = true;
        lastMotionTime = currentTime;  // Update last motion time
    } else {
        doc["motionDetected"] = false;
    }
    
    // Check if a card was scanned in the last 10 seconds
    if ((currentTime - lastCardScanTime) <= 10000) {
        doc["cardScannedInLast10Seconds"] = true;
    } else {
        doc["cardScannedInLast10Seconds"] = false;
    }

    // Update serial output only once per second
    if (currentTime - lastSerialUpdate >= serialUpdateInterval) {
        serializeJson(doc, Serial);
        Serial.println();  // Newline to separate messages
        lastSerialUpdate = currentTime;  // Update the last serial update time
    }

    // Check RFID if a card is scanned every second
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String readUID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            readUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
        }
        readUID.toUpperCase();
        
        lastCardScanTime = currentTime;  // Update the last card scan time
        
        // If the card is authorized, turn on the LED
        if (checkUID(readUID)) {
            digitalWrite(LED_PIN, HIGH);  // Turn the LED on
            ledOnTime = currentTime;      // Record when the LED was turned on
        }

        mfrc522.PICC_HaltA();  // Stop reading the card
    }

    // Turn off the LED after 2 seconds
    if (currentTime - ledOnTime >= 2000) {
        digitalWrite(LED_PIN, LOW);  // Turn off the LED
    }

    delay(100); // Small delay before the next loop iteration
}
