/*
   Designed and developed by Nenad Kuzic

   Bachelor thesis project,
   Internet of Things, Automatation with
   Arduino Uno and ESP8266 NodeMCU

   Electrical and Computer Engineering
   Singidunum University, Serbia

   Mentor,
   prof. dr. Spalevic Petar


   ESP8266-NODEMCU PART OF PROJECT


   Important notice:

   Create software serial communication.
   ESP8266 => Pin D7 & Pin D8 as RX and TX pins,
   Connecting to Arduino pin 2 & 3 as RX and TX,
   Arduino Uno and ESP8266 use 9600 baud rate.

*/
#include "DHT.h"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <DNSServer.h>
#include <WiFiManager.h>

#define DHTTYPE DHT11   // DHT 11

/* Local network SSID & Password */
const char* ssid = "Tenda_C6E6D0";
const char* password = "Ambient1112";

/* Set web server port to 80*/
ESP8266WebServer server(80);
/* Put IP Address details for static
  IPAddress local_ip(192,168,1,10);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);
*/
/* Initialize DHT sensor. */
uint8_t DHTPin = D5;  // pin
DHT dht(DHTPin, DHTTYPE);

float temperature;
float humidity;
float actTemp1 = 23; // default value for temperature system activation
float actTemp2 = 26; // default value for temperature system activation
bool LED6status = false;
bool LED7status = false;
bool LED8status = false;  // PIN 8 for TIME activation from ARDUINO
bool LED9status = false;  // PIN 9 for TIME activation from ARDUINO

/* Make serial software communication Arduino<->NodeMCU */
SoftwareSerial SUART(D1, D2);

void setup() {
  /* Open serial communications:  */
  Serial.begin(9600);
  SUART.begin(9600);

  Serial.println();
  Serial.println("2019 Designed and Developed by Nenad Kuzic");
  Serial.println("Electrical and Computer Engineering");
  Serial.println("Singidunum University, Serbia");
  Serial.println();

  pinMode(DHTPin, INPUT);
  dht.begin();
  /* wifi menager */
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  Serial.println("Opening configuration portal");
  //Local intialization. Once its business is done, there is no need to keep it around

  WiFiManager wifiManager;
  //sets timeout in seconds until configuration portal gets turned off.
  //If not specified device will remain in configuration mode until
  //switched off via webserver.
  if (WiFi.SSID() != "") wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.

  //it starts an access point
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.startConfigPortal("ESP8266", "password")) //Delete these two parameters if you do not want a WiFi password on your configuration access point
  {
    Serial.println("Not connected to WiFi but continuing.");
  }
  else
  {
    //if you get here you have connected to the WiFi
    Serial.println("Connected!");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("failed to connect, finishing setup");
  }
  else
  {
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }

  /* Join local Wi-Fi network
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    // WiFi.config(local_ip, gateway, subnet); // if static

    Check if NodeMCU is connected to WiFi network by checking WiFi status
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected to local network!");
    Serial.print("NodeMCU IP Address: ");
    Serial.println(WiFi.localIP());
  */
  
  /*
      Handle HTTP request when URL is hit.
      To specify which function to execute,
      we use ON method.
  */
  // webapp root
  server.on("/", handle_OnConnect); 
  // Handle FORM, take temerature input
  server.on("/action_page", handleForm); 
  // turn ON LED and Relay on PIN[6]
  server.on("/led6_ON", handle_led6_ON); 
  // turn OFF LED and Relay on PIN[6]
  server.on("/led6_OFF", handle_led6_OFF);
  // turn ON LED and Relay on PIN[7]
  server.on("/led7_ON", handle_led7_ON);
  // turn OFF LED and Relay on PIN[7]
  server.on("/led7_OFF", handle_led7_OFF);
  // no connection, wrong path
  server.onNotFound(handle_NotFound);

  // Starts HTTP server
  server.begin();
  Serial.println("Server is up and running!");
  delay(2000);
}

void loop() {
  // Handle HTTP requests
  server.handleClient();

  // Store DHT11 values to variables 
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  /* Serial monitor humidity and temperature
    Serial.print("Humidity(%): ");
    Serial.println((float)humidity, 2);
    Serial.print("Temperature(C): ");
    Serial.println((float)temperature, 2);
  */

  /* Calling functions to send DHT11 data 
  temperature and humidity to arduino */
  sendHumidity();
  sendTemperature();

  /*
   * Depending on temperature and desired temperature value
   * proper data is sent to Arduino.
   */
  if ((temperature < actTemp1) && (temperature < actTemp2)) {
    systemStatus1();
  } else if ((temperature > actTemp2) && (temperature > actTemp1)) {
    systemStatus2();
  } else if ((temperature < actTemp2) && (temperature > actTemp1)) {
    systemStatus3();
  } else if ((temperature > actTemp2) && (temperature < actTemp1)) {
    systemStatus4();
  }

  if (LED6status && !LED7status) {
    systemStatus5();
  } else if (LED7status && !LED6status) {
    systemStatus6();
  } else if ((LED6status) && (LED7status)) {
    systemStatus7();
  } else if (!(LED6status) && !(LED7status)) {
    systemStatus8();
  }


  while (SUART.available()) {
    char c = SUART.read();
    if (c == 'q') {
      Serial.println("Time Sys PIN[8] is ON!");
      LED8status = true;
      LED9status = false;
      delay(50);
    } else if (c == 'w') {
      Serial.println("Time Sys PIN[9] is ON!");
      LED8status = false;
      LED9status = true;
      delay(50);
    }  else if (c == 'o') {
      Serial.println("Time Systems are OFF!");
      LED8status = false;
      LED9status = false;
      delay(50);
    }
  }
}

void sendHumidity() {
  //SUART.print("Humidity: ");
  SUART.print('h');
  SUART.println((float)humidity, 2);
  delay(100);
}

void sendTemperature() {
  //SUART.print("Temperature: ");
  SUART.print('t');
  SUART.println((float)temperature, 2);
  delay(100);
}
/* Depending on temperature appropriate message is sent to Arduino
    1 - System 1 is activated
    b - System 2 is activated
    c - Temperature Systems are on
    d - Temperature Systems are off
*/
void systemStatus1() {
  SUART.print('a');
  delay(100);
}
void systemStatus2() {
  SUART.print('b');
  delay(100);
}
void systemStatus3() {
  SUART.print('c');
  delay(100);
}
void systemStatus4() {
  SUART.print('d');
  delay(100);
}
void systemStatus5() {
  SUART.print('e');
  delay(100);
}
void systemStatus6() {
  SUART.print('f');
  delay(100);
}
void systemStatus7() {
  SUART.print('g');
  delay(100);
}
void systemStatus8() {
  SUART.print('i');
  delay(100);
}

void handle_OnConnect() {
  /*
     Respond to HTTP request with SEND method.
     SendHTML - function which createds our page.
  */
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

/* Function to handle data for temperature from webapp */
void handleForm() {
  Serial.print("PIN[4] will activate on: ");
  actTemp1 = (server.arg("systemTemp1")).toFloat();
  Serial.print(actTemp1);
  Serial.println("(C)");
  Serial.print("PIN[5] will activate on: ");
  actTemp2 = (server.arg("systemTemp2")).toFloat();
  Serial.print(actTemp2);
  Serial.println("(C)");

  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

void handle_led6_ON() {
  Serial.println("Button LED6: ON");
  LED6status = true;
  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

void handle_led6_OFF() {
  Serial.println("Button LED6: OFF");
  LED6status = false;
  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

void handle_led7_ON() {
  Serial.println("Button LED7: ON");
  LED7status = true;
  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

void handle_led7_OFF() {
  Serial.println("Button LED7: OFF");
  LED7status = false;
  server.send(200, "text/html", SendHTML(temperature, humidity, LED6status, LED7status));
}

/* Function to handle errors */
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}


String SendHTML(float temperature, float humidity, bool led6stat, bool led7stat) {
  String s = "<!DOCTYPE html> <html>\n";

  s += "<head>\n";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  /* s +="<meta http-equiv=\"refresh\" content=\"5\" >\n"; <- meta tag to autorefresh page every 5s */
  /* Google fonts for css style */
  s += "<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  s += "<title> Kuzic Nenad </title>\n";

  /* CSS styling web page */
  s += "<style>html {margin: 0;padding: 0;font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;}\n";
  s += "html{background-color: #DCDCDC;color: #333333;}\n";
  s += "body{margin: 0 auto;padding: 0;background-color: #DCDCDC;}\n";
  s += "h1 {margin: 0 auto;color: white;}\n";
  s += ".headerTop{padding-top: 12px; padding-bottom: 12px; border: solid 2px black;background-color: #34495e;color: white;}\n";
  s += ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  /* Sensor icons and text */
  s += ".humidity-icon{background-color: #3498db;width: 40px;height: 40px;border-radius: 50%;line-height: 48px;}\n";
  s += ".humidity-text{font-weight: 600;padding-left: 12px;font-size: 20px;width: 160px;text-align: left;}\n";
  s += ".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";

  s += ".temperature-icon{background-color: #f39c12;width: 40px;height: 40px;border-radius: 50%;line-height: 48px;}\n";
  s += ".temperature-text{font-weight: 600;padding-left: 12px;font-size: 20px;width: 160px;text-align: left;}\n";
  s += ".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";

  s += ".dataText{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
  s += ".data{padding: 2px;}\n";

  /* Styling LED status */
  s += ".led-green {background-color: #228B22; width: 20px;height: 20px;border-radius: 50%;}\n";
  s += ".led-red {background-color: #FF0000; width: 20px;height: 20px;border-radius: 50%;}\n";
  /* LED texts */
  s += ".led-text{font-weight: 600;padding-right: 2px;font-size: 16px;width: auto;text-align: left;}\n";

  /* FORM style*/
  s += "form{text-align: center;width: auto;padding-bottom: 10px;border-top: solid 2px black;border-bottom: solid 2px black;}\n";
  s += "form p{font-weight: 600;font-size: 20px;}\n";
  s += ".formInput{font-weight: 400;font-size: 18px;width: 60px;border: none;border-bottom: solid 1px black;background-color: #DCDCDC;text-align: left;}\n";
  s += ".formInput:hover{border-bottom: solid 1px #f39c12;}\n";
  s += ".formButton{font-weight: 600;padding: 10px;font-size: 20px;width: 360px;float: middle;text-align: center;cursor: pointer;background-color: #34495e;color: white;border: none;}\n";
  s += ".formButton:hover{background-color: #f39c12;}\n";

  /* Copyrights, footer */
  s += ".copy{vertical-align: middle; position: relative; background-color: #DEB887; border: solid 2px black;background-color: #34495e;}\n";
  s += ".copy-text{font-weight: 600; padding-top: 12px;padding-bottom: 12px;font-size: 16px;font-style: italic;color: white;}\n";

  /* CSS for buttons */
  s += ".button {display: block;width: 160px;background-color: #1abc9c;border: none;color: white;padding: 10px;text-decoration: none;font-size: 20px;margin: 0px auto 35px;cursor: pointer;}\n";
  s += ".button-on {background-color: #1abc9c;}\n";
  s += ".button-on:active {background-color: #16a085;}\n";
  s += ".button-off {background-color: #34495e;}\n";
  s += ".button-off:active {background-color: #2c3e50;}\n";
  s += ".buttonClickable:hover{background-color: #f39c12;}\n";

  s += "</style>\n";

  /*
      JavaScript, AJAX
     Function setInterval() is used to refresh web page.
     Inside function lodaDoc() XMLHttp object is created.
     This object requests data from a web server.

  */
  s += "<script>\n";
  s += "setInterval(loadDoc,10000);\n";

  s += "function loadDoc() {\n";
  s += "var httpRequest = new XMLHttpRequest();\n";
  s += "httpRequest.onreadystatechange = function() {\n";
  s += "if (this.readyState == 4 && this.status == 200) {\n";
  s += "document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  s += "};\n";

  s += "httpRequest.open(\"GET\", \"/\", true);\n";
  s += "httpRequest.send();\n";
  s += "}\n";
  s += "</script>\n";


  s += "</head>\n";

  /* Showing content on webpage */
  s += "<body>\n";

  s += "<div id=\"webpage\">\n";

  s += "<div class=\"headerTop\">\n";
  s += "<h1> Embedded Internet of Things </h1>\n";
  s += "<pclass=\"copy-text copy\"> Automation with Arduino Uno and ESP8266 </p>\n";
  s += "</div>\n";
  /*
     SVG(Scalable Vector Graphics) for icons.
     Using Google SVG Editor for less effort
     After the icons we are showing values from DHT11
  */
  s += "<div class=\"data\">\n";
  s += "<div class=\"side-by-side temperature-icon\">\n";
  s += "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
  s += "width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
  s += "<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
  s += "c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
  s += "c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
  s += "c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
  s += "c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
  s += "</svg>\n";
  s += "</div>\n";
  /* Showing temperature and humidity readings */
  s += "<div class=\"side-by-side temperature-text\">Temperature</div>\n";
  s += "<div class=\"side-by-side temperature\">";
  s += (int)temperature;
  s += "<span class=\"dataText\">C</span></div>\n";
  s += "</div>\n";
  s += "<div class=\"data\">\n";
  s += "<div class=\"side-by-side humidity-icon\">\n";
  s += "<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
  s += "<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
  s += "c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
  s += "</svg>\n";
  s += "</div>\n";
  s += "<div class=\"side-by-side humidity-text\">Humidity</div>\n";
  s += "<div class=\"side-by-side humidity\">";
  s += (int)humidity;
  s += "<span class=\"dataText\">%</span></div>\n";
  s += "</div>\n";

  /* Form to enter values on which PIN4 AND PIN4 temperature will power ON */
  s += "<div class=\"data\">\n";
  s += "<form action=\"/action_page\">\n";
  s += "  <p>Enter desired room temperature: </p>\n";
  s += "  <p class=\"side-by-side\">Minimum temperature[PIN4] </p><input class=\"formInput side-by-side\" type=\"number\" min=\"10\" name=\"systemTemp1\" required>\n";
  s += " <br> \n";
  s += "  <p class=\"side-by-side\">Maximum temperature[PIN5] </p><input class=\"formInput side-by-side\" type=\"number\" min=\"10\" name=\"systemTemp2\" required>\n";
  s += " <br> \n";
  s += "  <input class = \"formButton\"type=\"submit\" value=\"Submit\">\n";
  s += "</form> \n";
  s += "</div>\n";

  /* Buttons to switch RELAY 3 and RELAY 4 ON and OFF
      PIN6 AND PIN7
  */
  s += "<div class=\"data\">\n";
  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (led6stat)
  {
    s += "<p class=\"side-by-side led-text\">PIN[6] Status: ON </p><div class=\"side-by-side led-green\"></div><a class=\"side-by-side buttonClickable button button-off\" href=\"led6_OFF\">Turn OFF</a>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[6] Status: OFF </p><div class=\"side-by-side led-red\"></div><a class=\"side-by-side buttonClickable button button-on\" href=\"led6_ON\">Turn ON</a>\n";
  }
  s += "</div>\n";

  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (led7stat)
  {
    s += "<p class=\"side-by-side led-text\">PIN[7] Status: ON </p><div class=\"side-by-side led-green\"></div><a class=\"side-by-side buttonClickable button button-off\" href=\"led7_OFF\">Turn OFF</a>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[7] Status: OFF </p><div class=\"side-by-side led-red\"></div><a class=\"side-by-side buttonClickable button button-on\" href=\"led7_ON\">Turn ON</a>\n";
  }
  s += "</div>\n";
  s += "</div>\n";

  /* TEMPERATURE BASED RELAYS Showing LED stauts to display working system  */
  s += "<div class=\"data\">\n";
  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (temperature < actTemp1)
  {
    s += "<p class=\"side-by-side led-text\">PIN[4] Status: ON </p><div class=\"side-by-side led-green\"></div><p class=\"side-by-side button button-on\">Min temp ";
    s += (int)actTemp1;
    s += "</p>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[4] Status: OFF </p><div class=\"side-by-side led-red\"></div><p class=\"side-by-side button button-off\">Min temp ";
    s += (int)actTemp1;
    s += "</p>\n";
  }
  s += "</div>\n";

  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (temperature > actTemp2)
  {
    s += "<p class=\"side-by-side led-text\">PIN[5] Status: ON </p><div class=\"side-by-side led-green\"></div><p class=\"side-by-side button button-on\">Max temp ";
    s += (int)actTemp2;
    s += "</p>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[5] Status: OFF </p><div class=\"side-by-side led-red\"></div><p class=\"side-by-side button button-off\">Max temp ";
    s += (int)actTemp2;
    s += "</p>\n";
  }
  s += "</div>\n";
  s += "</div>\n";

  /* TIME BASED RELAYS Showing LED stauts to display working system  */
  s += "<div class=\"data\">\n";
  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (LED8status)
  {
    s += "<p class=\"side-by-side led-text\">PIN[8] Status: ON </p><div class=\"side-by-side led-green\"></div><p class=\"side-by-side button button-on\">ONLINE ";
    s += "</p>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[8] Status: OFF </p><div class=\"side-by-side led-red\"></div><p class=\"side-by-side button button-off\">OFFLINE ";
    s += "</p>\n";
  }
  s += "</div>\n";

  s += "<div class=\"buttonSwitch side-by-side\">\n";
  if (LED9status)
  {
    s += "<p class=\"side-by-side led-text\">PIN[9] Status: ON </p><div class=\"side-by-side led-green\"></div><p class=\"side-by-side button button-on\">ONLINE ";
    s += "</p>\n";
  }
  else
  {
    s += "<p class=\"side-by-side led-text\">PIN[9] Status: OFF </p><div class=\"side-by-side led-red\"></div><p class=\"side-by-side button button-off\">OFFLINE ";
    s += "</p>\n";
  }
  s += "</div>\n";
  s += "</div>\n";


  /* COPYRIGHTS FOOTER */
  s += "<div class=\"copy-text copy\">\n";
  s += "<p>Designed and Developed by &copy; Nenad Kuzic 2019</p>\n";
  s += "<p>Singidunum University Electrical and Computer Engineering</p>\n";
  s += "</div>\n";

  s += "</div>\n";

  s += "</body>\n";
  s += "</html>\n";

  return s;
}
