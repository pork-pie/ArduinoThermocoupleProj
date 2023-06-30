
#include "max6675.h" // include the thermocouple library code
#include <LiquidCrystal_I2C.h> // include the lcd library code:
#include <SPI.h> // include the Ethernet library code;
#include <Ethernet.h> // include the Ethernet library code;
////////////////////////////////////////
// define thermocouple pins for 4 inputs
const int ktcCLK=38,ktcSO=36,ktc1CS=30,ktc2CS=34,ktc3CS=28,ktc4CS=32;  //The addresses for all the thermocouple channels
byte degc[8] = {B10000,B00111,B01000,B01000,B01000,B01000,B00111,B00000}; // create special degree C character
const int delaytime=100; //this is the delay between readings in ms
const int averages=10; //number of temperature averages to remove jitter
const int pumpchannel = 13;
const int dutycycle = 15; //The pump duty cycle in %. Min 15% Max 100%
const int Col[] = {0,0,10,10};
const int Row[] = {0,1,0,1};
const float height[] = {0,0.21,0.55,0.97,1.18,1.7};
int x;
int q;
int pumpactive = 0;
////////////////////////////////////////

// initialize 4 instances of the ktc
MAX6675 ktc1(ktcCLK, ktc1CS, ktcSO);
MAX6675 ktc2(ktcCLK, ktc2CS, ktcSO);
MAX6675 ktc3(ktcCLK, ktc3CS, ktcSO);
MAX6675 ktc4(ktcCLK, ktc4CS, ktcSO);
////////////////////////////////////////

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);
////////////////////////////////////////

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
// Pin 10 has to be connected along with the ICSP connecters
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//IPAddress ip(192,168,1,123);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
////////////////////////////////////////

void setup() {
  pinMode(pumpchannel,OUTPUT); //This sets the pump control channel to output. Has to be a PWM channel
  analogWrite(pumpchannel,0); //Set pump to off initially
  ////////Start the LCD screen
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0,degc);
  ////////Start the serial port for debugging
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Thermocouple web server, looking for ethernet controller");
  lcd.setCursor(0,0);
  lcd.print("Thermo web server");
  lcd.setCursor(0,1);
  lcd.print("Looking for web");
  ///////////////////////////
  // start the Ethernet connection and check its status:
  while (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    lcd.setCursor(0,2);
    lcd.print("DHCP Failed!");
    delay(1);
  }
  // Check for Ethernet hardware present
  while (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      lcd.setCursor(0,3);
      lcd.print("Shield not found.");
      delay(1); // do nothing, no point running without Ethernet hardware
  }
  
  // start the server and print status and address
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  lcd.clear();
  lcd.setCursor(0,3);
  lcd.print("IP:");
  lcd.print(Ethernet.localIP());
}

void loop() {
      //Read the temperature and print it to the lcd screen
   //Do averages
   float Temp[] = {0,0,0,0};
   x = 1;
   while(x <= averages){
   Temp[0] = Temp[0] + ktc1.readCelsius();
   Temp[1] = Temp[1] + ktc2.readCelsius();
   Temp[2] = Temp[2] + ktc3.readCelsius();
   Temp[3] = Temp[3] + ktc4.readCelsius();
   x = x + 1;
   delay(delaytime);
   }
   Temp[0] = Temp[0] / averages;
   Temp[1] = Temp[1] / averages;
   Temp[2] = Temp[2] / averages;
   Temp[3] = Temp[3] / averages;
   //Display the averaged data
   x = 1;
   while(x <= 4){
    lcd.setCursor(Col[x-1],Row[x-1]);
    lcd.print(x);
    lcd.print(":");
    lcd.print(String(Temp[x-1],1));
    lcd.write(0);
    x = x + 1;
   }
   //Turn on pump to destratify if top and bottom of tank are too differnet in temp
   //Temp at top has to be more than 60 degrees to start then runs while bottom is more than 15 degrees less.
   //Need to add in a dwell so the pump does not turn on and off too much, say 3 degrees
   int activatetemp = 60; //this is the temperature above which the pump will start de-stratifying.
   int deltatemp[] = {15,12}; //this is the max and min difference in temperaures to turn the pump on and off.
   if (Temp[0] > activatetemp and (Temp[0] - Temp[3]) > deltatemp[pumpactive]){
      lcd.setCursor(0,2);
      lcd.print("Pump on at ");
      lcd.print(dutycycle);
      lcd.print("%");
      analogWrite(pumpchannel, (256*dutycycle/100)); //This turns on the pump with the duty cycle that has been defined
      pumpactive = 1;
   } else {
      lcd.setCursor(0,2);
      lcd.print("Pump off      ");
      analogWrite(pumpchannel, 0);
      pumpactive = 0;
   }
   // listen for incoming clients
   EthernetClient client = server.available();
   if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int tempChannel = 0; tempChannel <= 3; tempChannel++) {
            client.print("Temp from Channel ");
            client.print(tempChannel+1);
            client.print(" is ");
            client.print(String(Temp[tempChannel],1));
            client.print("C");
            client.println("<br />");
          }
          if (pumpactive) {
            client.print("Pump on");
            client.println("<br />");
          } else {
            client.print("Pump Off");
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}



      
