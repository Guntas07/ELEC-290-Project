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
int debounceDelay = 3000;          // Delay to debounce sensor (in milliseconds)
int rfidReady = false;             // Flag to indicate system is ready to read RFID
unsigned long rfidTimeout = 5000;  // How long to wait for RFID after motion (milliseconds)
String authorizedUIDs[] = {"A3FDF04F", "33F0F002"};  // Array of authorized UIDs

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
    pirStat = digitalRead(PIR_PIN); 
    unsigned long currentTime = millis();

    // Create a JSON document
    StaticJsonDocument<200> doc;
    
    if (pirStat == HIGH && (currentTime - lastMotionTime) > debounceDelay) {
        lastMotionTime = currentTime; // Update the last motion time
        rfidReady = true; // Set the system as ready to read RFID
        
        doc["motionDetected"] = true;
        doc["message"] = "Motion Detected!";
        Serial.println("Motion Detected!");
    } else {
        doc["motionDetected"] = false;
    }

    if (rfidReady && (currentTime - lastMotionTime) < rfidTimeout) {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
            String readUID = "";
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                readUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
            }
            readUID.toUpperCase();

            doc["cardUID"] = readUID;  // Add the scanned UID to the JSON document
            Serial.print("Card UID: ");
            Serial.println(readUID);

            if (checkUID(readUID)) {
                doc["accessGranted"] = true;
                doc["accessStatus"] = "Authorized access";
                digitalWrite(LED_PIN, HIGH);   // turn LED ON
                delay(5000);                   // keep LED ON for 5 seconds
                digitalWrite(LED_PIN, LOW);    // turn LED OFF
            } else {
                doc["accessGranted"] = false;
                doc["accessStatus"] = "Unauthorized access";
            }
            mfrc522.PICC_HaltA();  // Stop reading the card
            rfidReady = false; // Reset RFID ready state
        }
    } else if (currentTime - lastMotionTime >= rfidTimeout) {
        rfidReady = false; // Reset RFID ready state if timeout has passed
    }

    if (!rfidReady) {
        digitalWrite(LED_PIN, LOW); // Ensure LED is off if no motion detected
    }

    // Serialize the JSON object to the serial monitor
    serializeJson(doc, Serial);
    Serial.println();

    delay(100); // Small delay before the next loop iteration
}
