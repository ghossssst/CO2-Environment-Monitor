//libraries
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

// network credentials
const char* ssid = "****";
const char* password = "****";

//create webserver on port 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Setup Adafruit_SCD30
Adafruit_SCD30  scd30;

// OLED display width, in pixels
#define SCREEN_WIDTH 128 
// OLED display height, in pixels
#define SCREEN_HEIGHT 32 

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



void setup(void) {
  //Start the serial output at 115,200 baud
  Serial.begin(115200);

  //Connect to the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //Connect to the OLED display
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  //Clear display
  display.clearDisplay(); 
  //Set text size
  display.setTextSize(1); 
  //Set text colour
  display.setTextColor(WHITE); 
  //Set text position 
  display.setCursor(0, 0);
  //Set text
  display.println("Ghosts Air Sensor");
  //Display text
  display.display(); 
  //Wait 2 seconds
  delay(2000); 

  //Initialise SCD30
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 chip");
    while (1) { delay(10); }
  }

  //Clear display
  display.clearDisplay();

  Serial.println("SCD30 Found!");
  // Connect to Wi-Fi network with SSID and password
  display.setCursor(0, 0);
  Serial.print("Connecting to ");
  display.println("Connecting to: ");
  Serial.println(ssid);
  display.println(ssid);
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Serial print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}



void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();

  if (scd30.dataReady()){
  Serial.println("Data available!");

    if (!scd30.read()){ Serial.println("Error reading sensor data"); return; }
    //Display readings on serial monitor
    Serial.print("Temp: ");                                            
    Serial.println(scd30.temperature);
    Serial.print("Humidity: ");
    Serial.println(scd30.relative_humidity);
    Serial.print("CO2: ");
    Serial.println(scd30.CO2, 2);
  } else {
    //Serial.println("No data");
  }

  //Update text on display to show current readings
  display.clearDisplay();
  display.setTextSize(1);
  //Display temp
  display.setCursor(0,0);
  display.println("Temperature: ");
  display.setCursor(75,0);
  display.print(scd30.temperature);
  display.println(" C");
  //Display humidity
  display.setCursor(0,8);
  display.println("Humidity: ");
  display.setCursor(57,8);
  display.print(scd30.relative_humidity);
  display.println(" %");
  //Display Co2
  display.setCursor(0,16);
  display.println("Co2: ");
  display.setCursor(27,16);
  display.print(scd30.CO2, 2);
  display.println(" ppm");
  //Display IP
  display.setCursor(0,24);
  display.println("IP: ");
  display.setCursor(20,24);
  display.print(WiFi.localIP());
  display.display(); 

  // If a new client connects,
  if (client) {
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // Display the HTML web page
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 2");  // refresh the page automatically every 1 sec
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Web Page Heading
            client.println("<body><h1>ESP32 Environment Sensor Web Server</h1>");
            //Print temperature
            client.print("Temperature: ");
            client.print(scd30.temperature);
            client.print("C");
            client.println("<br />");
            //Print Relative Humidity
            client.print("Relative Humidity: ");
            client.print(scd30.relative_humidity);
            client.print("%");
            client.println("<br />");
            //Print Relative Humidity
            client.print("Co2: ");
            client.print(scd30.CO2, 2);
            client.print(" ppm");
            client.println("<br />");

            client.println("</body></html>");
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
