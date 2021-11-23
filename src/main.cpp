#include <Arduino.h>

// SomfyDuino
// by Bjoern Foldenauer
// adapted by Holger Döring
// 11/2021

#define DEBUG true

// Voltage Measuring Pins A0 = Ch1 (zB. Wohnzimmer), A1 = Ch2 (Küchentür), A2 = Ch3 (Küchenfenster), A3 = Ch4(Bad), A4 = Ch5 (Oben)
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

boolean channelSwitchNecessary = false;      // Temporary indicator if a channelSwitchNecessary is needed

int iActualChannel = 0;             // Indicator which is the actual Channel
float measuredLedVoltage = 1.0;     // LED voltage Level, when LED is Switched on
int wantedChannel = 0;              // New channel to switch to
unsigned long holdTime = 0;
int repeatSend = 0;

char incomingChar;

void setup() {
  Serial.begin(9600);

  if(DEBUG){
    Serial.println("SomfyDuino 0.71");
    Serial.println("adapted by Holger Doering");
    Serial.println("-----");
    Serial.println("receives single serial characters like:");
    Serial.println("u - prepare to press UP");
    Serial.println("d - prepare to press DOWN");
    Serial.println("m - prepare to press My");
    Serial.println("p - prepare to press PROG");
    Serial.println("s - prepare to press SELECT");
    Serial.println("r - increase send count by one");
    Serial.println("any digit X - increase hold time by X*100 ms");
    Serial.println("e - Execute! Runs a defined (multi button) press and hold");
    Serial.println("");
    Serial.println("Example: regular UP: u5e ");            //-->c[1-5]u5e
    Serial.println("         3 second UP/DOWN: ud9993e ");  //-->c[1-5]ud9993e 
    Serial.println("         5 second PROG: p999995e ");
    Serial.println("");
  }

  switchChannelOrWakeUpRemote();

  if(DEBUG){
    Serial.print("Actual Channel is: ");
    Serial.println(iActualChannel);
  }

  pinMode(progPin, OUTPUT);
  pinMode(upPin, OUTPUT);
  pinMode(downPin, OUTPUT);
  pinMode(myPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(selectPin, OUTPUT);

  digitalWrite(progPin, HIGH);                    //no buttons pressed, low active (a real button press, connects the actual pin to GND)
  digitalWrite(upPin, HIGH);
  digitalWrite(downPin, HIGH);
  digitalWrite(myPin, HIGH);
  digitalWrite(selectPin, HIGH);

  digitalWrite(ledPin, LOW);                      // LED off    
                
}

void loop() {
  
  while (Serial.available() == 0) { 
    delay(50);                                    // wait until serial command comes in
    //switchChannelOrWakeUpRemote();
    //delay(5000);
  }  
  
  if (Serial.available() > 0) {
    incomingChar = Serial.read();                 // read first byte / char which comes on Serial
    
    if(DEBUG){
      Serial.println(incomingChar);
    }
                                                  // c3u5e
    if (incomingChar == 'u') {upPress = true;}    // for every char in the message configure the actions
    if (incomingChar == 'd') {downPress = true;}
    if (incomingChar == 'c') {channelSwitchNecessary = true;} 
    if (incomingChar == 'm') {myPress = true;}
    if (incomingChar == 'p') {progPress = true;}
    if (incomingChar == 's') {selectPress = true;}
    
    if (incomingChar == '1' && channelSwitchNecessary == false) {holdTime += 100;} // if no 'c' found --> channelSwitchNecessary remains false
    else if(incomingChar == '1' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}                                   // if 'c' found --> channelSwitchNecessary is true, then our number is our channel
    
    if (incomingChar == '2' && channelSwitchNecessary == false) {holdTime += 200;}
    else if(incomingChar == '2' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}

    if (incomingChar == '3' && channelSwitchNecessary == false) {holdTime += 300;}
    else if(incomingChar == '3' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '4' && channelSwitchNecessary == false) {holdTime += 400;}
    else if(incomingChar == '4' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '5' && channelSwitchNecessary == false) {holdTime += 500;}
    else if(incomingChar == '5' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '6' && channelSwitchNecessary == false) {holdTime += 600;}
    else if(incomingChar == '6' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '7' && channelSwitchNecessary == false) {holdTime += 700;}
    else if(incomingChar == '7' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '8' && channelSwitchNecessary == false) {holdTime += 800;}
    else if(incomingChar == '8' && channelSwitchNecessary == true){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == '9' && channelSwitchNecessary == false) {holdTime += 900;}
    else if(incomingChar == '9' && channelSwitchNecessary == false){
        wantedChannel = incomingChar;
        doChannelSwitch = true;
        channelSwitchNecessary = false;}
    
    if (incomingChar == 'r') {repeatSend++;}
    if (incomingChar == 'e') {sendCommand();}         //execute the command
  }
}

void sendCommand() {

  for(int i=0; i <= repeatSend; i++) {
    if (doChannelSwitch){switchToChannel(wantedChannel);}
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

// Resetting configuration
void resetConfiguration(){
  upPress = false;                                                              
  downPress = false;
  myPress = false;
  progPress = false;
  selectPress = false;
  doChannelSwitch = false;
  holdTime = 0;
  repeatSend = 0;
}

void switchToChannel(int newChannel){
  while(getActiveChannel() != newChannel){
    if(DEBUG){
      Serial.print("Actual Channel ");
      Serial.println(iActualChannel);
    }
    switchChannelOrWakeUpRemote();
  }

  if(DEBUG){
    Serial.print("New Actual Channel ");
    Serial.println(newChannel);
  }

  // Hopefully we are at the correct channel now, so a channel switch is done!
  // doChannelSwitch = false;
}

//Press select to switch the Channel
void switchChannelOrWakeUpRemote(){
  if(DEBUG){
    Serial.print("Switching Channel / Waking up remote ");
  }
  pressSelect(100);
}

// Get actual Channel
int getActiveChannel(){
  if(measureVoltage(A0) > measuredLedVoltage){ //measure LED1
    iActualChannel = 0;
    return iActualChannel;
  }
  if(measureVoltage(A1) > measuredLedVoltage){ //measure LED2
    iActualChannel = 1;
    return iActualChannel;
  }
  if(measureVoltage(A2) > measuredLedVoltage){ //measure LED3
    iActualChannel = 2;
    return iActualChannel;
  }
  if(measureVoltage(A3) > measuredLedVoltage){ //measure LED4
    iActualChannel = 3;
    return iActualChannel;
  }
  if(measureVoltage(A4) > measuredLedVoltage){ //measure LED5
    iActualChannel = 4;
    return iActualChannel;
  }
  // No Voltage measured, all LEDs were off
  iActualChannel = -1;
  return iActualChannel;
}

double measureVoltage(uint8_t Pin){
  delay(10);
  if(DEBUG){
    Serial.print("Measuring ");
    Serial.println(Pin);
  }

  double voltage = (analogRead(A0)*REF_VOLTAGE/PIN_STEPS);
  
  if(DEBUG){
    Serial.print("U = ");
    Serial.print(voltage);
    Serial.println(" V");
  }
  return voltage;
}

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

