#include <Ethernet.h>
#include <SPI.h>
#include "SoftwareSerial.h"
#include "Adafruit_Thermal.h"

int printer_RX_Pin = 2;  // This is the green wire
int printer_TX_Pin = 3;  // This is the yellow wire

Adafruit_Thermal printer(printer_RX_Pin, printer_TX_Pin);

////////////////////////////////////////////////////////////////////////
//CONFIGURE
//////////////f//////////////////////////////////////////////////////////
  //byte ip[] = { 192, 168, 0, 1 };   //Manual setup only
  //byte gateway[] = { 192, 168, 0, 1 }; //Manual setup only
  //byte subnet[] = { 255, 255, 255, 0 }; //Manual setup only

  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

  EthernetServer server = EthernetServer(80); //port 80
  EthernetClient client;
////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(19200);
  pinMode(7, OUTPUT); digitalWrite(7, LOW); // To also work w/IoTP printer
  printer.begin();
  printer.setDefault(); // Restore printer to defaults

  Ethernet.begin(mac);
  //Ethernet.begin(mac, ip); //for manual setup

  server.begin();
  printer.println(Ethernet.localIP());
  printer.feed(1);
}

void loop(){
  // listen for incoming clients, and process qequest.
  checkForClient();
}

void checkForClient(){
  client = server.available();

  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean reading = false;
    String myStr = "";
 
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (reading && c == ' ') reading = false;
        if (c == '?') reading = true; //found the ?, begin reading the info

        if (reading){
          if (c!='?') {
            myStr += c;
          }
        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    
    String parsedStr = parseParameters(myStr);
    printString(parsedStr);
    sendResponse(parsedStr);
    delay(100); // give the web browser time to receive the data
    client.stop(); // close the connection:    
  } 
}

String parseParameters(String str) {
  int startIndex = str.indexOf("p");
  int endIndex = str.length();
  String rawStr = str.substring(startIndex + 2, endIndex);
  
  // Copy string
  String decodedStr = rawStr;

  // Decode space characters
  decodedStr.replace("%20", " ");
  return decodedStr;
}

void printString(String str) {
  // Currently only supports one bold substring.
  int startBold = str.indexOf("**") + 2;
  int endBold;
  if (startBold != -1) {
    endBold = str.indexOf("**", startBold);
    String boldStr = str.substring(startBold, endBold);
    str = str.substring(endBold + 2, str.length());
    printer.boldOn();
    printer.println(boldStr);
    printer.boldOff();
  }
  
  printer.println(str);
  printer.feed(1);
}

void sendResponse(String str) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  // the connection will be closed after completion of the response
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  // output the value of each analog input pin
  client.println("<h1>Printing: " + str + "</h1>");
  client.println("</html>");
}
