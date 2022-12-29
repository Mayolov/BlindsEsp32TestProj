#include <Arduino.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <string>

WiFiClient RemoteClient;

// Set web server port number to 80
WiFiServer Server(80);

// Variable to store the HTTP request
String header;

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

/* These variable take integer values and are assigned to checking variables
   The though of this is when in the loop if a variable is changed a checker var
   checks it in an if statement and if they are not the same the checked variable changes 
   to the new variable and changes the time of which the alarm is set at and remakes the alarm.
*/
//*************************************
int alarmOpenHours = 7;
int alarmOpenHoursCheck = alarmOpenHours;

int alarmOpenMins = 0;
int alarmOpenMinsCheck =  alarmOpenMins;

int alarmCloseHours = 20;
int alarmCloseHoursCheck = alarmCloseHours;


int alarmCloseMins = 25;
int alarmCloseMinsCheck = alarmCloseMins;
//***************************************

// These are for the while loops that move the motor for a specific amount of time
unsigned long time_since_last_reset = 0;
int interval_one = 5000;
int interval_two = 3000;

// Should be the closing way, but itll change if its not right when I test it 
void motorClockWise(void)
{
  Serial.println("\nALARM 1 TRIGGERED!\n");
  
  if(isOpen == true){
  time_since_last_reset = millis();

  analogWrite(speedPin, mSpeed);
  while ((millis() - time_since_last_reset) < interval_one)
  {
    // Counter-Clockwise
    digitalWrite(dir1, HIGH);
    digitalWrite(dir2, LOW);
  }
  digitalWrite(dir1, LOW);
  isOpen = false;
  }else{
    Serial.println("---------\nalready open\n---------");
  }
  
  Alarm.delay(200);
}

// Should be the opening way, but itll change if its not right when I test it 
void motorCounterClockWise(void)
{
  // Psuedo code for the open or close on the app
  /* if sense close button && isOpen == True{
      close using function
      isOpen == false;
  }
  */
  Serial.println("\nALARM 2 TRIGGERED!\n");

  if(isOpen == false){
  time_since_last_reset = millis();
  analogWrite(speedPin, mSpeed);

while ((millis() - time_since_last_reset) < interval_one)
  {
    // Clockwise
    digitalWrite(dir1, LOW);
    digitalWrite(dir2, HIGH);
  }
  digitalWrite(dir2, LOW);
  isOpen = true;
  }else{    
    Serial.println("---------\nalready closed\n---------");
  }


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

std::string readRequest(WiFiClient client) {
  // Read the incoming request into a string
  std::string request = "";
  while (client.available()) {
    request += (char)client.read();
  }
  return request;
}

// Parse the incoming request to determine the command
std::string parseCommand(std::string str, char start = '{', char end = '}') {

  int beginning = str.find(start);
  int ending = str.find(end);

  return str.substr(beginning,ending);
}

void parseAndExecute(std::string command){
  
  // Find the first occurrence of c in s
  std::size_t indexOpenBlinds = command.find("openBlinds");
  std::size_t indexCloseBlinds = command.find("closeBlinds");
  std::size_t indexOpenTime = command.find("blindsOpenTime");
  std::size_t indexShutTime = command.find("blindsShutTime");
  

  if(indexOpenBlinds != std::string::npos){
    motorClockWise();
  }
  else if(indexCloseBlinds != std::string::npos){
    motorCounterClockWise();
  }
  else if(indexOpenTime != std::string::npos){
    std::string fHour;
    std::string fMin;
    std::string strTime;

    indexOpenTime += 17;  // Skip past "\"blindsOpenTime\":\""
    std::size_t end = command.find("\"", indexOpenTime);
    if (end != std::string::npos) {
      strTime = command.substr(indexOpenTime, end - indexOpenTime);
    }

    std::size_t pos = strTime.find(":");
    if (pos != std::string::npos) {
      // Extract the hour and minute values
      fHour = strTime.substr(0, pos);
      fMin = strTime.substr(pos + 1);
    }

    alarmOpenHours = std::stoi(fHour);
    alarmOpenMins = std::stoi(fMin);
    Serial.print("The time showing: ");
    Serial.println(strTime.c_str());
    //std::string strTime = command.substr(L,R);
    Serial.print("Hours: ");
    Serial.println(fHour.c_str());
    Serial.print("Mins: ");
    Serial.println(fMin.c_str());
    Serial.println("");
  }
  else if(indexShutTime != std::string::npos){
    std::string fHour;
    std::string fMin;
    std::string strTime;

    indexShutTime += 17;  // Skip past "\"blindsOpenTime\":\""
    std::size_t end = command.find("\"", indexShutTime);
    if (end != std::string::npos) {
      strTime = command.substr(indexShutTime, end - indexShutTime);
    }

    std::size_t pos = strTime.find(":");
    if (pos != std::string::npos) {
      // Extract the hour and minute values
      fHour = strTime.substr(0, pos);
      fMin = strTime.substr(pos + 1);
    }

    alarmOpenHours = std::stoi(fHour);
    alarmOpenMins = std::stoi(fMin);
    Serial.print("The time showing: ");
    Serial.println(strTime.c_str());
    //std::string strTime = command.substr(L,R);
    Serial.print("Hours: ");
    Serial.println(fHour.c_str());
    Serial.print("Mins: ");
    Serial.println(fMin.c_str());
    Serial.println("");
  }
}

void setup()
{
  WiFi.mode(WIFI_STA);

  Serial.begin(9600);

  // local initialization
  WiFiManager wm;
  
  // wipes stored credentials for testing
  //wm.resetSettings();

  bool res;

  res = wm.autoConnect("coffeeeee","1234567890");

  if(!res){
    Serial.println("Failed to connect");
    //ESP.restart();
  }else{
    // connected to wifi
    Serial.print("Connected!!!!!!!!!!!!!!!!");
  }
  // Motor pin modes
  pinMode(speedPin, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);

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


  //wsClient.beginSSL(WS_HOST,WS_PORT,WS_URL,"","wss");
  Server.begin(); // Start the server
}

void loop()
{
  timeClient.update();
  
  WiFiClient client = Server.available();
  if (client) {
    // Read the incoming HTTP request
    std::string request = readRequest(client);

    // Parse the incoming request to determine the command
    std::string command = parseCommand(request);

 

    // // Perform the appropriate action based on the command
    // executeCommand(command, data);

    // // Send a response back to the client
    // sendResponse(client);
    Serial.println("why is this not posting.... ");
    Serial.print("request => readRequest: ");
    Serial.println(request.c_str());
    Serial.println("------------");
    Serial.print("command => parseCommand: ");
    Serial.println(command.c_str());
    // Extract the data from the reques
    Serial.println("------------");
    parseAndExecute(request);
    Serial.println("------------");
    Serial.println("------------");
  }

  Serial.print("Waiting for alarms\n ");

  // If the updated times on the non-Check vars are changed then go and changed their alarm
  if(hour() == alarmOpenHoursCheck && minute() == alarmOpenMinsCheck){
    motorClockWise();
    //set isOpen to false/ true depending on how this operates
  }
    if(hour() == alarmCloseHoursCheck && minute() == alarmCloseMinsCheck){
    motorCounterClockWise();
    //set isOpen to false/ true depending on how this operates
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

  Alarm.delay(2000);
  /*
  if sense open button && isOpen ==False{
    open using function
    isOPen == true
  }
  */
}