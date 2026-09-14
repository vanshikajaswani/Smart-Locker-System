#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WebServer.h>
WebServer server(80);

// 🔐 WiFi
const char* ssid = "Hi";
const char* password = "nhidenanet";

// 💻 YOUR SERVER
String serverURL = "http://10.65.167.1:5000/capture";

// 🔐 LOCK PIN
int lockPin = 4;

// 📟 LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 🔢 Keypad
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 🔑 OTP
String enteredPassword = "";
String generatedOTP = "";
unsigned long otpTime = 0;
unsigned long otpValidity = 60000; // 60 seconds

// ✅ Allowed digits (NO 2, NO 5)
char allowedDigits[] = {'0','1','3','4','6','7','8','9'};

// 🔵 FUNCTION TO GENERATE OTP
void generateOTP() {
  generatedOTP = "";

  for (int i = 0; i < 4; i++) {
    int index = random(0, 8);
    generatedOTP += allowedDigits[index];
  }
  otpTime = millis();
  Serial.print("Generated OTP: ");
  Serial.println(generatedOTP);
}

void setup() {
  
  Serial.begin(115200);
  Serial.println(WiFi.localIP());
  // 🔐 Lock setup
  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
Serial.print("ESP32 IP: ");
Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.print("WiFi Connected");
  Serial.print("ESP32 IP: ");
Serial.println(WiFi.localIP());
  delay(1000);
  server.on("/generate", []() {
  generateOTP();

  HTTPClient http;
  String url = "http://10.65.167.1:5000/send_otp/" + generatedOTP;

  http.begin(url);
  http.GET();
  http.end();

  server.send(200, "text/plain", "OTP Generated");
});

server.begin();
  // 🔥 Generate OTP
  randomSeed(micros());
  generateOTP();
  // 🔥 SEND OTP TO TELEGRAM
HTTPClient http;
String otpURL = "http://10.240.155.1:5000/send_otp/" + generatedOTP;

http.begin(otpURL);
int code = http.GET();

Serial.print("OTP Sent, Response: ");
Serial.println(code);

http.end();
  lcd.clear();
  lcd.print("Enter OTP:");
}

void loop() {
  char key = keypad.getKey();
server.handleClient();
  if (key) {
    enteredPassword += key;

    lcd.setCursor(0,1);
    lcd.print(enteredPassword);

    if (enteredPassword.length() == 4) {
      delay(500);
      lcd.clear();

       if (millis() - otpTime > otpValidity) {

  lcd.print("OTP Expired");
  Serial.println("OTP Expired");
  delay(2000);

  generateOTP();  // new OTP

} 
else if (enteredPassword == generatedOTP) {

        lcd.print("Access Granted");
        Serial.println("Correct OTP");

        // 🔓 UNLOCK
digitalWrite(lockPin, HIGH);
delay(5000);

// 🔒 LOCK FIRST
digitalWrite(lockPin, LOW);
delay(1000);  // small delay for stability

// 📸 TAKE PHOTO AFTER LOCK
Serial.println("Sending request...");
HTTPClient http;
http.begin(serverURL);
int code = http.GET();
Serial.print("Response: ");
Serial.println(code);
http.end();
        delay(2000);

        // 🔒 LOCK AGAIN
        digitalWrite(lockPin, LOW);

        // 🔥 GENERATE NEW OTP (expire old one)
        generateOTP();

      } else {
        lcd.print("Wrong OTP");
        Serial.println("Wrong OTP");
        delay(2000);
      }

      enteredPassword = "";
      lcd.clear();
      lcd.print("Enter OTP:");
    }
  }
}
