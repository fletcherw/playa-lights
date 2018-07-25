#include <SoftwareSerial.h>

// Swap RX/TX connections on bluetooth chip
//   Pin 10 --> Bluetooth TX
//   Pin 9 --> Bluetooth RX
SoftwareSerial mySerial(10, 9); // RX, TX

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(1000);
}

void waitForResponse() {
    delay(1000);
    while (mySerial.available()) {
      int code = mySerial.read();
      Serial.println((char) code);
      if (code == 'P') {
        int mode = mySerial.read();
        Serial.println((char) mode);
        if (mode == '0') {
          digitalWrite(13, LOW);
          Serial.println("Turning off!");
        }
        if (mode == '1') {
          digitalWrite(13, HIGH);
          Serial.println("Turning on!");
        }
      }
    }
}

void loop() {
  waitForResponse();  
}
