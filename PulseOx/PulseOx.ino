//Luiza Wolf and Alex Chen
//OLED initialization

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// H-bridge pins
int redPNP_pin = 2;
int redNPN_pin = 3;
int IRPNP_pin = 4;
int IRNPN_pin = 5;
int time_led_on = 5000;

// Sensing pins
int photodiode_pin = 7;

// Sensing variables
float photodiode_reading = 0;
float red_AC[5000];
float IR_AC[5000];
float average_red_AC = 0;
float average_IR_AC = 0;
float spo2 = 0;


void setup() {
  // Begin serial monitor / plotter
  Serial.begin(9600);

  // OLED Code
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.display();
  delay(2000);
  display.clearDisplay();
  
  //test to see if the display is working so we draw a single pixel to see if it turns
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(2000);
  //since we want to continuously change the heartrate the code will always run so up here we just set it 
  //to run while true so forever.
  

  // Set pin modes for h-bridge pins
  pinMode(redPNP_pin, OUTPUT);
  pinMode(redNPN_pin, OUTPUT);
  pinMode(IRPNP_pin, OUTPUT);
  pinMode(IRNPN_pin, OUTPUT);

  // Set sensing pin modes
  pinMode(photodiode_pin, INPUT);
}

void loop() {
  delay(1000);
  
  // turn RED LED on for 5s and read from photodiode
  turnRedOn(time_led_on);
  for( int i = 0; i < time_led_on; i++){

    // read and store to red photodiode array
    photodiode_reading = analogRead(photodiode_pin);
    Serial.println(photodiode_reading);
    red_AC[i] = photodiode_reading;
    average_red_AC += photodiode_reading;
    
    // 1ms sampling rate
    delay(1);
  }
  // turn everything off for 1s
  average_red_AC = average_red_AC / time_led_on;
  reset();  

  // turn IR LED on for 5s and read from photodiode
  turnIROn(time_led_on);
  for( int i = 0; i < time_led_on; i++){

    // read and store IR array
    photodiode_reading = analogRead(photodiode_pin);
    Serial.println(photodiode_reading);
    IR_AC[i] = photodiode_reading;
    average_IR_AC += photodiode_reading;

    // 1ms sampling rate
    delay(1);
  }
  // turn everything off for 1s
  average_IR_AC = average_IR_AC / time_led_on;
  reset();

  spo2 = average_red_AC/average_IR_AC;
  writeSpO2(spo2);
  delay(1000000);
  
}


// HELPER CODE FOR H-BRIDGE

// Turns RED LED on
void turnRedOn(int duration){
  reset();
  digitalWrite(redPNP_pin, LOW);
  digitalWrite(redNPN_pin, HIGH);
}

// Turns IR LED on
void turnIROn(int duration){
  reset();
  digitalWrite(IRPNP_pin, LOW);
  digitalWrite(IRNPN_pin, HIGH);
}


// Turns everything off so no current can flow through h-bridge
void reset(){
  // block any current from flowing through h-bridge
  digitalWrite(redPNP_pin, HIGH);
  digitalWrite(redNPN_pin, LOW);
  digitalWrite(IRPNP_pin, HIGH);
  digitalWrite(IRNPN_pin, LOW);

  // wait 1s
  delay(1000);
}

// HELPER CODE FOR OLED
void writeSpO2(int reading) {
  display.clearDisplay(); //clear anthing on the display

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE); //sets text color as white
  display.setCursor(2, 0); //shows where to start writing
  display.print(F("SpO2:")); //displays the characters presentes
  display.println(reading);  //tells OLED what to write
  delay(100);
}
