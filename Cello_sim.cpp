#include <Servo.h>
#include <stdio.h>
#include <math.h> 

#define TempoCal 512
#define TempoPotMax 1023
#define PwmMax 255

#define rest 0

#define Octive 4

//Music Notes based on Octive--
#define C 16.3516*pow(2,Octive)
#define D 18.35405*pow(2,Octive)
#define E 20.60172*pow(2,Octive)
#define F 21.82676*pow(2,Octive)
#define G 24.49971*pow(2,Octive)
#define A 27.5*pow(2,Octive)
#define B 30.86771*pow(2,Octive)
#define high_C 32.70320*pow(2,Octive)


#define speakerPin 6
#define baseServoPin 7
#define armServoPin 8


#define NUM_STATES 5

// the pin that the pushbutton is attached to
const int  Start = 2;    
const int  Stop = 3;
const int  TempoSync = 4; 
const int  TimeSync = 5; 


// the pin that the LED is attached to
const int idlePin = 13;   
const int tempoPin = 12; 
const int testPin = 11; 
const int timePin = 10;
const int playPin = 9; 

unsigned long timePress = 0;
unsigned long timePressLimit = 0;
int clicks = 0;


//Row Row Row Your Boat
int songLength = 54;  
int notes[] = {C, rest, C, rest, C, rest, D, rest, E, rest, E, rest, D, rest, E, rest, F, rest, G, rest, high_C, rest, high_C, rest, high_C, rest, G, rest, G, rest, G, rest, E, rest, E, rest, E, rest, C, rest, C, rest, C, rest, G, rest, F, rest, E, rest, D, rest, C, rest};
int beats[] = {2,1,2,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,5,1};


int song_tempo = 250;
int baseIncrement = 1;
int armIncrement = 0;
int basePos = 0; 
int armPos = 90; 


//setup the servo output
Servo baseServo;
Servo armServo;

 
/*******************************************************************
  Function Prototypes
*******************************************************************/
void IDLE_STATE(void);    // State IDLE
void TEMPO_STATE(void);   // State TEMPO
void TEST_STATE(void);    // State TEST
void TIME_STATE(void);    // State TIME
void PLAY_STATE(void);    // State PLAY

/*******************************************************************
  State Machine Skeleton
*******************************************************************/

 // enum of each state
 typedef enum
 {
     STATE_ONE,
     STATE_TWO,
     STATE_THREE,
     STATE_FOUR,
     STATE_FIVE
 }StateType;

 // define state machine table structure
 typedef struct
 {
     StateType State;       // Define the command
     void(*func)(void);     // Defines the function to run
 }StateMachineType;
 
// Table of valid states of the sm and function to be executed for 
StateMachineType StateMachine[] =
 {
     {STATE_ONE, IDLE_STATE},
     {STATE_TWO, TEMPO_STATE},
     {STATE_THREE, TEST_STATE},
     {STATE_FOUR, TIME_STATE},
     {STATE_FIVE, PLAY_STATE}
 };

// Store current state of state machine
StateType SM_STATE = STATE_ONE;


/*******************************************************************
  Initialization
*******************************************************************/
void setup()
{

  pinMode(Start, INPUT); 
  pinMode(Stop, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Stop), IDLE_STATE, CHANGE);
  pinMode(TempoSync, INPUT);
  pinMode(TimeSync, INPUT);
  
  Serial.begin(9600);
  //set up outputs
  pinMode(idlePin, OUTPUT); 
  pinMode(tempoPin, OUTPUT); 
  pinMode(testPin, OUTPUT);
  pinMode(timePin, OUTPUT);
  pinMode(playPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  baseServo.attach(baseServoPin);
  armServo.attach(armServoPin);
  
  //set the servo to zero initial condition.
  baseServo.write(0);
  armServo.write(80);
  
}


/*******************************************************************
  Main Loop
*******************************************************************/
void loop()
{
  // Start the state machine
  RUN_STATEMACHINE();

  
}


/*******************************************************************
  IDLE STATE:
  • Reset memory 
  • Hit start to to begin tempo sync 
  • Doulbe tap start to go into the TEST state

  COMPLETE
*******************************************************************/
void IDLE_STATE(void)
{
  
    digitalWrite(idlePin, HIGH);
    if (digitalRead(tempoPin) == HIGH) {
     
      digitalWrite(tempoPin, LOW);
      SM_STATE = STATE_ONE;
      
    }
    else if (digitalRead(testPin) == HIGH){
      digitalWrite(testPin, LOW);
      noTone(speakerPin);
      baseServo.write(basePos);
      armServo.write(armPos);
      
      SM_STATE = STATE_ONE;
      
    }
    else if (digitalRead(timePin) == HIGH) {
      digitalWrite(timePin, LOW);
      SM_STATE = STATE_ONE;
    }
    else if (digitalRead(playPin) == HIGH) {
      digitalWrite(playPin, LOW);
      noTone(speakerPin);
      baseServo.write(basePos);
      armServo.write(armPos);
      
      SM_STATE = STATE_ONE;
    }

    
    if ( digitalRead(Start) == HIGH) {
      delay(200);
      if (clicks ==0) {
        timePress = millis();
        timePressLimit = timePress + 1000;
        clicks = 1; 
       }

      else if ( clicks == 1 && millis() < timePressLimit) {
        // Button pressed twice 
        
        //set variables back to 0
        timePress = 0;
        timePressLimit = 0;
        clicks = 0;   

        digitalWrite(idlePin, LOW);
        SM_STATE = STATE_THREE;
      }    
    }
      if (clicks == 1 && timePressLimit != 0 && millis() > timePressLimit){
        // Pressed once 
        timePress = 0;
        timePressLimit = 0;
        clicks = 0;
        digitalWrite(idlePin, LOW);
        SM_STATE = STATE_TWO;
       }
}
     


/*******************************************************************
  TEMPO STATE:
  • Write function to read in audio and determine octive level tempo 
  • Go to IDLE when stop is hit 

 Write code to determine tempo sync
 
*******************************************************************/
void TEMPO_STATE(void)
{
    
    digitalWrite(tempoPin, HIGH);
    
    if (digitalRead(TempoSync) == HIGH){     
      digitalWrite(tempoPin, LOW);
      SM_STATE = STATE_FOUR;
    }

  
   
}

/*******************************************************************
  TEST STATE:
  • PLay default audio sound 
  • Make Robot move 
  • Go to IDLE when stop is hit

  write code for robot movement and audio out 
*******************************************************************/
void TEST_STATE(void)
{
    digitalWrite(testPin, HIGH);  
    int duration;                  
    int tempo;
    int tempo_pot = 736; // default
  
    int speed; 
    
    //play the song
    int i_note_index = 0;
    
    while (digitalRead(testPin)) 
    {
      tempo = song_tempo*float(tempo_pot)/TempoCal; //read the tempo POT
      
      //set the Servo speed
      speed = (1 - float(tempo_pot)/TempoPotMax)*PwmMax;
      
      // Base servo movement
      if (basePos == 180) 
      {
        baseIncrement = 0; 
      }
      
      if ( baseIncrement == 1) 
      {
        basePos ++;  
      }
      else 
      {
        basePos --;
      }
      
      baseServo.write(basePos);
      delay(speed / 10);
      
      // arm servo movement
      if (armPos == 0) 
      {
        armIncrement = 1; 
      }
      
      if ( armIncrement == 0) 
      {
        armPos --;  
      }
      else  
      {
        armPos ++;
      }
      armServo.write(armPos);
      delay(speed / 10);
         
      // Play song 
      duration = beats[i_note_index] * tempo;
      tone(speakerPin, notes[i_note_index], duration);
      delay(duration);
      
      //increment the note counter
      ++i_note_index;
      if(i_note_index >= songLength) 
      {
        i_note_index = 0;
      }
      
    }
}


/*******************************************************************
  TEMPO STATE:
   • Write function to read in audio and sync the time 
   • Go to IDLE when stop is hit 

  Write code to determine time sync
*******************************************************************/
void TIME_STATE(void)
{
  
    digitalWrite(timePin, HIGH);
    if (digitalRead(TimeSync) == HIGH){
    digitalWrite(timePin, LOW);
    SM_STATE = STATE_FIVE;
    }
    
       
    

}

/*******************************************************************
  PLAY STATE:
  • Audio out and robot dance for certain amount of time

  Write code to play audio and move robot 
*******************************************************************/
void PLAY_STATE(void)
{
  
    digitalWrite(playPin, HIGH);
    int duration;                  
    int tempo;
    int tempo_pot = 736; // default
  
    int speed; 
    
    //play the song
    int i_note_index = 0;
    
    while (digitalRead(playPin)) 
    {
      tempo = song_tempo*float(tempo_pot)/TempoCal; //read the tempo POT
      
      //set the Servo speed
      speed = (1 - float(tempo_pot)/TempoPotMax)*PwmMax;
      
      // Base servo movement
      if (basePos == 180) 
      {
        baseIncrement = 0; 
      }
      
      if ( baseIncrement == 1) 
      {
        basePos ++;  
      }
      else 
      {
        basePos --;
      }
      
      baseServo.write(basePos);
      delay(speed / 10);
      
      // arm servo movement
      if (armPos == 0) 
      {
        armIncrement = 1; 
      }
      
      if ( armIncrement == 0) 
      {
        armPos --;  
      }
      else  
      {
        armPos ++;
      }
      armServo.write(armPos);
      delay(speed / 10);
         
      // Play song 
      duration = beats[i_note_index] * tempo;
      tone(speakerPin, notes[i_note_index], duration);
      delay(duration);
      
      //increment the note counter
      ++i_note_index;
      if(i_note_index >= songLength) 
      {
        i_note_index = 0;
      }
      
    }
}

/*******************************************************************
  Run State Machine
*******************************************************************/
void RUN_STATEMACHINE(void)
{
    
    // Make Sure States is valid
    if (SM_STATE < NUM_STATES)
    {
        // Call function for state
        (*StateMachine[SM_STATE].func)();
    }
    else
    {
        // Code should never reach here
        while(1)
        {
            // Some exception handling;
        }
    }
}


    


