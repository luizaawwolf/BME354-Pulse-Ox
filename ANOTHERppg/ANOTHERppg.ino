#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include <PeakDetection.h>


MAX30105 particleSensor;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
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
PeakDetection peakDetection; // create PeakDetection object


void setup() {
  Serial.begin(115200);
  particleSensor.begin(Wire, I2C_SPEED_STANDARD);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  peakDetection.begin(5, 0.8, 0.6); // sets the lag, threshold and influence

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F ; //Options: 0=Off to 255=50mA
  byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

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
  oled.clearDisplay();
}

void loop() {

  // Display is only 128 pixels wide, so if we're add the end of the display, clear the display and start back over
  if(x>127)  
  {
    oled.clearDisplay();
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
 
  oled.setTextColor(WHITE);
  long reading = particleSensor.getIR();    // Read pulse ox sensor; since this is a pulse pleth, we're really only after the IR component
  peakDetection.add(reading); // adds a new data point
  int y=40-(map(reading, lastMin, lastMax, 0, 40));   // Normalize the pleth waveform against the rolling IR min/max to keep waveform centered
//  double movingAve = peakDetection.getFilt(); // moving average
//  y = reading - movingAve;
  Serial.println(y);
  oled.drawLine(lastx,lasty,x,y,WHITE);

  // Keep track of min/max IR readings to keep waveform centered
  if (reading > rollingMax){
    rollingMax = reading;
  }

  if (reading < rollingMin){
    rollingMin = reading;
  }
  
  // Keep track of this IR reading so we can draw a line from it on the next reading
  lasty=y;
  lastx=x;

  oled.display();
  x++;
  z++;
}
