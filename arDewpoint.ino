
/*********************************************************************
ArDewpoint Ardunio based ventilation system

Author Michael Niemi. https://github.com/mvniemi
BSD License

Uses library SparkFunBME280.h and example code from SPARKFUN under MIT License
Uses libraries and example code from ADAFRUIT under BSD License
*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdint.h>
#include "SparkFunBME280.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//Relay control pins
#define RELAY1 8
#define RELAY2 9

//Set the frequency of updating the relay here, in minutes. The larger the value, the less likely you will have rapid on/off cycling
#define RELAYINTERVAL 1

//Set the ventilation threshold of the DEWPOINT in degrees celcius. I recccomend it being greater than 1, as that seems to be the margin of error for these sensors
#define VENTTHRESHOLD 1.5

//Define sensor objects
BME280 sensorIn;
BME280 sensorOut;

//Globals. We use longs because we are counting seconds, 16bit would overrun
long ventingCount=0;
long notVentingCount=0;
long loopCount=0;
long relayCount=0;
long minutes=0;
long subcount=29;

bool vent=false;

void setup()   {                
  Serial.begin(9600);

  //Init the Display
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(2000);
  display.clearDisplay();
  // init done


  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  //Initialize Sensors
  sensorIn.settings.commInterface = I2C_MODE;
  sensorOut.settings.I2CAddress = 0x77;

  sensorIn.settings.commInterface = I2C_MODE;
  sensorOut.settings.I2CAddress = 0x76;

  sensorIn.settings.runMode = 3; //Normal mode
  sensorOut.settings.runMode = 3; //Normal mode

  sensorIn.settings.tStandby = 0;
  sensorOut.settings.tStandby = 0;  

  sensorIn.settings.filter = 0;
  sensorOut.settings.filter = 0;  

  sensorIn.settings.tempOverSample = 1;
  sensorOut.settings.tempOverSample = 1;
  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    sensorIn.settings.pressOverSample = 1;
    sensorOut.settings.pressOverSample = 1; 
  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  sensorIn.settings.humidOverSample = 1;
  sensorOut.settings.humidOverSample = 1; 

  delay(1000);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  Serial.println(sensorIn.begin(), HEX);
  Serial.println(sensorOut.begin(), HEX);

  
  //Initialize Relays, set to OFF (which is HIGH, arduino set to LOW at boot)
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);
  digitalWrite(RELAY1,HIGH);
  digitalWrite(RELAY2,HIGH);
  

}


void loop() {
  loopCount++;
  subcount++;
  //Get our readings from the sensors:
  float tempIn = sensorIn.readTempC();
  float tempOut= sensorOut.readTempC();
  float rhIn =  sensorIn.readFloatHumidity();
  float rhOut = sensorOut.readFloatHumidity();

  float dpIn = dewpoint(tempIn,rhIn);
  float dpOut = dewpoint(tempOut,rhOut);

  //Decide whether to vent
  if(subcount==(60*RELAYINTERVAL)){
    subcount=1;
    if ((dpOut+VENTTHRESHOLD)<dpIn){
        if(!vent){relayCount++;}
        vent=true;
        digitalWrite(RELAY1,LOW);
        digitalWrite(RELAY2,LOW);
        ventingCount++;
    }
    else{
        if(vent){relayCount++;}
        digitalWrite(RELAY1,HIGH);
        digitalWrite(RELAY2,HIGH);
        notVentingCount++;
        vent=false;
        }
  }
  
  //Send status to display

  display.clearDisplay();
  display.setCursor(0,0);

  display.print("In  T:");
  display.print(tempIn, 0);  
  display.print(" DP:");
  display.print(dpIn, 0);
  display.print(" H:");
  display.println(rhIn, 0);

  display.print("Out T:");
  display.print(tempOut, 0);    
  display.print(" DP:");
  display.print((dpOut), 0);
  display.print(" H:");
  display.println(rhOut, 0);
  
  if(vent){display.println("Venting");}
  else{display.println("Not Venting");}
  display.print("VentTime:");
  display.println(float(ventingCount)/60,3);
  display.print("OffTime:");
  display.println(float(notVentingCount)/60,3);


  display.print("TimeAlive: ");
  display.println(float(loopCount)/3600,3);
  display.print("RelayCycles:");
  display.println(relayCount);
  display.print("Updating in: ");
  display.println((60*RELAYINTERVAL-subcount));
  display.display();

  delay(1000);
}

//quick dp approximation, http://en.wikipedia.org/wiki/Dew_point
float dewpoint(float temperature, float humidity){
{
  float a = 17.271;
  float b = 237.7;
  float temp = (a * temperature) / (b + temperature) + log((double) humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}
}








   


