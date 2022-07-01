/* Hardware Connections:

IMPORTANT: The APDS-9960 can only accept 3.3V!
 
 Arduino Pin  APDS-9960 Board  Function
 
 3.3V         VCC              Power
 GND          GND              Ground
 A4           SDA              I2C Data
 A5           SCL              I2C Clock
 2            INT              Interrupt

Resources:
Include Wire.h and SparkFun_APDS-9960.h
*/

#include <Wire.h> 
#include <SparkFun_APDS9960.h>
#include <LiquidCrystal.h>
#include <Servo.h>

#define PINSIZE 7 
#define SERVOPIN 9//How many gestures long your password is
// Pins
#define APDS9960_INT    2 // Needs to be an interrupt pin
LiquidCrystal lcd(12,11,3,4,5,6);
Servo servo;
uint8_t proximity_data = 0;
int red_led = A1;
int green_led = A2;
int pageNum = 0; //The LCD "page"
String secret[PINSIZE] = {"RIGHT","LEFT","UP","UP","RIGHT","LEFT","FAR"}; //Password goes here. Must be in CAPS. Can be "UP", "DOWN", "LEFT", or "RIGHT"
String currentGesture = "";
int numRight = 0;
int prevTimer = 0;
int recordTime = 100;
int first = 0;

SparkFun_APDS9960 apds = SparkFun_APDS9960(); //Init APDS-9960 library
int isr_flag = 0;

void setup() {

  // Set interrupt pin as input
  pinMode(APDS9960_INT, INPUT);
lcd.begin(16,2);
servo.attach(SERVOPIN);
servo.write(0);
servo.detach();
  // Initialize Serial port
  Serial.begin(9600);
  Serial.print("Your password is ");
  Serial.print(PINSIZE);
  Serial.println(" gestures long.");
  Serial.println();
  Serial.println(F("----------------"));
  Serial.println(F("APDS-9960 Lock"));
  Serial.println(F("----------------"));
  
  
  // Initialize interrupt service routine
  attachInterrupt(0, interruptRoutine, FALLING);

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  if ( !apds.setProximityGain(PGAIN_1X) ) {
    Serial.println(F("Something went wrong trying to set PGAIN"));
  }
  if ( apds.enableProximitySensor(false) ) {
    Serial.println(F("Proximity sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during sensor init!"));
  }
  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
  
}

void(* resetFunc) (void) = 0;

void loop() {
  if(pageNum==0){
    if ( !apds.readProximity(proximity_data) ) {
    Serial.println("Error reading proximity value");
  } else {
    Serial.println(proximity_data);
    if((proximity_data >= 40) && (pageNum==0)){
    pageNum=1;
    delay(2000);
  }
  }
  } 
  switch(pageNum){
    case 0:
    LCDClear();
    lcd.print("Locked- get");
    lcd.setCursor(0,1);
    lcd.print("3 inches away.");
    blinkRed();
    LCDClear();
  break;
  
  case 1:
  numRight = 0;
  lcd.print("Enter code...");
  lcd.setCursor(0,0);
  for(int x=0; x<PINSIZE;x++){
    Serial.println(x);
    while(apds.isGestureAvailable()==false){
      
    }
    if( isr_flag == 1 ) {
    detachInterrupt(0);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(0, interruptRoutine, FALLING);
    if(currentGesture != "NONE"){
    if(currentGesture==secret[x]){
    LCDClear();
    lcd.print("Next");
    numRight += 1;
    analogWrite(green_led,255);
    delay(1000);
    allOff();
  }
  else if(currentGesture != secret[x]){
    LCDClear();
    lcd.print("Wrong, return to");
    lcd.setCursor(0,1);
    lcd.print("main page.");
    analogWrite(red_led,255);
    delay(1000);
    allOff();
    pageNum = 0;
    break;
  }
  if(numRight == PINSIZE){
    pageNum = 2;
    break;
  }
    }
    else if(currentGesture == "NONE"){
      LCDClear();
      lcd.print("Not recognized:");
      lcd.setCursor(0,1);
      lcd.print("Try again.");
      x -= 1;
    }
  }
  }
  
  break;
  case 2:
  LCDClear();
  int timer = millis()/1000;
  lcd.print("Enter now.");
  lcd.setCursor(0,1);
  lcd.print("Time taken: ");
  lcd.setCursor(11,1);
  if (first == 0)
  {
    lcd.print(timer - prevTimer);
  }
  else
  {
    lcd.print(timer - prevTimer - 15);
  }
  prevTimer = timer;
  first++;
  lcd.setCursor(13,1);
  lcd.print("sec");
  pageNum = 0;
  servo.attach(SERVOPIN);
  servo.write(90);
  enter();
  delay(1000);
  servo.write(0);
  break;
  }
  
  
  
}

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
    if (timeCheck() > 30)
    {
      LCDClear();
      lcd.print("You took too");
      lcd.setCursor(0,1);
      lcd.print("much time");
      analogWrite(red_led,255);
      delay(1000);
      allOff();
      pageNum = 0;
      resetFunc();
    }
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        LCDClear();
        lcd.print("Up");
        currentGesture = "UP";
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        LCDClear();
        lcd.print("Down");
        currentGesture = "DOWN";
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        LCDClear();
        lcd.print("Left");
        currentGesture = "LEFT";
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        LCDClear();
        lcd.print("Right");
        currentGesture = "RIGHT";
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        LCDClear();
        lcd.print("Near");
        currentGesture = "NEAR";
        break;
      case DIR_FAR:
        Serial.println("FAR");
        LCDClear();
        lcd.print("Far");
        currentGesture = "FAR";
        break;
      default:
        Serial.println("NONE");
        LCDClear();
        lcd.print("None");
        currentGesture = "NONE";
    }
  }
}

void allOff(){
  analogWrite(red_led,0);
  analogWrite(green_led,0);
}

void blinkRed(){
  analogWrite(red_led,255);
  delay(250);
  allOff();
  delay(250);
}


void LCDClear(){
  lcd.clear();
  lcd.setCursor(0,0);
}

void enter(){
  //total delay of 16 seconds
  for(int x=0;x<30;x++){
  analogWrite(green_led,255);
  delay(100);
  allOff();
  delay(100);
}
for(int x=9;x>0;x--){
  LCDClear();
  lcd.print("Closing in: ");
  lcd.setCursor(0,1);
  lcd.print(x);
  lcd.setCursor(1,1);
  lcd.print(" seconds...");
  delay(1000);
}
LCDClear();
lcd.print("Closing");
}

int timeCheck()
{
  int timer = millis()/1000;
  if (first == 0)
  {
    return timer - prevTimer;
  }
  else
  {
    return timer - prevTimer - 15;
  }
}
