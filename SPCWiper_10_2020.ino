//This is the firmware for the October 2020 revision of the SPC Hydrowiper. This switches to using PWM and a simple motor driver instead of a motor control board.
// Written by D. Ratelle

//Define the Arduino Nano Every pin  mapping.
const int U1Pin = A0;
const int U2Pin = A1;
const int U3Pin = A2;
const int U4Pin = A3;
const int U5Pin = A6;
const int U6Pin = A7;

const int fault_pin = 8;
const int brake_pin = 7;
const int dir_pin = 4;
const int pwm_pin = 3;
const int sleep_pin = 2;
const int current_pin = A4;

unsigned long ramp_t = 2000;  //Ramp up time
unsigned long ramp_t2 = 2000; //Ramp down time
unsigned long ramp_start;
unsigned long t_inc = ramp_t/50;
unsigned long t_inc2 = ramp_t2/50;
unsigned long inc_start;

const long interval = 1000;
const long timeout = 100000;
const long angledelay = 1250;

unsigned long timer = 0;
unsigned long lastprint = 0;
unsigned long hallset = 0;
unsigned long anglecorrect = 0;
unsigned long test;

int ramp_min = 5;
int ramp_max = 255;
int ramp_step = 5;
int ramp_pwm = ramp_min;
int state = 0;
int h1 = 0;
int h2 = 0;
int h3 = 0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println(millis());
//Initialize sensor pins
pinMode(U2Pin, INPUT);
pinMode(U4Pin, INPUT);
pinMode(U6Pin, INPUT);
Serial.println("Sensor pins initialized.");

//Initialize pins for Motor Driver
pinMode(pwm_pin, OUTPUT);
pinMode(dir_pin, OUTPUT);
pinMode(current_pin, INPUT);
pinMode(brake_pin, OUTPUT);
Serial.println("Motor Driver control pins initialized.");

//Set pwm to 0 as a precaution, direction to FWD, and braking on.
analogWrite(pwm_pin, 0);
digitalWrite(dir_pin, 0); //Clockwise for the motor is actually reverse.
digitalWrite(brake_pin, 1);
test = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
while (state < 5){
  timer = millis();
  h1 = analogRead(U2Pin);
  h2 = analogRead(U4Pin);
  h3 = analogRead(U6Pin);
  
  if (timer - lastprint >= interval)
  {
    hall_print();
  }
  
  if (timer > timeout)
    {
      motor_rampdown(millis());
      Serial.print("Timeout! Stopping Motor.");
      state = 5;
      delay(1500);
     }
  
  if (state == 0)
  {  
    if (ramp_pwm < 100)
    {
    digitalWrite(dir_pin, 1);
    motor_rampup(millis());
    }
    if (h1<600)
      {
        hallset = millis();
        Serial.print("Hall Effect 1 Triggered, reversing\n");
        state = 1;
      }
  }
  
  else if (state == 1)
  {
    anglecorrect = millis();
    if (anglecorrect - hallset >= angledelay)
    {
      motor_rampdown(millis());
      digitalWrite(dir_pin, 0);
      motor_rampup(millis());
      hallset = anglecorrect;
      state = 2;
    }
  }
  
  else if (state == 2)
  {
     if(h2<600)
     {
      hallset = millis();
      state = 3;
      Serial.print("Hall Effect 2 Triggered, returning to park\n");
     }
  }
   
  else if (state == 3)
  {
    anglecorrect = millis();
    if (anglecorrect - hallset >= angledelay)
    {
      motor_rampdown(millis());
      digitalWrite(dir_pin,1);
      motor_rampup(millis());
      hallset = anglecorrect;
      state = 4;
    }
  }
  
  else {
    if(h3<600)
    {
      motor_rampdown(millis());
      Serial.print("Hall Effect Park Triggered\n");
      state = 5;
    }
  }
}

motor_rampdown(millis());
Serial.print('\n');
Serial.print(state);
timer = millis();
Serial.print('\t');
Serial.print(timer);
}

void hall_print() {
  Serial.print("Hall Effect Sensor 1: ");
  Serial.print(h1);
  Serial.print('\t');
  Serial.print("Hall Effect Sensor 2: ");
  Serial.print(h2);
  Serial.print('\t');
  Serial.print("Hall Effect Sensor 3: ");
  Serial.println(h3);
  lastprint = timer;
}

void motor_rampup (unsigned long ramp_start) {
  Serial.println("Motor rampup.");
  while ((millis()-ramp_start) <= ramp_t) {
    if ((millis()-inc_start) >=  t_inc) {
      if (ramp_pwm < ramp_max) {
        ramp_pwm += ramp_step;
      }
      else {
        ramp_pwm = ramp_max;
      }
      analogWrite(pwm_pin, ramp_pwm);
      inc_start = millis();
    }
  }
}

void motor_rampdown (unsigned long ramp_start) {
  Serial.println("Motor rampdown.");
  while ((millis()-ramp_start) <= ramp_t2) {
    if ((millis()-inc_start) >=  t_inc2) {
      if (ramp_pwm >= ramp_min) {
        ramp_pwm -= ramp_step;
      }
      else {
        ramp_pwm = 0;
      }
      analogWrite(pwm_pin, ramp_pwm);
      inc_start = millis();
    }
  }
  Serial.print("Rampdown complete, ");
  Serial.println(timer);
}
