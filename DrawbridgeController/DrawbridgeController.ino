/*
  Drawbrdge Controller - Rory Sweeney

    LCD SI pin to digital pin 2
    LCD CLK pin to digital pin 3
    LCD RS pin to digital pin 4
    LCD CSB pin to digital pin 5
    LCD RESET pin is not used
    LCD Backlight pin is not used
    LCD Red Light pin to pin 9
    LCD Green Light pin to pin 10
    LCD Blue Light pin to pin 11

  Library homepage : https://github.com/xxxxxxxxxxx

*/

// include the library code:
#include <DogLcd.h>

// initialize the library with the numbers of the interface pins
DogLcd lcd(2, 3, 4, 5);

const int redPin = 9;
const int grnPin = 10;
const int bluPin = 11;

const int raiseBtn = 6;
const int dropBtn = 7;
const int topSensor = 8;
const int bottomSensor = 12;

const int motorRaisePin = A0;
const int motorDropPin = A1;
const int motorOnPin = A2;

const int lightSensor = A3;
const int lights = A4;
const int lightSensorThreshold = 750;  // Above this value it is considered dark...

int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial

// Colours for LCD Screen...
enum {
  RED,
  AMBER,
  GREEN
} eColour;

// Values for State Machine...
enum { 
  STATE_UNKNOWN, 
  BRIDGE_DOWN, 
  BRIDGE_RAISING, 
  BRIDGE_UP, 
  BRIDGE_DROPPING 
} eState;

int currentState = STATE_UNKNOWN;

void setup() {

  // set up the LCD type and the contrast setting for the display
  lcd.begin(DOG_LCD_M163);

  // Print a message to the LCD.
  lcd.print("Drawbridge Ctrl");
  lcd.setCursor(0, 1); 
  lcd.print("State:");

  pinMode(redPin, OUTPUT);   // sets the pins as output
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 

  pinMode(raiseBtn, INPUT_PULLUP);   // sets the pins as output
  pinMode(dropBtn, INPUT_PULLUP);   
  pinMode(topSensor, INPUT_PULLUP); 
  pinMode(bottomSensor, INPUT_PULLUP);

  pinMode(motorRaisePin, OUTPUT);   // sets the pins as output
  pinMode(motorDropPin, OUTPUT);   
  pinMode(motorOnPin, OUTPUT); 

  pinMode(lightSensor, INPUT);
  pinMode(lights, OUTPUT);
      
  LCD_UpdateScreenColour(AMBER);
  
  if (DEBUG) {           // If we want to see values for debugging...
    Serial.begin(9600);  // ...set up the serial ouput 
  }
}

void loop() {

  StateMachine();
  delay(50);
  
}

void StateMachine()
{

  LCD_UpdateState(currentState);

  CheckLightSensor();
  
  // Based on current state, check for valid transitions...
  switch (currentState) {
    
    case STATE_UNKNOWN:
      if (Calibrate() == true) {
        currentState = BRIDGE_DOWN;  
        LCD_UpdateScreenColour(GREEN);
      }
      break;
      
    case BRIDGE_DOWN:
      
      if (CheckButton_Raise() == true) {
        // Turn the lights RED and then open the bridge...
        StopTheTraffic();
        RaiseTheBridge();
        currentState = BRIDGE_RAISING;
      }
      break; 
      
    case BRIDGE_RAISING:
      
      if (CheckSwitch_Top() == true) {
        StopTheBridge();
        currentState = BRIDGE_UP;
      }
      break;

    case BRIDGE_UP:
      
      if (CheckButton_Drop() == true) {
        DropTheBridge();
        currentState = BRIDGE_DROPPING;
      }
      break;
      
    case BRIDGE_DROPPING:
     
      if (CheckSwitch_Bottom()== true) {
        StopTheBridge();
        currentState = BRIDGE_DOWN;
        StartTheTraffic();
      }
      break;
  }
}

void CheckLightSensor()
{
  int val = analogRead(lightSensor);

  String lsVal = "LightSensor value: ";
  lsVal += val;
  
  Serial.println(lsVal);

  // Turn on lights if its dark...
  if (val > lightSensorThreshold) {
    digitalWrite(lights, HIGH);
  }
  else {
    digitalWrite(lights, LOW);
  }
}

void RaiseTheBridge()
{

  digitalWrite(motorDropPin, LOW);
  digitalWrite(motorRaisePin, HIGH);
  digitalWrite(motorOnPin, HIGH);
  
}

void DropTheBridge()
{

  digitalWrite(motorDropPin, HIGH);
  digitalWrite(motorRaisePin, LOW);
  digitalWrite(motorOnPin, HIGH);

}

void StopTheBridge()
{

  digitalWrite(motorOnPin, LOW);
  
}

void StopTheTraffic()
{
    LCD_UpdateScreenColour(AMBER);
    delay(5000);
    LCD_UpdateScreenColour(RED);
    delay(5000);
}

void StartTheTraffic()
{
    LCD_UpdateScreenColour(AMBER);
    delay(5000);
    LCD_UpdateScreenColour(GREEN);
}

// Calibrate and get to known state (BRIDGE CLOSED)...
bool Calibrate()
{
  LCD_UpdateAction("CALIBRATE");  

  delay(5000);
  
  return true;
}

bool CheckButton_Raise()
{
  LCD_UpdateAction("BTN RAISE ");  

  int val = digitalRead(raiseBtn);

  if (val == HIGH) {
    return false;
  }
  return true;
}

bool CheckButton_Drop()
{
  LCD_UpdateAction("BTN DROP  "); 
     
  int val = digitalRead(dropBtn);

  if (val == HIGH) {
    return false;
  }
  return true;
}

bool CheckSwitch_Top()
{
  LCD_UpdateAction("TOP HIT   ");    
 
  int val = digitalRead(topSensor);

  if (val == HIGH) {
    return false;
  }
  return true;
}

bool CheckSwitch_Bottom()
{
  LCD_UpdateAction("BTM HIT   ");  
  
  int val = digitalRead(bottomSensor);

  if (val == HIGH) {
    return false;
  }
  return true;
}

void LCD_UpdateState(int pState)
{
  lcd.setCursor(6, 1);  
  
  switch (pState) {

    case STATE_UNKNOWN:
      lcd.print("UNKNOWN ");
      break;
      
    case BRIDGE_UP:
      lcd.print("UP      ");
      break;
      
    case BRIDGE_RAISING:
      lcd.print("RAISING ");
      break;
      
    case BRIDGE_DROPPING:
      lcd.print("DROPPING");
      break;
          
    case BRIDGE_DOWN:
      lcd.print("DOWN    ");
      break;
  }
}

void LCD_UpdateAction(char *pAction)
{
//  Serial.println("LCD_UpdateAction called: ");
//  Serial.println(pAction);
  
  lcd.setCursor(0, 2);  
  lcd.print("Wait: ");
  lcd.print(pAction);
}

void LCD_UpdateScreenColour(int pColour)
{

  switch (pColour) {

    case RED:
      analogWrite(redPin, 255);   // Write current values to LED pins
      analogWrite(grnPin, 0);      
      analogWrite(bluPin, 0); 
      break;
      
    case AMBER:
      analogWrite(redPin, 255);   // Write current values to LED pins
      analogWrite(grnPin, 191);      
      analogWrite(bluPin, 0); 
      break;
      
    case GREEN:
      analogWrite(redPin, 0);   // Write current values to LED pins
      analogWrite(grnPin, 255);      
      analogWrite(bluPin, 0); 
      break;
  }

}
