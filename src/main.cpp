#include <Arduino.h>
#include <Wifi.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <NTPClient.h>
#include <WiFiUDP.h>

// wifi credentials
const char* ssid = "7LeavesCafe";
const char* password = "berry129";

// soft webserver thing
const char* ssidServer = "HASH-SLINGING-SLASHER";
const char* passwordServer = NULL;

// Set timezone PST and the server we're using 
#define NTP_OFFSET 16*60*60
#define NTP_ADDRESS "us.pool.ntp.org"

// intizializing this stuff
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,NTP_ADDRESS, NTP_OFFSET);

// The Blinds are open if true
bool isOpen = true;

// Motor controls
const int speedPin = 14;
const int dir1 = 27;
const int dir2 = 26;
int mSpeed = 255;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output25State = "off";
String output33State = "off";

// Wifi control stuff
const int output25 = 25;
const int output33 = 33;

/* These variable take integer values and are assigned to checking variables
   The though of this is when in the loop if a variable is changed a checker var
   checks it in an if statement and if they are not the same the checked variable changes 
   to the new variable and changes the time of which the alarm is set at and remakes the alarm.
*/
//*************************************
int alarmOpenHours = 2;
int alarmOpenHoursCheck = alarmOpenHours;

int alarmOpenMins = 44;
int alarmOpenMinsCheck =  alarmOpenMins;

int alarmCloseHours = 2;
int alarmCloseHoursCheck = alarmCloseHours;


int alarmCloseMins = 45;
int alarmCloseMinsCheck = alarmCloseMins;
//***************************************

// These are for the while loops that move the motor for a specific amount of time
unsigned long time_since_last_reset = 0;
int interval_one = 5000;
int interval_two = 3000;

// Should be the closing way, but itll change if its not right when I test it 
void motorClockWise(void)
{
  timeClient.update();

  time_since_last_reset = millis();
  Serial.println("\nALARM 1 TRIGGERED!\n");

  analogWrite(speedPin, mSpeed);
  while ((millis() - time_since_last_reset) < interval_one)
  {
    // Counter-Clockwise
    digitalWrite(dir1, HIGH);
    digitalWrite(dir2, LOW);
  }
  digitalWrite(dir1, LOW);
  Alarm.delay(200);
}

// Should be the opening way, but itll change if its not right when I test it 
void motorCounterClockWise(void)
{
  Serial.println("\nALARM 2 TRIGGERED!\n");
  time_since_last_reset = millis();
  analogWrite(speedPin, mSpeed);

  while ((millis() - time_since_last_reset) < interval_one)
  {
    // Clockwise
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, HIGH);
  }
  digitalWrite(dir2, LOW);

  Alarm.delay(200);
}

// Prints the digits of the time to print them out 
void printDigits(int digits){
  Serial.print(":");
  if(digits<10){
    Serial.print('0');
  }
  Serial.print(digits);
}

// Displays the time in Serial
void digitalClockDisplay(){
  Serial.print(hour());
  printDigits(minute());
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, password); // Connect to WiFi

  // Motor pin modes
  pinMode(speedPin, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);

  // Checks if connected to wifi
  while (WiFi.status() != WL_CONNECTED)
  { 
    Alarm.delay(500);
    Serial.print("."); 
  }

// Print local IP address and start web server 
Serial.println(""); 
Serial.println("WiFi connected."); 
Serial.println("IP address: "); 
Serial.println(WiFi.localIP());

// Serial prints time when setup is happening
timeClient.update();
String newtime = timeClient.getFormattedTime(); 
Serial.print("the time is : "); 
Serial.println(newtime); 
Serial.print("Hour : "); 
Serial.println((newtime.substring(0,2)).toInt()); 
Serial.print("Minute : "); 
Serial.println((newtime.substring(3,5)).toInt()); 
Serial.print("Seconds : "); 
Serial.println((newtime.substring(6,8)).toInt()); 
Serial.println(timeClient.getFormattedTime());
setTime((newtime.substring(0,2)).toInt(),(newtime.substring(3,5)).toInt(),(newtime.substring(6,8)).toInt(),1,1,20);  // Initialize the output variables as outputs


//************************************************************************************************
//*****************************************Separation*********************************************
//************************************************************************************************
// Connect to Wi-Fi network with SSID and password
Serial.print("Setting AP (Access Point)…");
// Remove the password parameter, if you want the AP (Access Point) to be open
WiFi.softAP(ssidServer, passwordServer); //HERECOM BACKAOMEWFNUBOGB#&*B#@*&OG#B*&GB#&GB@&*G#B#&G*BG@&*BG#

IPAddress IP = WiFi.softAPIP();
Serial.print("AP IP address: ");
Serial.println(IP);
  
server.begin();
//************************************************************************************************
//*****************************************Separation*********************************************
//************************************************************************************************

// alarm one goes clockwise
// could be made into a function instead
  /*While alarm1Set != 'ok' or 'skip'{

      get hour1:
      get minute1:

  }*/
  
// alarm two goes counterclockwise
  /*While alarm2Set != 'ok' or 'skip'{

      get hour2:
      get minute2:

  }*/
}

void loop()
{

  timeClient.update();

//************************************************************************************************
//*****************************************Separation*********************************************
//************************************************************************************************
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
  }
  // Clear the header variable
  header = "";
  // Close the connection
  client.stop();
  Serial.println("Client disconnected.");
  Serial.println("");
//************************************************************************************************
//*****************************************Separation*********************************************
//************************************************************************************************
  Serial.print("Waiting for alarms\n ");
  // If the updated times on the non-Check vars are changed then go and changed their alarm
  if(hour() == alarmOpenHoursCheck && minute() == alarmOpenMinsCheck && isOpen == true){
    motorClockWise();
    //set isOpen to false/ true depending on how this operates
    isOpen = false;
  }
    if(hour() == alarmCloseHoursCheck && minute() == alarmCloseMinsCheck && isOpen == false){
    motorCounterClockWise();
    //set isOpen to false/ true depending on how this operates
    isOpen = true;
  }

  if(alarmOpenHoursCheck!= alarmOpenHours && alarmOpenMinsCheck != alarmOpenMins){
    alarmOpenMinsCheck = alarmOpenMins;
    alarmOpenHoursCheck = alarmOpenHours;
  } 
  if(alarmCloseHoursCheck!= alarmCloseHours && alarmCloseMinsCheck != alarmCloseMins){
    alarmCloseMinsCheck = alarmCloseMins;
    alarmCloseHoursCheck = alarmCloseHours;
  }



  // displays the time in minutes
  digitalClockDisplay();

  Alarm.delay(20000);

  // Psuedo code for the open or close on the app
  /* if sense close button && isOpen ==True{
      close using function
      isOpen == false;
  }
  */

  /*
  if sense open button && isOpen ==False{
    open using function
    isOPen == true
  }
  */

  // do this command if the time alarm time is equal to this and if the blinds are closed
  //if (ClockObj.isAlarm1() && isOpen == false){
  //motorClockWise();

  //isOpen = true;
  //}

  // do this command if the time alarm time is equal to this and if the blinds are open
  //if (ClockObj.isAlarm2() && isOpen == true){
  //motorCounterClockWise();
  //isOpen = false;
  //}

}
