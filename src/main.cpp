#include <Arduino.h>

// SomfyDuino
// by Bjoern Foldenauer
// adapted by Holger Döring
// 11/2021

#define TESTMODE false
#define DEBUG false

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

int getActiveChannelTest();

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
float ledVoltageLevel = 2.0;     // LED voltage Level, when LED is Switched on
int wantedChannel = 0;              // New channel to switch to
unsigned long holdTime = 0;
int repeatSend = 0;

char incomingChar;

void setup() {
  Serial.begin(9600);

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
  Serial.println("Example: regular UP: c[1-5]u5e ");            //-->c[1-5]u5e
  Serial.println("         3 second UP/DOWN: c[1-5]ud9993e ");  //-->c[1-5]ud9993e 
  Serial.println("         5 second PROG: c[1-5]p999995e ");
  Serial.println("");
  
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

  switchChannelOrWakeUpRemote();
  getActiveChannel();


  Serial.print("Actual Channel is: ");
  Serial.println(iActualChannel);
 
                
}

void loop() {

  while (TESTMODE){

    delay(1000);
    // =========== Measuring Test, all Channels
    //Serial.print("Pressing Switch Button: ");
    //switchChannelOrWakeUpRemote();
    //Serial.print("Active Channel: ");
    //Serial.println(getActiveChannel());

    // =========== Test, Switchen to specified Channels

    switchToChannel(3);
    delay(2000);
    switchToChannel(5);
    delay(2000);
    switchToChannel(2);
    delay(2000);
    switchToChannel(5);
    delay(2000);
    switchToChannel(1);
    delay(1000);

  }
  
  while (Serial.available() == 0) { 
    delay(50);                                    // wait until serial command comes in
       
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
  channelSwitchNecessary = false;
  holdTime = 0;
  repeatSend = 0;
}

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

//Press select to switch the Channel
void switchChannelOrWakeUpRemote(){
  if(DEBUG){
    Serial.println("Switching Channel / Waking up remote ");
  }
  pressSelect(100);
}

int getActiveChannelTest(){
  if(measureVoltage(A1) < ledVoltageLevel){ //measure LED1
    iActualChannel = 0;
    return iActualChannel;
  }
}

// Get actual Channel
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

double measureVoltage(uint8_t Pin){
  delay(10);
  /*if(DEBUG){
    Serial.print("Measuring ");
    Serial.println(Pin); //14
  }*/

  double voltage = (analogRead(Pin)*REF_VOLTAGE/PIN_STEPS);
  
  /*if(DEBUG){
    Serial.print("U = ");
    Serial.print(voltage);
    Serial.println(" V");
  }*/
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

