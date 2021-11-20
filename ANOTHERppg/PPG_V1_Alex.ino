#include <Wire.h>
#include "MAX30105.h"
#include <PeakDetection.h>

// OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


MAX30105 particleSensor;
PeakDetection peakDetection; // create PeakDetection object
int prevBeat = 0;


// SPO2 variables
float average_red_AC = 0;
float average_IR_AC = 0;
float spo2 = 0;


Adafruit_SSD1306 display(128, 64, &Wire, -1);
byte x;
byte y;
byte z;
byte lastx;
byte lasty;
long baseValue = 0;
long lastMin=2200000;
long lastMax=0;
long rollingMin = 2200000;
long rollingMax=0;

int tcount = 0;
int beatCount = 0;
bool cleard = false;

void setup() {
  Serial.begin(115200);
  particleSensor.begin(Wire, I2C_SPEED_STANDARD);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  //Take an average of IR readings at power up; this allows us to center the plot on start up
  const byte avgAmount = 30;
  long reading;
  for (byte x = 0 ; x < avgAmount ; x++){
    reading = particleSensor.getIR();

    // Find max IR reading in sample
    if (reading > lastMax){
      lastMax = reading;
    }

    // Find min IR reading in sample
    if (reading < lastMin){
      lastMin = reading;
    }
  }
  
  x = 0;
  y = 0;
  lastx = 0;
  lasty = 0;
  delay(2000);
  display.clearDisplay();

  // Peak detection
  peakDetection.begin(1, 0.3, 0.6); // sets the lag, threshold and influence
  
  display.clearDisplay();
  display.setTextSize(1);                    
  display.setTextColor(WHITE);             
  display.setCursor(30,5);                
  display.println("Please Place "); 
  display.setCursor(30,15);
  display.println("your finger ");
  display.display();
  pinMode(LED_BUILTIN, OUTPUT);


  
}

void loop() {

  long irValue = particleSensor.getIR();
  if (irValue < 3000){ //50000
    Serial.println(" No finger?");
    display.clearDisplay();
    display.setTextSize(1);                    
    display.setTextColor(WHITE);             
    display.setCursor(30,5);                
    display.println("Please Place "); 
    display.setCursor(30,25);
    display.println("your finger ");
    display.display();
    tcount = 0;
    beatCount = 0;
    cleard = false;
  } else {
    if(!cleard){
      display.clearDisplay();
      cleard = true;
    }
    
    //if has been reading for 20s, then report value
    int numSamples = 200;
    if( tcount >= numSamples ){
      int hr = int(beatCount / 40.0 * 60);
      average_red_AC = average_red_AC / numSamples;
      average_IR_AC = average_IR_AC / numSamples;
      spo2 = average_red_AC/ average_IR_AC; //dividing by 2 for calibration purposes
      writeOLED(spo2, hr);
      exit(0);
    } else {

      //SHOW PPG WHILE READING
      tcount++;
      
      // Display is only 128 pixels wide, so if we're add the end of the display, clear the display and start back over
      if(x>127)  
      {
        display.clearDisplay();
        x=0;
        lastx=x;
      }
    
      // Even though we're keeping track of min/max on a rolling basis, periodically reset the min/max so we don't end up with a loss of waveform amplitude
      if (z > 20) {
        z = 0;
        lastMax = rollingMax;
        lastMin = rollingMin;
        rollingMin = 2200000;
        rollingMax = 0;
      }
     
      display.setTextColor(WHITE);

      //Get data
      long irValue = particleSensor.getIR();    // Read pulse ox sensor; since this is a pulse pleth, we're really only after the IR component
      peakDetection.add(irValue); // adds a new data point
      average_IR_AC += irValue;
      long redValue = particleSensor.getRed();
      average_red_AC += redValue;

      //Count beats
      int peak = peakDetection.getPeak(); // returns 0, 1 or -1
      double filtered = peakDetection.getFilt(); // moving average
      if( (prevBeat == 0 or prevBeat == -1) and (peak == 1) and (filtered > 10000) ){
        beatCount++;
        digitalWrite(LED_BUILTIN, HIGH);
      }
      prevBeat = peak;
      digitalWrite(LED_BUILTIN, LOW);

      
      //Plot PPG
      int y=40-(map(irValue, lastMin, lastMax, 10, 25));   // Normalize the pleth waveform against the rolling IR min/max to keep waveform centered
      Serial.println(y);
      display.drawLine(lastx,lasty,x,y,WHITE);
    
      // Keep track of min/max IR readings to keep waveform centered
      if (irValue > rollingMax){
        rollingMax = irValue;
      }
    
      if (irValue < rollingMin){
        rollingMin = irValue;
      }
      
      // Keep track of this IR reading so we can draw a line from it on the next reading
      lasty=y;
      lastx=x;

      display.setTextSize(1);  
      display.setTextColor(WHITE, BLACK);             
      display.setCursor(5,50);                
      display.print("Beat Count: "); 
      display.println(beatCount);
    
      display.display();
      digitalWrite(LED_BUILTIN, LOW);
      x += 3; //scale x-axis
      z++;
      }
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
