// include the thermocouple library code
#include "max6675.h"
// include the lcd library code:
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// include the Ethernet library code;
#include <SPI.h>
#include <Ethernet.h>

// define thermocouple pins for 4 inputs
const int ktcSO = 51, ktcCLK = 53, ktc1CS = 49 , ktc2CS = 47, ktc3CS = 45, ktc4CS = 43;
// create special degree C character
byte degc[8] = {B10000,B00111,B01000,B01000,B01000,B01000,B00111,B00000};
const int delaytime=350; //this is the delay between moves in ms

// initialize 4 instances of the ktc
MAX6675 ktc1(ktcCLK, ktc1CS, ktcSO);
MAX6675 ktc2(ktcCLK, ktc2CS, ktcSO);
MAX6675 ktc3(ktcCLK, ktc3CS, ktcSO);
MAX6675 ktc4(ktcCLK, ktc4CS, ktcSO);
int x = 0;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192,168,1,100);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
////////////////// set up the LCD's number of columns and rows:///////////////////
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0,degc);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//    while (true) {
//      delay(1); // do nothing, no point running without Ethernet hardware
//    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
}
void loop() {
   delay(500);
   float Temp[] = {ktc1.readCelsius(),ktc2.readCelsius(),ktc3.readCelsius(),ktc4.readCelsius()};
   int Col[] = {0,0,0,0};
   int Row[] = {0,1,2,3};
   x = 0;
   while(x <= 3){
    lcd.setCursor(Col[x],Row[x]);
    lcd.print(x + 1);
    lcd.print(":");
    lcd.print(String(Temp[x],1));
    lcd.write(0);
    lcd.print("    ");
    x = x + 1;
   }
}



      
