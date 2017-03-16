/*
   Code adapted from the following examples:
   WiFiServer
   BMETest

   If you wish to run this, you must have the following installed
   Arduino Core for ESP32 (alt 8266)
   Adafruit BME280 library
   Adafruit Sensor library
   LEDCSoftwareFade

   If you wish to use another sensor,
   simply change the librares and update startBme and printBmeStats accordingly

*/

//for WiFi and sensor functionality
#include <WiFi.h>

//for BME280 sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define BME_SCK 17
#define BME_MISO 5
#define BME_MOSI 18
#define BME_CS 19
#define SEALEVELPRESSURE_HPA (1013.25)

//name of the access point which will be created
#define AP_SSID "IoT_Demo"

/* use this if you want to connect to an existing network */
#define ssid "********"
#define password "********"

//If you want to add more LEDs, don't forget to add the timer channels in startLedTimer
#define LEDOne 23
#define LEDTwo 22
//LEDs are dimmed with their timer channels and not the actual pins
#define LEDOneChan 0
#define LEDTwoChan 1

//the duty cycle is how long the pwm signal is up, its 8bit, thus 90 => 90/255 = ~35%
#define Duty 90
#define LEDC_BASE_FREQ 5000


//the server which actually sends the website
WiFiServer server(80);

//definitions for bme280
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
boolean bmeStarted = false;
String mainBmeLoc = "LT7";


void startWiFiAndServer() {
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
  //currently analogWrite is not implemented, so we use PWM via a timer instead (See LEDCSoftwareFade)
  //I didn't have a resistor on hand so use this instead
  startLedTimer();

  // We start by connecting to a WiFi network


}

void startLedTimer()
{
  int channelOne = 0; //channel of the timer
  int channelTwo = 1;
  int timerPrecision = 13; //bits
  ledcSetup(LEDOneChan, LEDC_BASE_FREQ, timerPrecision);
  ledcAttachPin(LEDOne, LEDOneChan);
  ledcSetup(LEDTwoChan, LEDC_BASE_FREQ, timerPrecision);
  ledcAttachPin(LEDTwo, LEDTwoChan);
}
// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 120) {
  // calculate duty
  //min not recognized for some reason, forums suggested just use _min
  uint32_t duty = (LEDC_BASE_FREQ / valueMax) * _min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void startBme()
{
  bmeStarted = bme.begin();
}

String printBmeStats()
{
  if (!bmeStarted) {
    startBme(); //startBme toggles bmeStrated, so if it isn't started with this, bmeStarted stays false
  }  //make sure it was started
  if (!bmeStarted) {
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

void loop() {
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
            client.print("Turn Red LED  <a href=\"/23/H\">on</a> or <a href=\"/23/L\">off</a>?<br>");
            client.print("Turn Blue LED <a href=\"/22/H\">on</a> or <a href=\"/22/L`\">off</a>?<br>");

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
        //TODO Find a better way to route the requests ...
        if (currentLine.endsWith("GET /23/H")) {
          ledcAnalogWrite(LEDOneChan, 90);
          Serial.println("Turning LED On");
          //          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /23/L")) {
          ledcAnalogWrite(LEDOneChan, 0);
          Serial.println("Turning led off");
          //          digitalWrite(5, LOW);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /22/H")) {
          ledcAnalogWrite(LEDTwoChan, 90);
        }
        if (currentLine.endsWith("GET /22/L")) {
          ledcAnalogWrite(LEDTwoChan, 0);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}


