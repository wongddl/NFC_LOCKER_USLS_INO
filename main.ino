#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Servo.h>

// Define constants
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
Servo Servo1;
String myPasscode = "hi";
int SOLENOID = 13;
int RED = 12;
int GREEN = 11;
int YELLOW = 10; // Pin for the CHECKER LED
int SERVOPIN = 9;
const unsigned long intervalState0 = 400; // x seconds for state 0 //change here jp
const unsigned long intervalState1 = 5000; // y seconds for state 1 //change here jp

// Variables
int stateValue = 0; // Current state value
unsigned long previousMillis = 0; // Store the last time state was updated

void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(YELLOW, OUTPUT); // Set the LED pin as output
  pinMode(SOLENOID, OUTPUT); // Set the solenoid pin as output
  pinMode(RED, OUTPUT); // Set the red LED pin as output
  pinMode(GREEN, OUTPUT); // Set the green LED pin as output
  Servo1.attach(SERVOPIN); // Attach Servo to pin 9
  
  digitalWrite(SOLENOID,HIGH); // Set initial state of Solenoid to OFF
  Servo1.write(45); // Set initial position to 45 degrees
  nfc.begin();
}

void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  unsigned long currentInterval = (stateValue == 0) ? intervalState0 : intervalState1;

  if (currentMillis - previousMillis >= currentInterval) {
    previousMillis = currentMillis; // Update the last time

    // Update the state value
    stateValue = (stateValue + 1) % 2; // Toggle between state 0 and 1

    // Print the current state value for debugging
    Serial.print("State: ");
    Serial.println(stateValue);

    // Code for each state
    if (stateValue == 0) {
      Serial.println("State 0: Delay in progress"); // Example action for state 0
      digitalWrite(YELLOW, LOW); // Turn off the LED
      Servo1.write(135); // Move Servo to 45 degrees
      scanAtonNFC();

    } else if (stateValue == 1) {
      Serial.println("State 1: Scanning NFC"); // Example action for state 1
      digitalWrite(YELLOW, HIGH); // Turn on the LED
      Servo1.write(45); // Move Servo to 135 degrees
    }
  }
}

void scanAtonNFC() {
  Serial.println("Waiting for an NFC card...");
  String currentInput = "";

  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();

    // Displaying UID
    Serial.print("Card UID: ");
    Serial.println(tag.getUidString()); // Print UID as a string

    // Reading NDEF Data
    if (tag.hasNdefMessage()) {
      Serial.println("NDEF Message found:");
      NdefMessage message = tag.getNdefMessage();
      currentInput = getNdef(message);

      Serial.print("currentInput: ");
      Serial.println(currentInput); // Print the content of the NFC tag
      Serial.print("myPasscode: ");
      Serial.println(myPasscode);

      if (currentInput == myPasscode) {
        Serial.println("Access Granted");
        digitalWrite(SOLENOID, LOW);
        digitalWrite(GREEN, HIGH);
        unsigned long solenoidStart = millis();
        while (millis() - solenoidStart < 3000) {
          // Non-blocking delay
        }
        digitalWrite(SOLENOID, HIGH);
        digitalWrite(GREEN, LOW);
      } else {
        Serial.println("Access Denied");
        digitalWrite(RED, HIGH);
        unsigned long redStart = millis();
        while (millis() - redStart < 3000) {
          // Non-blocking delay
        }
        digitalWrite(RED, LOW);
      }
    } else {
      // Handle if no NDEF message is found
      Serial.println("No NDEF Message found on the card.");
    }
  } else {
    // Handle when no card is detected
    Serial.println("No NFC card detected.");
  }
}

String getNdef(NdefMessage message) {
  String currentInput = "";  // Initialize currentInput to store the concatenated string

  // Loop through all the records in the message
  for (int i = 0; i < message.getRecordCount(); i++) {
    NdefRecord record = message.getRecord(i);
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);

    Serial.println(payloadLength);


    Serial.print("Record ");
    Serial.print(i + 1);
    Serial.print(": ");

    // Loop through the payload and concatenate characters into currentInput
    for (int j = 0; j < payloadLength; j++) {
      currentInput.concat((char)payload[j]);
    }
    currentInput = currentInput.substring(3);
  }

  return currentInput;  // Return the concatenated string after processing all records
}
