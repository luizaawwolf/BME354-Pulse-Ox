
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
  Serial.begin(9600);

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
