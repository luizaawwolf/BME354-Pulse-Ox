// Sensor libraries
#include <heartRate.h>
#include <spo2_algorithm.h>
#include <MAX30105.h>
#include <Wire.h>
#include <SPI.h>

// Declare sensor
MAX30105 particleSensor;

// OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int numSamples = 300;
int currentSamples = 0;
int samplingTime = 1; //1ms

// SPO2 variables
float average_red_AC = 0;
float average_IR_AC = 0;
float spo2 = 0;

// Heart rate variables
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int fullReading = 0;
int beatAvg;
int hbcount = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("MAX30105 Basic Readings Example");

  // Initialize sensor
  if (particleSensor.begin() == false)
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive

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

void loop() {

  long irValue = particleSensor.getIR();

  int i = 0;
  average_red_AC = 0;
  average_IR_AC = 0;
  hbcount = 0;
  while( irValue >= 50000 and i < 300){
    //take a reading for 15s
    display.clearDisplay(); //clear anything on the display
    display.setCursor(2, 0); //shows where to start writing
    display.println(F("Reading...")); //displays the characters presentes
    display.display();


    //SPO2 READING INFORMATION
    // read red
    long redValue = particleSensor.getRed();    
    average_red_AC += redValue;

    // read IR photodiode
    irValue = particleSensor.getIR();   
    average_IR_AC += irValue;

    // delay for sampling rate
    delay(1);

    //HEARTRATE READING INFORMATION
    if (checkForBeat(irValue) == true) {
      if( hbcount == 0 ){
        i = 0; //reset the count to start at the first heartbeat
      }
      hbcount += 1;
//      Serial.println("BEAT");
//      display.print(("BEAT! "));
//      display.print(hbcount);
//      display.display();
//      delay(500);
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();
  
      beatsPerMinute = 60 / (delta / 1000.0);
      Serial.println("BEAT");
      Serial.println(beatsPerMinute);
  
//      if (beatsPerMinute < 255 && beatsPerMinute > 20)
//      {
//        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
//        rateSpot %= RATE_SIZE; //Wrap variable
//  
//        //Take average of readings
//        beatAvg = 0;
//        for (byte x = 0 ; x < RATE_SIZE ; x++)
//          beatAvg += rates[x];
//        beatAvg /= RATE_SIZE;
//        Serial.println(beatAvg);
//      }
    }

    // check IR and increment
    irValue = particleSensor.getIR();
//    Serial.println(i);
    i++;
  }
  
  // got all data points for 15s with no interruptions
  if( i == 300 ){
      average_red_AC = average_red_AC / 1500;
      average_IR_AC = average_IR_AC / 1500;
      spo2 = average_red_AC/average_IR_AC;
      beatAvg = hbcount*4; //hbcount for 15s, times 4 for beats per minute
      Serial.println(spo2);
      Serial.println(beatAvg);
      writeOLED(spo2, beatAvg);
      delay(20000);
  }
  Serial.println(" No finger?");
  display.clearDisplay(); //clear anthing on the display

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE); //sets text color as white
  display.setCursor(2, 0); //shows where to start writing
  
  display.print(F("No finger")); //displays the characters presentes
  display.display();
  



//  if (irValue < 50000){
//    Serial.println(" No finger?");
//    display.clearDisplay(); //clear anthing on the display
//
//    display.setTextSize(2); // Draw 2X-scale text
//    display.setTextColor(WHITE); //sets text color as white
//    display.setCursor(2, 0); //shows where to start writing
//    
//    display.print(F("No finger")); //displays the characters presentes
//    display.display();
//  } else {
//    display.clearDisplay(); //clear anthing on the display
//    display.setCursor(2, 0); //shows where to start writing
//    display.print(F("Reading...")); //displays the characters presentes
//    display.display();
//    
//    //read for 15s
//    fullReading = 1;
//    average_red_AC = 0;
//    average_IR_AC = 0;
//    for( int i = 0; i < 1500; i++){ 
//
//      //start over if interrupted
//      if (irValue < 50000){
//        fullReading = 0;
//        Serial.println("BROKEN");
//        break;
//      }
//      
//      
//      // read red photodiode
//      long redValue = particleSensor.getRed();    //Reading the IR value
//      average_red_AC += redValue;
//  
//      // read IR photodiode
//      long irValue = particleSensor.getIR();    //Reading the IR value
//      average_IR_AC += irValue;
////      Serial.print("...");
//
//      if (checkForBeat(irValue) == true)
//      {
//        //We sensed a beat!
//        long delta = millis() - lastBeat;
//        lastBeat = millis();
//    
//        beatsPerMinute = 60 / (delta / 1000.0);
//        Serial.println(beatsPerMinute);
//    
//        if (beatsPerMinute < 255 && beatsPerMinute > 20)
//        {
//          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
//          rateSpot %= RATE_SIZE; //Wrap variable
//    
//          //Take average of readings
//          beatAvg = 0;
//          for (byte x = 0 ; x < RATE_SIZE ; x++)
//            beatAvg += rates[x];
//          beatAvg /= RATE_SIZE;
//        }
//      }
//      
//      delay(10);
//    }
//    
//    Serial.println();
//
//    //if we got a full reading (15s), do the math and display
//    if( fullReading == 1){
//      average_red_AC = average_red_AC / 1500;
//      average_IR_AC = average_IR_AC / 1500;
//      spo2 = average_red_AC/average_IR_AC;
//      Serial.println(spo2);
//      Serial.println(beatAvg);
//      writeOLED(spo2, beatAvg);
//      delay(2000);
//    }
//    
//  }
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
