//Library Imports
#include <EEPROM.h>
#include <CapSense.h>
#include <Servo.h> 



int CAP_SWITCH_SEND = 4;            //Pin number for Cap Sense Send signal
int CAP_SWITCH_RECEIVE = 2;         //Pin number for Cap Sense Receive signal
int SERVO_PIN = 14;                 //Pin number Servo Logic is attached to

int machineState = 5;
int buttonState = 0;
int lastState = -1;
long last_value = 0;
int lock_state = 0;

int hand_state = 0;

long count_total = 0;
long counter_total = 0;
long counter_highest = 0;

long highest = 0;
long range = 0;
unsigned long door_reset = 0;

//Knock Counters
int count = 0;
int count_read = 0;

int onTime = 0;
int offTime = 0;
unsigned long onStartPress = 0;
unsigned long offStartPress = 0;
int knock_array[10];

CapSense cs_switch = CapSense(CAP_SWITCH_SEND, CAP_SWITCH_RECEIVE); 
Servo door_servo;

void setup(){
  count_read = 0;
  count = 0;
  
  Serial.begin(9600);
  cs_switch.set_CS_Timeout_Millis(2000);
  
  readMem();
  change_lock(0);
  change_lock(1);
}

void get_hand_state(){
  long value = (last_value+cs_switch.capSense(30))/2;

  if(value > highest){
    highest = value;
    range = (highest-counter_total);
  }
  
  if(machineState == 5){
    get_baseline(value);
  }else{
    if(value > (counter_total*3) && hand_state == 0){
      hand_state = 1;
      buttonState = HIGH;
      //Serial.println("HIGH");
    }else if(value < counter_total+(counter_highest/2)){
      door_reset = millis();
      if(hand_state == 1){
        hand_state = 0;
        buttonState = LOW;
        //Serial.println("LOW");
      }
    }
    
    if(millis()-door_reset > 2000 && value < (highest-(range/1.5)) && machineState == 0){
      Serial.println("RESET ME"); 
      reset();
    }
  }
  
  if (lastState == -1){ lastState = buttonState; }
  last_value = value;

  if(offTime > 30000 && machineState == 0){
    Serial.println("CATS");
    reset();
  }
}

void loop(){
  get_hand_state();
  
  if (machineState == 1){
    if (buttonState != lastState){
      if (buttonState == HIGH){
        if(offTime > 0){
          Serial.println(offTime);
          knock_array[count] = offTime;
          count += 1; 
        }
      }
    }
  }else if (machineState == 0){
    if (buttonState != lastState){
      if (buttonState == HIGH){
        if(offTime > 0){
          int val1 = knock_array[count_read]+200;
          int val2 = knock_array[count_read]-200;
          if (offTime < val1 && offTime > val2){
            count_read += 1;
            if(knock_array[count_read] <= 0){
              Serial.println("Open"); 
              count_read = 0;
              change_lock(0);
              delay(2000);
              change_lock(1);
            }else{
              Serial.println("Next Step");
            }
          }else{
            count_read = 0; 
            Serial.println("Reset");
          }
        }    
      }
    }  
  }
  
  lastState = buttonState;
  calTime();  
  checkTimeouts();
}

void checkTimeouts(){
  if (onTime > 5000 && machineState == 0 && buttonState == HIGH){
    count = 0;
    clearMem();
    readMem();

    machineState = 1;
    change_lock(0);
    change_lock(1);
    Serial.println("Record");
  }
  
  if (offTime > 5000 && machineState == 1 && buttonState == LOW){
    count = 0;
    count_read = 0;
    clearMem();
    saveMem();
    readMem();
    machineState = 0;
    change_lock(0);
    change_lock(1);
    Serial.println("Finished Record");
  }
  
  if (offTime > 3000 && buttonState == LOW && machineState == 0){
    count_read = 0;  
  }
}

void calTime(){
  offTime = 0;
  onTime = 0;

  if (buttonState == LOW) {
    onStartPress = 0;
    if (offStartPress == 0) { // check if continued press
      offStartPress = millis();  // if not, set timer
    }else{
      offTime = (millis() - offStartPress);
    }
  }else{
    offStartPress = 0;
    if (onStartPress == 0) { // check if continued press
      onStartPress = millis();  // if not, set timer
    }else{
      onTime = (millis() - onStartPress);
    }
  } 
}

void readMem(){
  for (int i = 0; i< (sizeof(knock_array)/sizeof(int)); i++){
      knock_array[i] = 0;
      knock_array[i] = EEPROMReadInt(i);
      Serial.println((int) knock_array[i]);
  }
}

void saveMem(){
  for (int i = 0; i< (sizeof(knock_array)/sizeof(int)); i++){
      EEPROMWriteInt(i, knock_array[i]);
  }
}

void clearMem(){
  for (int i = 0; i< (sizeof(knock_array)/sizeof(int)); i++){
      EEPROMWriteInt(i, -1);
  }
}

void EEPROMWriteInt(int p_address, int p_value){
  /*  EEPROMWriteInt - Function
   *    Writes the passed two byte int value to EEPROM memory at the specified location
   *
   *  @param p_address: The position in memory the vlaue is to be stored
   *  @type p_address: int
   *
   *  @param p_value: A two byte int to be stored within memory
   *  @type p_value: int   
   */
  p_address = 2*(p_address+1);
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);	
}

unsigned int EEPROMReadInt(int p_address){
  /*  EEPROMReadInt - Function
   *    Returns a two bytes int found at the passed memory location
   *
   *  @param p_address: The position in memory the stored value is located
   *  @type p_address: int
   *
   *  @return The stored int value lcoated at the specified location
   *  @rtype int
   */
  p_address = 2*(p_address+1);
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void change_lock(int state){
  /*  change_lock - Function
   *    Changes the current lock to the specified state (0 - Open, 1 - Locked)
   *
   *  @param state: The new state the lock should be changed to
   *  @type state: int
   */  
  if(lock_state != state){
    if(state == 0){
      door_servo.attach(SERVO_PIN);
      door_servo.write(170); 
      delay(1000); 
      door_servo.detach(); 
      lock_state = 0;
      EEPROM.write(0, 0);
    }else{
      door_servo.attach(SERVO_PIN);
      door_servo.write(70); 
      delay(1000); 
      door_servo.detach();
      lock_state = 1;
      EEPROM.write(0, 1);
    }
    door_reset = millis();
  }
}

void reset(){
  offStartPress = millis();
  onStartPress = millis();
  machineState = 5;
  counter_total = 0;
  counter_highest = 0;
  count_total = 0;
  highest = 0;
}

void get_baseline(long value){
  counter_total += value;
  count_total += 1;
  if(value > counter_highest){
   counter_highest = value; 
  }
  if(count_total == 100){
    machineState = 0; 
    counter_total = (counter_total/101);
  } 
}

