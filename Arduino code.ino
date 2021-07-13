//--------------------------------------------------------------------// 
// Wifi and Thingspeak setup:
#include <WiFi.h> // wifi package
#include "ThingSpeak.h"  // ThingSpeak packge
unsigned long myChannelNumber = 1374514; // our channel number
const char * myWriteAPIKey = "B0VP3R0CXNWEOXE1"; //Channel key
const char* ssid = "HUJI-guest";  // WIFI SSID/Name
const char* password = "" ;// wifi pasword
const char* server = "api.things";
WiFiClient client;
//-------------------------------------------------------------------------//
// HX711.h - Scale Setup
#include "HX711.h" // Scale packge import
// configure pins in setup - easier to change in future
const int LOADCELL_DOUT_PIN = 18; 
const int LOADCELL_SCK_PIN = 19;
 // Semi-wet soil pre-checked - when scale measurement is made, lower water valve will open
 // Every day at 00:00, the weight will increase by 5% (we assume this is the rate of plant growth). 
float NetWeight = 1900;
float zero; // for scaling the weight 
HX711 scale; // re-naming the package name to scale, eaier to hendel 
// --------------------------------------------------------------------//
// Soil Prob Setup
#include "i2cArduino.h" // Soilprob package
#include <Adafruit_Sensor.h> // I2C package 
#include <Wire.h> // another I2C package 
SVCS3 vcs; // re-naming the package name to vcs, eaier to hendel 
// --------------------------------------------------------------------//
// Relay Setup
// configure pins in setup - easier to change in future
const int relay_1 = 25;
const int relay_2 = 26;
const int relay_3 = 27;
// --------------------------------------------------------------------//
// time setup
const char* ntpServer = "pool.ntp.org"; // Choosing a server to communicate with to get the time 
const long  gmtOffset_sec = 0; // starting with time 0 
const int   daylightOffset_sec = 3600; // how many second in hour 
///
void setup() {
  // code runs once! setting up wifi and thingspeak 
  //Wifi
  Serial.begin(9600); // to get Serial print 
  WiFi.disconnect(); // first disconect from any previus wifi 
  delay(10); // delay 
  // Wi-Fi connection process + Thing Speak onnection process 
  WiFi.begin(ssid, password); 
  Serial.println(); 
  Serial.println();
  Serial.print("Connecting to ");  
  Serial.println(ssid);
  ThingSpeak.begin(client); 
  WiFi.begin(ssid, password); 
  // as slong wifi couldnt connect keep trying
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print Succsess message
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
  Serial.println(ssid);
  Serial.println();
    /// ------------------------------------/////////
  // Scale setup
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // start Scale with the pins modified
  delay(1000);
  // Print Success message if it works
  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.println("HX711 Ready to read");
  } else {
    Serial.println("HX711 not found.");
  }
  /// ------------------------------------/////////
  //soil prob 
   vcs.init(0x63);
  /// ------------------------------------/////////
   //Realy Setup - Starting the relay and describing which switch opens the valve
  pinMode(relay_1, OUTPUT); // scale
  pinMode(relay_2, OUTPUT); // prob
  pinMode(relay_3, OUTPUT); // time
  /// ------------------------------------/////////

  // Init and get the time
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);// Reading the current time 
  //printLocalTime();
}

// function for the Scale - reading the scale + calibrate (done with known weight + excel)
void Scale_func()
{
    long reading_factor = 0; // define a paramter to get weight
    // reading the actual weight from scale 
    if (scale.is_ready()) { // if scale is ready, if not skip this reading
    long reading = scale.read();
    Serial.print("HX711 Scale reading: ");
    reading_factor += reading*0.0103+123.450; // calibrate and adding to the return paramer
    Serial.println(reading_factor);
    //Serial.println(reading*0.0103+123.450);
    ThingSpeak.setField(5,reading_factor); // hold in field untill push to could will happen.
    
    // If weight is smaller than wight defined the valve will be opened to 30 sec
    if (reading_factor<NetWeight) //NetWeight
    {
     Serial.println("Open Valve SCALE");
     digitalWrite(relay_1, HIGH); // open valve
     delay(30000);
     digitalWrite(relay_1, LOW); // close valve
     Serial.println("CLOSE Valve SCALE");
     delay(1000);
    }
    //Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
    
  }
  delay(10);
 
}

// function for the Soil Prob - reading E25,EC,Temp,VWC
void SoilProb_func()
{
  vcs.newReading(); // start sensor reading
  delay(100); //let sensor read data

//getting values one by one
  float e25 = vcs.getE25();
  float ec = vcs.getEC();
  float temp = vcs.getTemp();
  float vwc = vcs.getVWC();
  float dat[4]={0,0,0,0}; // empty array to hold values 
  vcs.getData(dat);
  //hold in field untill push to could will happen. 
  ThingSpeak.setField(1,dat[0]);  //E25
  ThingSpeak.setField(2,dat[1]); // EC
  ThingSpeak.setField(3,dat[2]); // Temp
  ThingSpeak.setField(4,dat[3]); // VWC
  // print the values 
  Serial.println("-----");
    Serial.print("e25");
    Serial.print("=");
    Serial.println(dat[0]);
    Serial.print("ec");
    Serial.print("=");
    Serial.println(dat[1]);
    Serial.print("temp");
    Serial.print("=");
    Serial.println(dat[2]);
    Serial.print("vwc");
    Serial.print("=");
    Serial.println(dat[3]);
    // Semi-wet soil pre-checked - when soil prob measurement is made, lower water valve will open
    if (dat[3]<35.0) // 35
    {
     Serial.println("Open Valve PROB");
     digitalWrite(relay_2, HIGH);  // open valve
     delay(30000);
     digitalWrite(relay_2, LOW); // close valve
     Serial.print("Close Valve PROB");
     delay(1000);
    }
}
// function for the Time irrigation
void Time_Func()
{
  // reading the current time (gmt+2 for israel - in winter +3)
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int current_hour = timeinfo.tm_hour;
  int current_min = timeinfo.tm_min;
  Serial.print("HOUR: ");
  Serial.println(current_hour);
  Serial.print("Minute");
  Serial.println(current_min);
    //if (current_hour==12 && current_min==45)
    if (current_hour==14) // we want to irrigat at 12:00, but the +2GMT for israel time
  {
     Serial.println("TIME VALVE OPEN");
     digitalWrite(relay_3, HIGH); // open Vale
     delay(3000);
     digitalWrite(relay_3, LOW);  //close valve
     delay(1000);
     Serial.println("TIME VALVE CLOSE"); 
     NetWeight+=NetWeight*1.05; // incrase weight in 5% each day for plant growth
    
     }
  
}

void loop()
{
  SoilProb_func(); // SoilProb func
  Scale_func(); // Scale func
  Time_Func(); // Time func
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); //upload values to thingSpeak
  digitalWrite(relay_1, LOW); // close valve for safty 
  digitalWrite(relay_2, LOW); //close valve for safty 
  digitalWrite(relay_3, LOW);//close valve for safty 
  Serial.println("15 min Delay");
  // There is a 15 minute delay between loops, this should allow us to observe plant physiology properly
  // Shorter intervals would not improve the data resolution enough
  delay(900000); 

}
