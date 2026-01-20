#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Password.h>
#include <SoftwareSerial.h>
#include <string.h>

Servo servoMotor; 

SoftwareSerial bluetooth(10, 11); 
LiquidCrystal_I2C lcd(0x27, 16, 2);

String newPasswordString;
char newPassword[6];
byte a = 5;
int attemptCounter = 0;
bool systemLocked = false;
bool awaitingActivationCode = false;
String activationCode = "";  
String keyCode = "";
Password password = Password("2000");

byte maxPasswordLength = 4;
byte currentPasswordLength = 0;
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'},
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int buzzerPin = 23; 
const int redPin = A0;  
const int greenPin = A1; 
const int bluePin = A2;  
const int motionSensorPin = 24; 

bool motionDetected = false;  

bool buzzerState = false;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  lcd.init();
  lcd.backlight(); 
  servoMotor.attach(13); 
  servoMotor.write(0);   

  pinMode(buzzerPin, OUTPUT); 
  pinMode(redPin, OUTPUT);    
  pinMode(greenPin, OUTPUT);  
  pinMode(bluePin, OUTPUT);  
  pinMode(motionSensorPin, INPUT); 

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  lcd.noBacklight();
  
  delay(2000);  
}

void loop() {
  
  int motionState = digitalRead(motionSensorPin);

  if (motionState == HIGH && !motionDetected) {
    motionDetected = true;  
    lcd.backlight();  
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("WELCOME TO");
    lcd.setCursor(0, 1);
    lcd.print("DOOR LOCK SYSTEM");
    delay(3000); 
    lcd.clear();  
  }

  if (bluetooth.available() > 0) {
    keyCode = bluetooth.readStringUntil('\n');
    if (strncmp(keyCode.c_str(), "PASS:", 5) == 0) {
      String pass = keyCode.substring(5);  // Get the substring after "PASS="
      password.set(pass.c_str());
      Serial.println(pass.c_str()); 
    }
  }

  if (systemLocked) {
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("WAITING...");
    digitalWrite(buzzerPin, HIGH); 
    digitalWrite(redPin, HIGH);   
    delay(5000);  
    awaitingActivationCode = true; 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER CODE");
    if (bluetooth.available() > 0){
      activationCode = bluetooth.readStringUntil('\n');
    }

    if (bluetooth.available() > 0) {
      keyCode = bluetooth.readStringUntil('\n'); 
      if (keyCode == "L") {
        turnOffLight(); 
        lcd.print("ENTER CODE");
      }
        
      if (keyCode == "B") {
        digitalWrite(buzzerPin,LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ENTER CODE");
      }
      // For passcode change
    }

    while (awaitingActivationCode) {
      char key = keypad.getKey();
      if (key != NO_KEY) {
        processActivationCode(key);
      }
      if (bluetooth.available() > 0) {
      keyCode = bluetooth.readStringUntil('\n'); 
      if (keyCode == "L") {
        turnOffLight(); 
        lcd.print("ENTER CODE");
      }
        
      if (keyCode == "B") {
        digitalWrite(buzzerPin,LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ENTER CODE");
      }
      // For pass code change
    }
    }
    return;
  }

  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");

  char key = keypad.getKey();
  if (key != NO_KEY) {
    delay(60);
    if (key == 'C') {
      resetPassword();
    } else {
      processNumberKey(key);
    }
  }
}

void processNumberKey(char key) {
  lcd.setCursor(a, 1);
  lcd.print("*");
  a++;
  if (a == 9) {
    a = 5;
  }
  currentPasswordLength++;
  password.append(key);

  if (currentPasswordLength == maxPasswordLength) {
    if (password.evaluate()) {
      displayCorrectPassword();
    } else {
      displayWrongPassword();
    }
  }
}

void displayCorrectPassword() {
  lcd.setCursor(0, 0);
  lcd.print("CORRECT PASSWORD");
  lcd.setCursor(0, 1);
  lcd.print("ACCESS GRANTED");
  bluetooth.println("Access Granted");

  digitalWrite(greenPin, HIGH); 
  moveServo();
  delay(2000);
  digitalWrite(greenPin, LOW);  

  resetPassword();
  attemptCounter = 0;
}

void displayWrongPassword() {
  attemptCounter++;

  if (attemptCounter <= 3) {
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD");
    lcd.setCursor(0, 1);
    lcd.print("TRY AGAIN");
    tone(buzzerPin, 1000, 500);
    digitalWrite(redPin, HIGH); 
    delay(500);
    digitalWrite(redPin, LOW); 
    delay(1500);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD");
    lcd.setCursor(0, 1);
    lcd.print("LOCKED OUT");
    digitalWrite(buzzerPin, HIGH); 
    digitalWrite(redPin, HIGH);   
    delay(1000);
    systemLocked = true;
    bluetooth.println("System Locked");
  }
  bluetooth.println("WRONG PASSWORD");
  resetPassword();
}

void resetPassword() {
  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  a = 5;
}

void processActivationCode(char key) {
  static String enteredCode = "";

  enteredCode += key;
  lcd.setCursor(enteredCode.length() - 1, 1);
  lcd.print(key);

  if (enteredCode.length() == 4) {
    if (enteredCode == activationCode) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACT.CODE CORRECT");
      lcd.setCursor(0, 1);
      lcd.print("SYSTEM UNLOCKED");
      digitalWrite(buzzerPin, LOW); 
      digitalWrite(redPin, LOW);    
      digitalWrite(greenPin, HIGH);
      moveServo();
      delay(2000);
      digitalWrite(greenPin, LOW);
      systemLocked = false;
      awaitingActivationCode = false;
      activationCode = "";
    } else {
      // tone(buzzerPin, 500, 500); 
      // digitalWrite(redPin, HIGH); 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("INVALID CODE");
      lcd.setCursor(0, 1);
      lcd.print("TRY AGAIN");
      delay(500);
      // digitalWrite(redPin, LOW);  
      //delay(1500);
    }
    enteredCode = "";
    lcd.clear();
  }
}

void moveServo() {
  servoMotor.write(90);  
  delay(15000);          
  servoMotor.write(0); 
}

void turnOffLight() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TURNING OFF LIGHT");
  digitalWrite(redPin, LOW);  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LIGHT OFF");
  lcd.clear();
  lcd.setCursor(0, 0);
}

void turnOffBuzzer(){
    digitalWrite(buzzerPin, LOW);  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BUZZER OFF");
}

void toggleBuzzer() {
  buzzerState = !buzzerState;
  if (buzzerState) {
    digitalWrite(buzzerPin, HIGH);  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BUZZER ON");
  } else {
    digitalWrite(buzzerPin, LOW);  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BUZZER OFF");
  }

  delay(1000);  
  lcd.clear();
  lcd.setCursor(0, 0);
}