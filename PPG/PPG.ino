/*
  Heart beat plotting!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 20th, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  Shows the user's heart beat on Arduino's serial plotter

  Instructions:
  1) Load code onto Redboard
  2) Attach sensor to your finger with a rubber band (see below)
  3) Open Tools->'Serial Plotter'
  4) Make sure the drop down is set to 115200 baud
  5) Checkout the blips!
  6) Feel the pulse on your neck and watch it mimic the blips

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"
#include <PeakDetection.h>

// OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


MAX30105 particleSensor;
PeakDetection peakDetection; // create PeakDetection object
int prevBeat = 0;


// SPO2 variables
float average_red_AC = 0;
float average_IR_AC = 0;
float spo2 = 0;
int numSamples = 300;

void setup()
{

  peakDetection.begin(5, 0.8, 0.6); // sets the lag, threshold and influence
  
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  //Arduino plotter auto-scales annoyingly. To get around this, pre-populate
  //the plotter with 500 of an average reading from the sensor

  //Take an average of IR readings at power up
  const byte avgAmount = 64;
  long baseValue = 0;
  for (byte x = 0 ; x < avgAmount ; x++)
  {
    baseValue += particleSensor.getIR(); //Read the IR value
  }
  baseValue /= avgAmount;

  //Pre-populate the plotter so that the Y scale is close to IR values
//  for (int x = 0 ; x < 500 ; x++)
//    Serial.println(baseValue);

//  Serial.println("ir:,peak:");//,ave:");

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);                    
  display.setTextColor(WHITE);             
  display.setCursor(30,5);                
  display.println("Please Place "); 
  display.setCursor(30,15);
  display.println("your finger ");
  display.display();
}

void loop()
{
  long irValue = particleSensor.getIR();
  if (irValue < 50000){
    Serial.println(" No finger?");
    display.clearDisplay();
    display.setTextSize(1);                    
    display.setTextColor(WHITE);             
    display.setCursor(30,5);                
    display.println("Please Place "); 
    display.setCursor(30,15);
    display.println("your finger ");
    display.display();
  } else {
    display.clearDisplay();
    display.setTextSize(1);                    
    display.setTextColor(WHITE);             
    display.setCursor(30,5);                
    display.println("Reading..."); 
    display.display();  

    //read for 30s 
    //don't count for first 10s
    //count peaks for 20s
    int beatCount = 0;
    for(int i = 0; i < 300; i++){
      long irValue = particleSensor.getIR();
      peakDetection.add(irValue); // adds a new data point
      average_IR_AC += irValue;
      
      int peak = peakDetection.getPeak(); // returns 0, 1 or -1
      double filtered = peakDetection.getFilt(); // moving average
      
      if( (prevBeat == 0 or prevBeat == -1) and (peak == 1) and (filtered > 10000) ){
        beatCount++;
      }
      prevBeat = peak;

      long redValue = particleSensor.getRed();
      average_red_AC += redValue;
  //    Serial.print(irValue); //Send raw data to plotter
  //    Serial.print(",");
  //    Serial.print((peak*500)+filtered+500); // print peak status
  //    Serial.print(",");
  //    Serial.println(filtered); // print moving average
    }
  
    int hr = beatCount * 2;
    average_red_AC = average_red_AC / numSamples;
    average_IR_AC = average_IR_AC / numSamples;
    spo2 = average_red_AC/average_IR_AC;
  //  Serial.print("HR: ");
  //  Serial.println(hr);
  
    writeOLED(spo2, hr);
    
    delay(10000);
  }
  
}

// HELPER CODE FOR OLED
void writeOLED(float spo2, int hr) {
  display.clearDisplay(); //clear anthing on the display

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE); //sets text color as white
  display.setCursor(2, 0); //shows where to start writing
  
  display.print(F("SpO2:")); //displays the characters presentes
  display.println(spo2);  //tells OLED what to write
  display.print(F("HR:")); //displays the characters presentes
  display.println(hr);  //tells OLED what to write
  display.display();

  delay(1000);
}
