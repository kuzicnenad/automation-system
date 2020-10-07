/*
   Designed and developed by Nenad Kuzic

   Bachelor thesis project,
   Internet of Things, home automatation with
   Arduino Uno and ESP8266 NodeMCU

   Electrical and Computer Engineering
   Singidunum University, Serbia

   Mentor,
   prof. dr. Spalevic Petar


   ESP8266-NODEMCU PART OF PROJECT

*/
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <virtuabotixRTC.h>

/* Make serial software communication Arduino<->NodeMCU
    Serial baud rate set to 9600
    Arduino PIN 2 and 3 as TX and RX
    EPS8266 PIN D1 and D2 as TX and RX
*/
SoftwareSerial SUART(2, 3);

// Create LCD 20x04 object
LiquidCrystal_I2C lcd(0x27, 20, 4);

/* RTC module
   Wiring SCLK -> 10, I/O -> 11, CE -> 12
   Or CLK -> 10 , DAT -> 11, Reset -> 12
*/

virtuabotixRTC myRTC(10, 11, 12);

// Variables for storing sensor data (DHT11 from ESP8266)
float humidity;
float temperature;

// PINs to controll relays
uint8_t CONTROL_PIN4 = 4;
uint8_t CONTROL_PIN5 = 5;
uint8_t CONTROL_PIN6 = 6;
uint8_t CONTROL_PIN7 = 7;
uint8_t CONTROL_PIN8 = 8;
uint8_t CONTROL_PIN9 = 9;

// Chars containing date and time information
char displayDate[80];
char displayTime[80];

void setup() {
  /* Open serial communications:  */
  Serial.begin(9600);
  SUART.begin(9600);

  /* initialize LCD */
  lcd.init();
  lcd.clear();
  lcd.backlight();

  Serial.println("2019 Designed and Developed by Nenad Kuzic");
  Serial.println("Electrical and Computer Engineering");
  Serial.println("Singidunum University, Serbia");
  Serial.println();
  lcd.setCursor(0, 0);
  lcd.print("IoT Arduino based");
  lcd.setCursor(0, 1);
  lcd.print("real-time system");
  lcd.setCursor(0, 2);
  lcd.print("Developed by Kuzic N");
  lcd.setCursor(0, 3);
  lcd.print("2019-Singidunum Uni");
  delay(2000);

  Serial.println("Initializing System.");
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Initializing System");
  /*
     Set the current date, and time in the following format:
     seconds, minutes, hours, day of the week, day of the month, month, year

     set only once, comment after setting.
  */
  myRTC.setDS1302Time(30, 05, 18, 5, 23, 9, 2019);

  // Set PINs as outputs
  pinMode(CONTROL_PIN4, OUTPUT);
  pinMode(CONTROL_PIN5, OUTPUT);
  pinMode(CONTROL_PIN6, OUTPUT);
  pinMode(CONTROL_PIN7, OUTPUT);
  pinMode(CONTROL_PIN8, OUTPUT);
  pinMode(CONTROL_PIN9, OUTPUT);

  // Initialize PINs
  digitalWrite(CONTROL_PIN4, LOW);
  digitalWrite(CONTROL_PIN5, LOW);
  digitalWrite(CONTROL_PIN6, LOW);
  digitalWrite(CONTROL_PIN7, LOW);
  digitalWrite(CONTROL_PIN8, LOW);
  digitalWrite(CONTROL_PIN9, LOW);

  // Display starting message
  lcd.setCursor(1, 2);
  lcd.print("System is up!");
  delay(1000);
  lcd.clear();
}

void loop() {

  /*
      Reading date and time from
      MH-real time clock modules - 2
      and printing it on LCD

  */
  myRTC.updateTime();
  // Printing date and time to Serial
  Serial.println(displayDate);
  Serial.println(displayTime);
  // Displaying date and time to LCD
  sprintf(displayDate, "Date: %02d/%02d/%04d", myRTC.dayofmonth, myRTC.month, myRTC.year);
  sprintf(displayTime, "Time: %02d:%02d:%02d", myRTC.hours, myRTC.minutes, myRTC.seconds);

  lcd.setCursor(1, 2);
  lcd.print(displayDate);
  lcd.setCursor(1, 3);
  lcd.print(displayTime);

  // Print Temperature and Humidity on LCD
  lcd.setCursor(1, 0);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(1, 1);
  lcd.print("Temperature: ");
  lcd.print(temperature);
  lcd.print("C");

  /*
      Depending on time, information is sent to ESP8266

      q - Activates RELAY on PIN[8] and sends data to ESP8266
      w - Activates RELAY on PIN[9] and sends data to ESP8266
      o - Deactivates RELAYs and sends data to ESP8266
  */
  switch (myRTC.hours) {
    case 7:
    case 8:
    case 9:
      digitalWrite(CONTROL_PIN8, HIGH);
      SUART.print('q');
      delay(100);
      break;
    case 19:
    case 20:
    case 21:
      digitalWrite(CONTROL_PIN8, HIGH);
      SUART.print('q');
      delay(100);
      break;
    case 4:
    case 5:
    case 6:
      digitalWrite(CONTROL_PIN9, HIGH);
      SUART.print('w');
      delay(100);
      break;
    case 22:
    case 23:
    case 0:
      digitalWrite(CONTROL_PIN9, HIGH);
      SUART.print('w');
      delay(100);
      break;
    default:
      digitalWrite(CONTROL_PIN8, LOW);
      digitalWrite(CONTROL_PIN9, LOW);
      SUART.print('o');
      break;
  }

  /*
      Checks if ESP8266 is sending data. Based on character
      recived, Arduino displays humidity and temperature, or
      activates/deactivates RELAY PINs.

      DHT11 DATA
      h - Stores humidity value in variable h and print proper information to Serial
      t - Stores temperature value in variable t and print proper information to Serial

      CONTROL RELAY PINS
      PIN 4 - HIGH if temperature is less then actTemp1 entered by user
      PIN 5 - HIGH if temperature is greater then actTemp2 entered by user
      PIN 6 - HIGH if button1 on webapp is switched ON
      PIN 7 - HIGH if button2 on webapp is switched ON
      PIN 8 - HIGH if time is right
      PIN 9 - HIGH if time is right
  */
  // Checks if ESP8266 is sending data.
  while (SUART.available()) {
    char c = SUART.read();
    if (c == 'h') {
      humidity = SUART.parseFloat();
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%");
      delay(100);
    } if (c == 't') {
      temperature = SUART.parseFloat();
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println("C");
      delay(100);
    }

    /*  Checking for chars [a, b, c, d, e, f, g, i]
        char a ; HIGH > PIN4, LOW > PIN5 ; print to serial "Relay PIN[4] power ON"
        char b ; HIGH > PIN5, LOW > PIN4 ; print to serial "Relay PIN[5] power ON"
        char c ; LOW > PIN4, LOW > PIN5 ; print to serial "Temperature Relay PINS are powered OFF!"
        char d ; HIGH > PIN4, HIGH > PIN5 ; print to serial "Temperature Relay PINS are powered ON!"
        char e ; HIGH > PIN6, LOW > PIN7 ; print to serial "Button1 Relay PIN[6] ON!"
        char f ; HIGH > PIN7, LOW > PIN6 ; print to serial "Button1 Relay PIN[7] ON!"
        char g ; HIGH > PIN6, HIGH > PIN7 ; print to serial "Button Relay PINs are powered ON!"
        char i ; LOW > PIN6, LOW > PIN7 ; print to serial "Button Relay PINs are powered OFF!"
    */
    if (c == 'a') {
      digitalWrite(CONTROL_PIN4, HIGH);
      digitalWrite(CONTROL_PIN5, LOW);
      Serial.println("Relay PIN[4] power ON");
      delay(100);
    }
    if (c == 'b') {
      digitalWrite(CONTROL_PIN4, LOW);
      digitalWrite(CONTROL_PIN5, HIGH);
      Serial.println("Relay PIN[5] power ON");
      delay(100);
    }
    if (c == 'c') {
      digitalWrite(CONTROL_PIN4, LOW);
      digitalWrite(CONTROL_PIN5, LOW);
      Serial.println("Temperature Relay PINS are powered OFF!");
      delay(100);
    }
    if (c == 'd') {
      digitalWrite(CONTROL_PIN4, HIGH);
      digitalWrite(CONTROL_PIN5, HIGH);
      Serial.println("Temperature Relay PINS are powered ON!");
      delay(100);
    }
    if (c == 'e') {
      digitalWrite(CONTROL_PIN6, HIGH);
      digitalWrite(CONTROL_PIN7, LOW);
      Serial.println("Button1 Relay PIN[6] ON!");
      delay(100);
    }
    if (c == 'f') {
      digitalWrite(CONTROL_PIN6, LOW);
      digitalWrite(CONTROL_PIN7, HIGH);
      Serial.println("Button2 Relay PIN[7] ON!");
      delay(100);
    }
    if (c == 'g') {
      digitalWrite(CONTROL_PIN6, HIGH);
      digitalWrite(CONTROL_PIN7, HIGH);
      Serial.println("Button Relay PINs are powered ON!");
      delay(100);
    }
    if (c == 'i') {
      digitalWrite(CONTROL_PIN6, LOW);
      digitalWrite(CONTROL_PIN7, LOW);
      Serial.println("Button Relay PINs are powered OFF!");
      delay(100);
    }

  }

}
