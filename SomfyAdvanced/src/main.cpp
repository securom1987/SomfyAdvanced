#include <Arduino.h>

/**=================================================================================================================
//
// Title:   SomfyDuino
// Purpose:   A Firmware for Arduino to control a Somfy IO Remote (Situo 5 IO)
//
// Created by: Bjoern Foldenauer
// Adapted by: Holger Döring 11/2021
//
//=================================================================================================================*/

#define REF_VOLTAGE    5.0
#define PIN_STEPS   1024.0
#define MAX_CHANNEL 5

int progPin = 8;
int upPin = 12;
int downPin = 10;
int myPin = 11;
int selectPin = 9;
int ledPin = 13;


void sendCommand();
double measureVoltage(uint8_t Pin);
int getActiveChannel();
void switchChannelOrWakeUpRemote();
void resetConfiguration();
void switchToChannel(int newChannel);
int getActiveChannelTest();
void setDebugMode();

void pressUp(int holdTime);
void pressDown(int holdTime);
void pressProg(int holdTime);
void pressMy(int holdTime);
void pressSelect(int holdTime);

boolean upPress = false;
boolean downPress = false;
boolean myPress = false;
boolean progPress = false;
boolean selectPress = false;
boolean doChannelSwitch = false;
boolean channelSwitchNecessary = false;       // Temporary indicator if a channelSwitchNecessary is needed
boolean DEBUG = false;

int iActualChannel = 0;                       // Global indicator which is the actual channel
float ledVoltageLevel = 2.0;                  // LED voltage Level. LED on -> measurement = 1.6V, LED off --> measurement = 3.3V
int wantedChannel = 0;                        // New channel to switch to
unsigned long holdTime = 0;
int repeatSend = 0;
char incomingChar;
int debugModeCounter = 0;

void setup() {
  Serial.begin(9600);

  Serial.println("SomfyDuino 0.90");
  Serial.println("adapted by Holger Doering");
  Serial.println("-----");
  Serial.println("receives single serial characters like:");
  Serial.println("c3 - prepare to change to channel 3");
  Serial.println("u  - prepare to press UP");
  Serial.println("d  - prepare to press DOWN");
  Serial.println("m  - prepare to press My");
  Serial.println("p  - prepare to press PROG");
  Serial.println("s  - prepare to press SELECT");
  Serial.println("r  - increase send count by one");
  Serial.println("any digit X - increase hold time by X*100 ms");
  Serial.println("e - Execute! Runs a defined (multi button) press and hold");
  Serial.println("");
  Serial.println("Examples: regular UP: u5e ");            
  Serial.println("          3 second UP/DOWN: ud9993e "); 
  Serial.println("          5 second PROG: p999995e "); 
  Serial.println("          regular up on CH1: c1u5e ");
  Serial.println("          regular down on CH4: c4d5e ");
  Serial.println("");
  
  pinMode(progPin, OUTPUT);
  pinMode(upPin, OUTPUT);
  pinMode(downPin, OUTPUT);
  pinMode(myPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(selectPin, OUTPUT);

  digitalWrite(progPin, HIGH);                    // No buttons pressed, low active (like a real button press, connects the actual pin to GND)
  digitalWrite(upPin, HIGH);
  digitalWrite(downPin, HIGH);
  digitalWrite(myPin, HIGH);
  digitalWrite(selectPin, HIGH);

  digitalWrite(ledPin, LOW);                      // LED off    

  switchChannelOrWakeUpRemote();
  getActiveChannel();


  Serial.print("Actual Channel is: ");
  Serial.println(iActualChannel);
  Serial.print("Debug-Mode is: ");
  Serial.println(DEBUG);
                
}

void loop() {
  
  while (Serial.available() == 0) { 
    delay(50);                                    // wait until serial command comes in
       
  }  
  
  if (Serial.available() > 0) {
    incomingChar = Serial.read();                 // read first byte / char which comes on Serial
    
    if(DEBUG){
      Serial.println(incomingChar);
    }

    if (incomingChar == '#') {setDebugMode();}
    if (incomingChar == 'u') {upPress = true;}    // for every char in the message configure the actions
    if (incomingChar == 'd') {downPress = true;}
    if (incomingChar == 'c') {channelSwitchNecessary = true;} 
    if (incomingChar == 'm') {myPress = true;}
    if (incomingChar == 'p') {progPress = true;}
    if (incomingChar == 's') {selectPress = true;}
    
    if (incomingChar == '1' && channelSwitchNecessary == false) {holdTime += 100;} // if no 'c' found --> channelSwitchNecessary remains false
    else if(incomingChar == '1' && channelSwitchNecessary == true){
        wantedChannel = incomingChar - 48;
        doChannelSwitch = true;
        channelSwitchNecessary = false;                                   // if 'c' found --> channelSwitchNecessary is true, then our number is our channel
    }

    if (incomingChar == '2' && channelSwitchNecessary == false) {holdTime += 200;}
    else if(incomingChar == '2' && channelSwitchNecessary == true){
        wantedChannel = incomingChar - 48;
        doChannelSwitch = true;
        channelSwitchNecessary = false;
    }

    if (incomingChar == '3' && channelSwitchNecessary == false) {holdTime += 300;}
    else if(incomingChar == '3' && channelSwitchNecessary == true){
        wantedChannel = incomingChar - 48;
        doChannelSwitch = true;
        channelSwitchNecessary = false;
    }

    if (incomingChar == '4' && channelSwitchNecessary == false) {holdTime += 400;}
    else if(incomingChar == '4' && channelSwitchNecessary == true){
        wantedChannel = incomingChar - 48;
        doChannelSwitch = true;
        channelSwitchNecessary = false;
    }

    if (incomingChar == '5' && channelSwitchNecessary == false) {holdTime += 500;}
    else if(incomingChar == '5' && channelSwitchNecessary == true){
        wantedChannel = incomingChar - 48;
        doChannelSwitch = true;
        channelSwitchNecessary = false;
    }

    if (incomingChar == '6' && channelSwitchNecessary == false) {holdTime += 600;}
    if (incomingChar == '7' && channelSwitchNecessary == false) {holdTime += 700;}
    if (incomingChar == '8' && channelSwitchNecessary == false) {holdTime += 800;}
    if (incomingChar == '9' && channelSwitchNecessary == false) {holdTime += 900;}
    if (incomingChar == 'r') {repeatSend++;}
    if (incomingChar == 'e') {sendCommand();}         //execute the command
  }
  
}

/**
 * Enables / Disables the debug mode flag
 **/
void setDebugMode(){
  debugModeCounter++;

  if(debugModeCounter == 3){
    if(DEBUG){
      Serial.println("Debug-Mode disabled");
    }
    else{
      Serial.println("Debug-Mode enabled");
    }
    DEBUG = !DEBUG;
    debugModeCounter = 0;
  }
}

/**
 * Executes the action defined by the command sent to arduino
 **/
void sendCommand() {

  if (doChannelSwitch){
    switchToChannel(wantedChannel);
  }

  for(int i=0; i <= repeatSend; i++) {
    if (upPress) {pressUp(holdTime);}                            
    if (downPress) {pressDown(holdTime);}
    if (myPress) {pressMy(holdTime);}
    if (progPress) {pressProg(holdTime);}
    if (selectPress) {pressSelect(holdTime);}
    digitalWrite(ledPin, HIGH);
  }
  resetConfiguration();
  
  if(DEBUG){
    Serial.println("done.");
  }
}

/**
 * Resets all configuration and command flags
 */
void resetConfiguration(){
  upPress = false;                                                              
  downPress = false;
  myPress = false;
  progPress = false;
  selectPress = false;
  doChannelSwitch = false;
  channelSwitchNecessary = false;
  holdTime = 0;
  repeatSend = 0;
}

/**
 * Switches to channel defined by argument
 * @param newChannel The new channel to switch to
 **/
void switchToChannel(int newChannel){
  while(getActiveChannel() != newChannel){
    if(DEBUG){
      Serial.print("Actual Channel is: ");
      Serial.println(iActualChannel);
    }
    switchChannelOrWakeUpRemote();
  }

  if(DEBUG){
    Serial.print("New Actual Channel ");
    Serial.println(newChannel);
  }
}

/**
 * Wakes up the remote or changes the channel via pressing select
 **/
void switchChannelOrWakeUpRemote(){
  if(DEBUG){
    Serial.println("Switching Channel / Waking up remote ");
  }
  pressSelect(100);
}

/**
 * Gets the actual channel as an integer return value and writes global variable iActualChannel accordingly.
 * @return The actual measured channel, -1 if no channel detected
 **/
int getActiveChannel(){
  double allMeasurements[4] = {0.0,0.0,0.0,0.0};
  allMeasurements[0] = measureVoltage(A1);          // Measure all Channels first
  allMeasurements[1] = measureVoltage(A2);
  allMeasurements[2] = measureVoltage(A3);
  allMeasurements[3] = measureVoltage(A4);

  if(allMeasurements[0] < ledVoltageLevel &&        // LED for CH1 is on (1.6 V)
     allMeasurements[1] > ledVoltageLevel &&        // all other LED are off (3.3 V)
     allMeasurements[2] > ledVoltageLevel && 
     allMeasurements[3] > ledVoltageLevel){ 
     iActualChannel = 1;
    return 1;
  }

  if(allMeasurements[0] > ledVoltageLevel &&        
     allMeasurements[1] < ledVoltageLevel &&        // LED for CH2 is on (1.6 V)
     allMeasurements[2] > ledVoltageLevel && 
     allMeasurements[3] > ledVoltageLevel){
     iActualChannel = 2; 
    return 2;
  }

  if(allMeasurements[0] > ledVoltageLevel &&        
     allMeasurements[1] > ledVoltageLevel &&        
     allMeasurements[2] < ledVoltageLevel &&        // LED for CH3 is on (1.6 V)
     allMeasurements[3] > ledVoltageLevel){ 
     iActualChannel = 3;
    return 3;
  }

  if(allMeasurements[0] > ledVoltageLevel &&        
     allMeasurements[1] > ledVoltageLevel &&        
     allMeasurements[2] > ledVoltageLevel &&        
     allMeasurements[3] < ledVoltageLevel){         // LED for CH4 is on (1.6 V)
     iActualChannel = 4;
    return 4;
  }

  // All LEDs are on --> measurement (around 1.6 V) is smaller then Level --> 2.0
  if(allMeasurements[0] < ledVoltageLevel &&        // all LEDs < 1,8 V = alle an --> CH5
     allMeasurements[1] < ledVoltageLevel && 
     allMeasurements[2] < ledVoltageLevel && 
     allMeasurements[3] < ledVoltageLevel){ 
     iActualChannel = 5;
    return 5;
    }

  iActualChannel = -1;
  return -1;
}

/**
 * Measures the specified analog pin
 * @param Pin AD pin of arduino which is measured
 * @return Measured voltage of the analog pin
 **/
double measureVoltage(uint8_t Pin){
  delay(10);
  if(DEBUG){
    Serial.print("Measuring ");
    Serial.println(Pin);
  }
  double voltage = (analogRead(Pin)*REF_VOLTAGE/PIN_STEPS);
  if(DEBUG){
    Serial.print("U = ");
    Serial.print(voltage);
    Serial.println(" V");
  }
  return voltage;
}

/**
 * Presses the UP (^) button of the remote virtually
 * @param holdTime The time (ms) how long the button is pressed
 **/
void pressUp(int holdTime){
  digitalWrite(upPin, LOW); 
  if(DEBUG){
    Serial.print("Pressing Up-Key for ");
    Serial.print(holdTime);                                                     
    Serial.println(" ms");
    }
  delay(holdTime);
  digitalWrite(upPin, HIGH); 
  delay(holdTime);
}

/**
 * Presses the DOWN (v) button of the remote virtually
 * @param holdTime The time (ms) how long the button is pressed
 **/
void pressDown(int holdTime){
  digitalWrite(downPin, LOW); 
  if(DEBUG){
    Serial.print("Pressing Down-Key for ");
    Serial.print(holdTime);                                                     
    Serial.println(" ms");
    }
  delay(holdTime);
  digitalWrite(downPin, HIGH); 
  delay(holdTime);
}

/**
 * Presses the MY button of the remote virtually
 * @param holdTime The time (ms) how long the button is pressed
 **/
void pressMy(int holdTime){
  digitalWrite(myPin, LOW); 
  if(DEBUG){
    Serial.print("Pressing My-Key for ");
    Serial.print(holdTime);                                                     
    Serial.println(" ms");
    }
  delay(holdTime);
  digitalWrite(myPin, HIGH); 
  delay(holdTime);
}

/**
 * Presses the PROG (PRG) button of the remote virtually
 * @param holdTime The time (ms) how long the button is pressed
 **/
void pressProg(int holdTime){
  digitalWrite(progPin, LOW); 
  if(DEBUG){
    Serial.print("Pressing Prog-Key for ");
    Serial.print(holdTime);                                                     
    Serial.println(" ms");
    }
  delay(holdTime);
  digitalWrite(progPin, HIGH); 
  delay(holdTime);
}

/**
 * Presses the SELECT button of the remote virtually
 * @param holdTime The time (ms) how long the button is pressed
 **/
void pressSelect(int holdTime){
  digitalWrite(selectPin, LOW); 
  if(DEBUG){
    Serial.print("Pressing Select-Key for ");
    Serial.print(holdTime);                                                     
    Serial.println(" ms");
    }
  delay(holdTime);
  digitalWrite(selectPin, HIGH); 
  delay(holdTime);
}

