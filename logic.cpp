#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Setup Software Serial for the fingerprint sensor
SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  mySerial.begin(57600);
  
  // Initialize the fingerprint sensor
  finger.begin(57600);
  delay(1000); // Give time for the sensor to initialize
  
  // Check if there are any templates to confirm sensor is working
  if (finger.getTemplateCount() == 0) {
    Serial.println("Fingerprint sensor not detected. Please check wiring.");
    while (1); // Halt the program if sensor is not found
  } else {
    Serial.println("Fingerprint sensor found!");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}


void loop() {
  Serial.println("Place your finger on the sensor.");
  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_OK) {
    Serial.println("Image taken");
  } else if (p == FINGERPRINT_NOFINGER) {
    Serial.println("No finger detected");
    delay(500);
    return;
  } else {
    Serial.println("Error reading fingerprint.");
    return;
  }

  p = finger.image2Tz();
  if (p == FINGERPRINT_OK) {
    Serial.println("Image converted");
  } else {
    Serial.println("Error converting image.");
    return;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint ID: "); Serial.print(finger.fingerID); 
    Serial.print(", Confidence: "); Serial.println(finger.confidence);
  } else {
    Serial.println("No match found.");
  }

  delay(1000);
}
