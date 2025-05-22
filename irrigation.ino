#include "DHT.h"                      
#include <DFRobot_DS1307.h>           // RTC Library
#define DHTTYPE DHT11               
#include <SD.h>                       // SD card 

DFRobot_DS1307 DS1307;                // something to do with the RTC idk
File myFile;                          // Creates file for sd card

const int DHTPIN = 5;                 
const int dir2PinA = 4;               // Pump
const int CSPin = 10;                 // SD Card
const int moistPin = A0;              
const int moistDigPin = 7;            // digital pin instead of 5v (corrosion problem)

DHT dht(DHTPIN, DHTTYPE);             

const int moistureThreshold = 400;   
int lastLoggedMinute = -1;            // tracking when logged

void setup() {
  Serial.begin(9600);                 
  Serial.println(" ");                

  pinMode(moistDigPin, OUTPUT);       
  pinMode(moistPin, INPUT);           
  DS1307.start();                    
  DS1307.setSqwPinMode(DS1307.eSquareWave_1Hz);  

  pinMode(dir2PinA, OUTPUT);          
  digitalWrite(dir2PinA, HIGH);      

  dht.begin();                       

  while(!(DS1307.begin()))  // error checking RTC
  {
    Serial.println("Communication with device failed, please check connection"); 
    delay(3000);                    
  }
}

void loop()
{
  float humid = dht.readHumidity();  // Read humidity from DHT sensor
  float temp = dht.readTemperature(); // Read temperature from DHT sensor
  int moisture = soil();              // Read soil moisture level

  uint16_t timeData[7] = {0};        
  DS1307.getTime(timeData);          
  int currentMinute = timeData[1];   // getting current minute
  if (currentMinute != lastLoggedMinute) { // checking if minute has changed since last log
    lastLoggedMinute = currentMinute;     
    rtc(humid, temp, moisture);
  }

  if (moisture < moistureThreshold)    
  {
    digitalWrite(dir2PinA, LOW);       
  } else {
    digitalWrite(dir2PinA, HIGH);      
  }
  delay(500);
}

int soil()
{
  float moistness;                   // moisture sensor reading

  digitalWrite(moistDigPin, HIGH); 
  moistness = analogRead(moistPin); 
  digitalWrite(moistDigPin, LOW); 
  return(moistness);               
}

void rtc(float humid, float temp, int moisture)
{
  uint16_t getTimeBuff[7] = {0};
  DS1307.getTime(getTimeBuff); 

  char outputarr[128];             
  sprintf(outputarr, "Time: %d/%d/%d-%d %d:%d:%d\r, ",
            getTimeBuff[6],        // Year
            getTimeBuff[5],        // Month
            getTimeBuff[4],        // Day
            getTimeBuff[3],        // Hour
            getTimeBuff[2],        // Minute
            getTimeBuff[1],        // Second
            getTimeBuff[0]         // Day of week
            );
  delay(1000);                   

  writeToSDCard(outputarr, humid, temp, moisture); 
}

void writeToSDCard(const char* outputarr, float humid, float temp, int moisture)
{
  Serial.print("Initializing SD card...");  
  pinMode(CSPin, OUTPUT);                  
  if (!SD.begin(CSPin)) {                  // error checking 
    Serial.println("initialization failed!"); 
    return;                                
  }

  myFile = SD.open("liama.txt", FILE_WRITE); 

  if (myFile)                                
  {
      Serial.print("Writing to SD card, "); 
      myFile.print(outputarr);             
      myFile.print(", Humidity = ");         
      myFile.print(humid,1);                
      myFile.print("%");
      myFile.print(", Temperature = ");
      myFile.print(temp,1);
      myFile.print("C");
      myFile.print(", Moisture = ");
      myFile.print(moisture,1);               
      myFile.print(", ");

      myFile.close();                         
      Serial.println("done.");                
  }
  else //error
  {
    Serial.println("error opening liamangel.txt");  
  }
}
