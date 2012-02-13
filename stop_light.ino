#include <EEPROM.h>

// traffic light controller
// outputs for traffic lights of lane 1
#define red_1	     13
#define yellow_1     7
#define green_1	     5
#define switchPin    2

// lamp timing
#define GreenTime   20
#define YellowTime  3
#define RedTime     20

// lamp states
#define INACTIVE_STATE      0
#define RED_STATE           1
#define GREEN_STATE         2
#define YELLOW_STATE        3
#define ALL_ON_STATE        4
#define RED_GREEN_STATE     5
#define RED_YELLOW_STATE    6
#define GREEN_YELLOW_STATE  7

// controller modes
#define STOPLIGHT_MODE      0
#define BLINKRED_MODE       1
#define BLINKYELLOW_MODE    2
#define RANDOM_MODE         3

#define OFF  LOW
#define ON   HIGH

// intialize controller mode
int modes = 0;
int powerDownMode = 0;
// initialize lamp state
int state = 1;
// intialize timing parameters
int nexttime = 1;
int blinkTime = 1; // blink time for blinking modes
int RandomLightTiming = 1;
int delayCounter = 1;

int blinkYellow = ON;
int blinkRed = ON;
int RandomState = 1;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup()
{
    //Setup serial communication
    Serial.begin(115200);

    //Initialize pin modes
    pinMode(red_1,OUTPUT);
    pinMode(yellow_1,OUTPUT);
    pinMode(green_1,OUTPUT);
    pinMode(switchPin, INPUT);

    powerDownMode = EEPROM.read(1);
    Serial.print("Mode");
    Serial.println(powerDownMode);
    if (powerDownMode == 255) {
        modes = 0;
    } else {
        modes = powerDownMode;
    }

    //Call initialization mode
    InitializeMode();

    //Seed the random number generator
    randomSeed(analogRead(0));

}

void loop()
{
    //Default delay is 1 second - nexttime and delayCounter use this delay to time the lights 
    delay(1000);

    //Report current state and mode every second
    Serial.println(state);
    Serial.print("mode ");
    Serial.println(modes);
  
    // read the state of the switch into a local variable:
    int reading = digitalRead(switchPin);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH),  and you've waited
    // long enough since the last press to ignore any noise:  

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }
 
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer
        // than the debounce delay, so take it as the actual current state:
        buttonState = reading;
    }
 
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButtonState = reading;
  
    // Check to see if the button has made a transition
    if (buttonState == 1 and lastButtonState == 0) {
        SwitchMode();
    }

    //Determines what mode the controller is in
    if (delayCounter >= nexttime) {
      delayCounter = 0;
      switch(modes) {
         case STOPLIGHT_MODE: // regular stop light mode
             StopLightMode();
             break;
         case BLINKRED_MODE: // blink red mode
    	     BlinkRedMode();
    	     break;
         case BLINKYELLOW_MODE: // blink yellow mode
    	     BlinkYellowMode();
    	     break;
         case RANDOM_MODE: // randomly turn on/off lights
    	     RandomMode();
    	     break;
       }
  } else {
    // Count up the delayCounter if the light hasn't been on for nexttime seconds
    delayCounter = delayCounter + 1;
  }
  
}

// regular stop light mode
void StopLightMode()
{
    switch (state) {
        case INACTIVE_STATE : // inactive state, all off
            SetLights(OFF,OFF,OFF);
            nexttime = 3;
            state = RED_STATE;
            break;
        case RED_STATE : // red
            SetLights(ON,OFF,OFF);

            // goon after yellow to green time
            nexttime = RedTime;
            state = GREEN_STATE;
            break;
        case GREEN_STATE : // green
            // switch walker lights to green
            SetLights(OFF,OFF,ON);

            nexttime = GreenTime;
            state = YELLOW_STATE;
            break;
        case YELLOW_STATE : // yellow
            SetLights(OFF,ON,OFF);
            
            nexttime = YellowTime;
            state = RED_STATE;
            break;
        case ALL_ON_STATE: // initialize all on
            SetLights(ON,ON,ON);
            nexttime = 2;
            state = INACTIVE_STATE;
            break;
  }
}

//Blink red mode
void BlinkRedMode()
{
    if(blinkRed == ON) {
        SetLights(ON,OFF,OFF);
  	    blinkRed = OFF;
	} else {
        SetLights(OFF,OFF,OFF);
        blinkRed = ON;
	}
	nexttime = blinkTime;
}

//Blink yellow mode
void BlinkYellowMode()
{
    if(blinkYellow == ON) {
        SetLights(OFF,ON,OFF);
        blinkYellow = OFF;
    } else {
        SetLights(OFF,OFF,OFF);
        blinkYellow = ON;
    }
    nexttime = blinkTime;
}

//Randomly turns on and off lights in pseudo random sequence
void RandomMode ()
{
    //Random light timing
    RandomLightTiming = random(1,4);
    //Randomize the next state
    RandomState = random(1,8);

    switch (state) {
        case INACTIVE_STATE : // inactive state, all off
            SetLights(OFF,OFF,OFF);
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case RED_STATE : // red
            SetLights(ON,OFF,OFF);

            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case GREEN_STATE : // green
            // switch walker lights to green
            SetLights(OFF,OFF,ON);
            
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case YELLOW_STATE : // yellow
            SetLights(OFF,ON,OFF);

            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case ALL_ON_STATE: // initialize all on
            SetLights(ON,ON,ON);
            
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case RED_GREEN_STATE: // initialize all on
            SetLights(ON,OFF,ON);
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case RED_YELLOW_STATE: // initialize all on
            SetLights(ON,ON,OFF);
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
        case GREEN_YELLOW_STATE: // initialize all on
            SetLights(OFF,ON,ON);
            nexttime = RandomLightTiming;
            state = RandomState;
            break;
          }
}

//Initialization sequence
void InitializeMode()
{
    SetLights(ON,ON,ON);
    delay(1000);
    SetLights(OFF,OFF,OFF);
    delay(1000);
    SetLights(ON,OFF,OFF);
    delay(1000);
    SetLights(OFF,ON,OFF);
    delay(1000);
    SetLights(OFF,OFF,ON);
    delay(1000);
    SetLights(OFF,OFF,OFF);
    delay(1000);
}

//Function to output the light commands
void SetLights( int sR, int sY, int sG)
{
    digitalWrite(red_1,sR);
    digitalWrite(yellow_1,sY);
    digitalWrite(green_1,sG);  
}

void SwitchMode()
{
    Serial.println("Change");
    if (modes >= 3) {
        modes = 0;
    } else {
        modes = modes + 1;
    }
    
    EEPROM.write(1,modes);
    //Reinitialize state to 1 after each mode switch
    state = 1;

    //Confirmation that the mode has been switched
    SetLights(OFF,OFF,OFF);
    delay(1000);
    SetLights(ON,ON,ON);
    delay(1000);
    SetLights(OFF,OFF,OFF);
    delay(1000);
    SetLights(ON,ON,ON);
    delay(1000);
    SetLights(OFF,OFF,OFF);
    delay(1000);
}
