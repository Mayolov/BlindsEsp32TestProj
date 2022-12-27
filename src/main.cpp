#include <Arduino.h>
#include <Wifi.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <NTPClient.h>
#include <WiFiUDP.h>

// wifi credentials
const char* ssid = "7LeavesCafe";
const char* password = "berry129";

// Set timezone PST and the server we're using 
#define NTP_OFFSET 3*60*60+60*60
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

/* These variable take integer values and are assigned to checking variables
   The though of this is when in the loop if a variable is changed a checker var
   checks it in an if statement and if they are not the same the checked variable changes 
   to the new variable and changes the time of which the alarm is set at and remakes the alarm.
*/
//*************************************
int alarmOpenHours = 1;
int alarmOpenHoursCheck = alarmOpenHours;

int alarmOpenMins = 1;
int alarmOpenMinsCheck =  alarmOpenMins;

int alarmCloseHours = 1;
int alarmCloseHoursCheck = alarmCloseHours;


int alarmCloseMins = 1;
int alarmCloseMinsCheck = alarmCloseMins;
//***************************************

AlarmId idAlarm1;
AlarmId idAlarm2; 

// These are for the while loops that move the motor for a specific amount of time
unsigned long time_since_last_reset = 0;
int interval_one = 5000;
int interval_two = 3000;

// Should be the closing way, but itll change if its not right when I test it 
void motorClockWise(void)
{

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
String newTime =  timeClient.getFormattedTime();
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
setTime((newtime.substring(0,2)).toInt(),(newtime.substring(3,5)).toInt(),(newtime.substring(6,8)).toInt(),1,1,20);
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
  // If the updated times on the non-Check vars are changed then go and changed their alarm 
  if(alarmOpenHoursCheck!= alarmOpenHours && alarmOpenMinsCheck != alarmOpenMins){

  } 
  if(alarmCloseHoursCheck!= alarmCloseHours && alarmCloseMinsCheck != alarmCloseMins){
    
  }



  // displays the time in minutes
  digitalClockDisplay();

  Alarm.delay(1000);

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
  Serial.print("Waiting for alarms\n ");
  analogWrite(speedPin, mSpeed);

  // do this command if the time alarm time is equal to this and if the blinds are closed
  //if (ClockObj.isAlarm1() && isOpen == false){
  motorClockWise();

  //isOpen = true;
  //}

  // do this command if the time alarm time is equal to this and if the blinds are open
  //if (ClockObj.isAlarm2() && isOpen == true){
  motorCounterClockWise();
  //isOpen = false;
  //}

}
