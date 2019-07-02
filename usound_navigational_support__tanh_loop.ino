#include <math.h>

unsigned int flag_FAR  = 0;
unsigned int flag_NEAR = 0;

// two motors
const int echo = 6;
const int trig = 5;

// single motor
//const int echo = 9;
//const int trig = 6;
const int motor_L = 11;
const int motor_R = 3;

long duration, distance;
int dutyCycle  = 0;

float alpha = 0.5; // exponential smoothing constant
float average_cm = 0; // initialization @ generic distance

// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;

unsigned long previousMillis = 0;
const long interval = 100;
unsigned int counts = 0;

signed int beat_out = 250;

void setup() {
  //TCCR2B=(TCCR2B&0xF8) | 2;
  pinMode(motor_L, OUTPUT);
  digitalWrite(motor_L, LOW);
  
  pinMode(motor_R, OUTPUT);
  digitalWrite(motor_R, LOW);
  
  pinMode(echo,INPUT);
  
  pinMode(trig,OUTPUT);
  digitalWrite(trig, LOW);
  
  Serial.begin(9600);

}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;

  if (currentMillis-previousMillis >= interval) {
    counts += 1;
    // Hold the trigger pin high for at least 10 us
    digitalWrite(trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(trig,LOW);

    // Wait for pulse on echo pin
    while (digitalRead(echo) == 0);

    // Measure how long the echo pin was held high (pulse width)
    // Note: the micros() counter will overflow after ~70 min
    t1 = micros();
    while (digitalRead(echo) == 1);
    t2 = micros();
    pulse_width = t2-t1;

    if ( pulse_width > MAX_DIST ) {
      cm = 400;
    } else {
      cm = pulse_width/58.0;
    } 
    average_cm = alpha*cm+(1-alpha)*average_cm;    //exponential smoothing

    if(average_cm <= 200 && average_cm > 35){
      //dutyCycle=round(255-1.7*average_cm);
      dutyCycle=round(127+127*((exp(-(average_cm-70)/35)-exp((average_cm-70)/35))/(exp(-(average_cm-70)/35)+exp((average_cm-70)/35))));
      //dutyCycle=round(296-1.48*average_cm);
      dutyCycle = constrain(dutyCycle,0,255); 
      analogWrite(motor_L,dutyCycle);
      analogWrite(motor_R,dutyCycle);
      
    } else if (average_cm <= 35 && (counts % 2) == 0) {
      beat_out = beat_out*(-1); 
      analogWrite(motor_L,constrain(beat_out,0,250));
      analogWrite(motor_R,constrain(beat_out,0,250));
      
    } else if (average_cm > 150 && (counts % 5) == 0) {
      beat_out = beat_out*(-1); 
      analogWrite(motor_L,constrain(beat_out,0,250));
      analogWrite(motor_R,constrain(beat_out,0,250));
      
    } else if (average_cm > 250 && (counts % 9) == 0) {
      //beat_out = beat_out*(-1); 
      analogWrite(motor_L,250);
      analogWrite(motor_R,250);
      delay(20);
      analogWrite(motor_L,0);
      analogWrite(motor_R,0);
      delay(20);
      analogWrite(motor_L,250);
      analogWrite(motor_R,250);
      delay(20);
      analogWrite(motor_L,0);
      analogWrite(motor_R,0);
      delay(20);
      analogWrite(motor_L,250);
      analogWrite(motor_R,250);
      delay(20);
      analogWrite(motor_L,0);
      analogWrite(motor_R,0);

    } else {
      dutyCycle = 0; 
      analogWrite(motor_L,dutyCycle);
      analogWrite(motor_R,dutyCycle);
    }

    Serial.print(cm);
    Serial.print(" cm \t");
    Serial.print(average_cm);
    Serial.print(" avg \t");
    Serial.print(dutyCycle);
    Serial.println(" dCycle");
    previousMillis = currentMillis;
  }
}
  
