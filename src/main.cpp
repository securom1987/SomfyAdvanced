#include <Arduino.h>
#include <Hardware.hpp>

// SomfyDuino Test Test Test
// by Bjoern Foldenauer
// 05/2017

#define DEBUG true

void sendCommand();
void wakeUpRemote();
double measureVoltage(uint8_t Pin);
int getActiveChannel();
void pressSelect();

//Voltage Measuring Pins A0 = Ch1 (zB. Wohnzimmer), A1 = Ch2 (Küchentür), A2 = Ch3 (Küchenfenster), A3 = Ch4(Bad), A4 = Ch5 (Oben)
#define REF_VOLTAGE    5.0
#define PIN_STEPS   1024.0
#define MAX_CHANNEL 5

int iActualChannel = 0;           // indicator which is the actual Channel
float measuredLEDVoltage = 2.0;   // LED Voltage Level, when LED is Switched on

int progPin = 8;
int upPin = 12;
int downPin = 10;
int myPin = 11;
int selectPin = 9;
int ledPin = 13;


boolean upPress = false;
boolean downPress = false;
boolean myPress = false;
boolean progPress = false;
boolean selectPress = false;
boolean channelSwitch = false;      // indicator if a channelSwitch is needed
int wantedChannel = 0;              // new Channel to switch to
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

  
  wakeUpRemote();
  //iActualChannel = getActiveChannel();

  if(DEBUG){
    Serial.println("Actual Channel is: " + iActualChannel);
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
  digitalWrite(ledPin, LOW);    
                
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
                                                  // c3u5e
    if (incomingChar == 'u') {upPress = true;}    // for every char in the message configure the actions
    if (incomingChar == 'd') {downPress = true;}
    if (incomingChar == 'c') {channelSwitch = true;} 
    if (incomingChar == 'm') {myPress = true;}
    if (incomingChar == 'p') {progPress = true;}
    if (incomingChar == 's') {selectPress = true;}
    
    if (incomingChar == '1' && channelSwitch == false) {holdTime += 100;} // if no 'c' found --> channelSwitch remains false
    else if(incomingChar == '1' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}                                   // if 'c' found --> channelSwitch is true, then our number is our channel
    
    if (incomingChar == '2' && channelSwitch == false) {holdTime += 200;}
    else if(incomingChar == '2' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}

    if (incomingChar == '3' && channelSwitch == false) {holdTime += 300;}
    else if(incomingChar == '3' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '4' && channelSwitch == false) {holdTime += 400;}
    else if(incomingChar == '4' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '5' && channelSwitch == false) {holdTime += 500;}
    else if(incomingChar == '5' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '6' && channelSwitch == false) {holdTime += 600;}
    else if(incomingChar == '6' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '7' && channelSwitch == false) {holdTime += 700;}
    else if(incomingChar == '7' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '8' && channelSwitch == false) {holdTime += 800;}
    else if(incomingChar == '8' && channelSwitch == true){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == '9' && channelSwitch == false) {holdTime += 900;}
    else if(incomingChar == '9' && channelSwitch == false){
        wantedChannel = incomingChar;
        channelSwitch = false;}
    
    if (incomingChar == 'r') {repeatSend++;}
    if (incomingChar == 'e') {sendCommand();}         //execute the command
  }
}

void sendCommand() {
  if(DEBUG){
    Serial.print("Pressing: ");
  }

  for(int i=0; i <= repeatSend; i++) {
    if (upPress) {digitalWrite(upPin, LOW); if(DEBUG){Serial.print("up ");}}    //pressing button virtually            
    if (downPress) {digitalWrite(downPin, LOW); if(DEBUG){Serial.print("down ");}}
    if (myPress) {digitalWrite(myPin, LOW); if(DEBUG){Serial.print("my ");}}
    if (progPress) {digitalWrite(progPin, LOW); if(DEBUG){Serial.print("prog ");}}
    if (selectPress) {digitalWrite(selectPin, LOW); if(DEBUG){Serial.print("select ");}}
    digitalWrite(ledPin, HIGH);

    if(DEBUG){
      Serial.print("for ");
      Serial.print(holdTime);                                                     
      Serial.println(" ms");
    }

    delay(holdTime);                                                            // how long should it be pressed

    digitalWrite(progPin, HIGH);                                                // unpressing the/all button again
    digitalWrite(upPin, HIGH);
    digitalWrite(downPin, HIGH);
    digitalWrite(myPin, HIGH);
    digitalWrite(selectPin, HIGH);
    digitalWrite(ledPin, LOW);

    delay(holdTime);                                                            // again wait some time
  }
  
  upPress = false;                                                              // resetting configuration
  downPress = false;
  myPress = false;
  progPress = false;
  selectPress = false;
  

  holdTime = 0;
  repeatSend = 0;
  
  if(DEBUG){
    Serial.println("done.");
  }
}

int switchToChannel(int newChannel){
  while(getActiveChannel() != newChannel){
    pressSelect();
  }
  return 0;
}

//Press select to switch the Channel
void pressSelect(){
  digitalWrite(selectPin, LOW);
  delay(100);
  digitalWrite(selectPin, HIGH);  
  delay(100);
}

// Get actual Channel
int getActiveChannel(){

  if(measureVoltage(A0) > measuredLEDVoltage){ //measure LED1
    iActualChannel = 0;
    return iActualChannel;
  }
  if(measureVoltage(A1) > measuredLEDVoltage){ //measure LED2
    iActualChannel = 1;
    return iActualChannel;
  }
  if(measureVoltage(A2) > measuredLEDVoltage){ //measure LED3
    iActualChannel = 2;
    return iActualChannel;
  }
  if(measureVoltage(A3) > measuredLEDVoltage){ //measure LED4
    iActualChannel = 3;
    return iActualChannel;
  }
  if(measureVoltage(A4) > measuredLEDVoltage){ //measure LED5
    iActualChannel = 4;
    return iActualChannel;
  }
  // No Voltage measured, all LEDs were off
  iActualChannel = -1;
  return iActualChannel;
}

double measureVoltage(uint8_t Pin){
  delay(10);
  double voltage = (analogRead(Pin)*REF_VOLTAGE/PIN_STEPS);
  if(DEBUG){
    Serial.println("U = ");
    Serial.print(voltage);
    Serial.println(" V");
  }
  return voltage;
}

// In case there is no voltage measured, no LED is glowing, and Remote is sleeping... hopefully, needs to be tested
void wakeUpRemote(){
  if(getActiveChannel() == -1){
    digitalWrite(selectPin, LOW);
    delay(20);
    digitalWrite(selectPin, HIGH);  
    delay(20);
  }
}
