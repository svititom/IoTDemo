/*
 * Code adapted from the following examples:
 * WiFiServer
 * BMETest
 * 
 * If you wish to run this, you must have the following installed
 * Arduino Core for ESP32 (alt 8266)
 * Adafruit BME280 library
 * Adafruit Sensor library
 * 
 * If you wish to use another sensor,
 * simply change the librares and update startBme and printBmeStats accordingly
 * 
 */

//for WiFi and sensor functionality
#include <WiFi.h>

//for BME280 sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define BME_SCK 19
#define BME_MISO 18
#define BME_MOSI 5
#define BME_CS 17
#define SEALEVELPRESSURE_HPA (1013.25)

//name of the access point which will be created
#define AP_SSID "IoT_Demo"

/* use this if you want to connect to an existing network */
#define ssid "********"
#define password "********"

//the server which actually sends the website
WiFiServer server(80);

//definitions for bme280
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
boolean bmeStarted = false;
String mainBmeLoc = "LT7";


void startWiFiAndServer(){
    Serial.println();
    Serial.println();
    Serial.print("Starting AP ");
    Serial.println(AP_SSID);

    //create access point
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(AP_SSID);
    WiFi.begin();


    //connect to existing network
//    WiFi.mode(WIFI_MODE_STA);
//    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    //IP when connected to network
    //Serial.println(WiFi.localIP);

    //IP when access point
    Serial.println(WiFi.softAPIP());
    
    server.begin();
  
}


void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode
    
    delay(10);
    startWiFiAndServer();
    // We start by connecting to a WiFi network


}


void startBme()
{
  bmeStarted = bme.begin();    
}

String printBmeStats()
{
 if (!bmeStarted){
      startBme(); //startBme toggles bmeStrated, so if it isn't started with this, bmeStarted stays false
 }  //make sure it was started
 if (!bmeStarted){
    return "No temperature sensor found!";
  } 
  //build the enviromental data string
  String stats = "<br><br>Location: ";
  stats += mainBmeLoc;
  stats += "<br>";

  stats += "Temperature: ";
  stats += bme.readTemperature();
  stats += "°C<br>";

  stats += "Humidity: ";
  stats += bme.readHumidity();
  stats += " %RH<br>";

  stats += "Pressure: ";
  stats += bme.readPressure() / 100.0F;
  stats += " hPa<br>";

  stats += "Aproximate altitude: ";
  stats += bme.readAltitude(SEALEVELPRESSURE_HPA);
  stats += " m<br>";
  return stats;
}

String printAdvertising()
{
  String advert = "<br><br>Source Code can be found at: <a href=\"https://github.com/svititom/IoTDemo\">Github</a><br>";
  advert +=  "More info can be found at: <a href=\"http://distributedweather.mzf.cz/post/show?postId=11\">Distributed Weather</a>";
  return advert;
}

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,

   
    
    Serial.println("new client");           // print a message out the serial port
    
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> turn the LED on pin 5 on<br>");
            client.print("Click <a href=\"/L\">here</a> turn the LED on pin 5 off<br>");
            
            client.print(printBmeStats());  //Print the data from the sensor

            client.print(printAdvertising()); //Print advertising
          
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
