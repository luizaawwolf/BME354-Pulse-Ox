// Sensor libraries
#include <heartRate.h>
#include <spo2_algorithm.h>
#include <MAX30105.h>
#include <Wire.h>
#include <SPI.h>
#include <PeakDetection.h>



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


// Peak detection code for bpm
PeakDetection peakDetection; // create PeakDetection object
int prevBeat = 0;
int bpm = 0;

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

  // Peak detection code for bpm
  peakDetection.begin(5, 0.8, 0.6); // sets the lag, threshold and influence
  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  Serial.println("ir:,peak:,ave:");
}

void loop() {

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


    //read for 30s and count peaks
    int beatCount = 0;
    
    for(int i = 0; i < numSamples; i++){
      long irValue = particleSensor.getIR();
      peakDetection.add(irValue); // adds a new data point
      int peak = peakDetection.getPeak(); // returns 0, 1 or -1
      double filtered = peakDetection.getFilt(); // moving average
      if( (prevBeat == 0 or prevBeat == -1) and (peak == 1) and (filtered > 10000) ){
        beatCount++;
      }
      prevBeat = peak;
//      Serial.print(irValue); //Send raw data to plotter
//      Serial.print(",");
//      Serial.print((peak*500)+filtered+500); // print peak status
//      Serial.print(",");
//      Serial.println(filtered); // print moving average


      long redValue = particleSensor.getRed();    //Reading the IR value
//      Serial.println(redValue);
      average_red_AC += redValue;
    }
  
    int hr = beatCount * 2;
//    Serial.print("HR: ");
//    Serial.println(hr);
//    delay(10000);
    
//    Serial.println();
  
    average_red_AC = average_red_AC / numSamples;
    average_IR_AC = average_IR_AC / numSamples;
    spo2 = average_red_AC/average_IR_AC;
//    Serial.println(spo2);
    writeOLED(spo2, hr);
    exit(0);
//    break;
//    delay(2000);
    
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
