
/**
 * David Harbottle
 * March 28, 2015
 * 
 * 
 * The only thing we need is some ABS friendly lube for the
 * worm to turn the wyrm gear at pwm-15.
 * 
 * At minimum pwm-19, rolling over (or overshoot) is 12-25 count,
 * over 1 revolution
 * 
 * 
 * -=Arduino UNO=-
 * D13 - 12ME.1.CLK , 12ME.2.CLK
 * D12 - 12ME.1.CS , 12ME.2.CS
 * D11 - 12ME.1.DATA
 * D10 - 12ME.2.DATA
 * D09 - MD1.PWM
 * D08 - MD1.INA
 * D07 - MD1.INB
 * D06 - MD2.PWM
 * D05 - MD2.INA
 * D04 - MD2.INB
 * D03 - MQ1.SIG
 * D02 - MQ2.SIG
 * D01 - UNUSED
 * D00 - UNUSED
 * 
 * A0 - LSOVR
 * A1 - CMP.MD1.CS
 * A2 - CMP.MD2.CS
 * A3 - LSOUT
 * A4 - Hall.DATA
 * A5 - UNUSED
 * 
 * 
 **/


#include <QueueList.h>
#include <DavidMotor.h>

#define SELECT_PIN 12
#define CLOCK_PIN 13
#define DATA_PIN 11

#define MD1_PWM 9
#define MD1_INA 8
#define MD1_INB 7

#define MD2_PWM 6
#define MD2_INA 5
#define MD2_INB 4

//#define QUAD_A_PIN 2 //not used
//#define QUAD_B_PIN 3 //not used

#define HALL_PIN A4

#define MD1CS_PIN A1
#define MD2CS_PIN A2

#define LSOUT A3
#define LSOVR A0

DavidMotor davmot0 (MD1_PWM, MD1_INA, MD1_INB);
DavidMotor davmot1 (MD2_PWM, MD2_INA, MD2_INB);

//struct dMotorOp {
//  byte moperation;
//  int motor0PWM;
//  int motor1PWM;
//  long motor0Arg;
//  long motor1Arg;
//};

struct dMotorOp {
  int motorOPWM;
  int motor1PWM;
  float angle0;
  float angle1;
};

struct myAngle {
  int mdegrees;
  int minutes;
  int seconds;
};


//assume valid input
//input is the 12-bit mag encoder's output
struct myAngle countToAngle (float input){
  myAngle temp;
  float temp2 = (input / 4096.0 ) * 360.0;
  temp.mdegrees = temp2;
  temp2 = (temp2 - temp.mdegrees) * 60.0;
  temp.minutes = temp2;
  temp2 = (temp2 - temp.minutes) * 60.0;
  temp.seconds = temp2;
  return temp;
}


//assume valid input
//intput is a struct myAngle, {degrees, minutes, seconds}
float angleToCount (struct myAngle input) {
  float temp = 0.0 + input.mdegrees;
  temp = temp + (input.minutes / 60.0);
  temp = temp + (input.seconds / 3600.0);
  temp = (temp * 4096.0) / 360.0;
  return temp;
}


int getCWDistance (int current, int target) {
  return (target - current + ((target < current)*4096));
}

//SAVE MEMORY - MERGE SIMILAR FUNCTIONS
int getCCWDistance (int current, int target) {
  return (current - target + ((target > current)*4096));
}


void motorGOCW (int inputMECount) {
  int temp;
  int tempspeed;

  while (true) {
    temp = getCWDistance (getMECount(), inputMECount);
    if  (temp >= 100)
      tempspeed = 255; 
    else if (temp >= 40)
      tempspeed = 100;
    else if (temp >= 1)
      tempspeed = 20;
    else
      break;
    davmot1.motorGo(tempspeed);
    delay(1);
  }
  davmot1.motorGo(0);
}

//SAVE MEMORY - MERGE SIMILAR FUNCTIONS
void motorGOCCW (int inputMECount) {
  int temp;
  int tempspeed;
  
  while (true) {
    temp = getCWDistance (inputMECount, getMECount());
    if  (temp >= 100)
      tempspeed = -255; 
    else if (temp >= 40)
      tempspeed = -100;
    else if (temp >= 1)
      tempspeed = -20;
    else
      break;
    davmot1.motorGo(tempspeed);
    delay(10);
  }
  davmot1.motorGo(0);
}


int getMECount() {
  return readPosition();
}


//read the current angular position, binary, 16 bits
unsigned int readPosition()
{
  unsigned int posbyte = 0;

  //There is a function in the Arduino Lib that does the following


  //shift in our data  
  digitalWrite(SELECT_PIN, LOW);
  delayMicroseconds(1);
  byte d1 = shiftIn(DATA_PIN, CLOCK_PIN); //read in 8 bits
  byte d2 = shiftIn(DATA_PIN, CLOCK_PIN); //read in 8 bits
  digitalWrite(SELECT_PIN, HIGH);

  //Bit Shifting, 16 bit unsigned int
  posbyte = d1;
  posbyte = posbyte << 8;
  posbyte |= d2;

  //the encoder only gives 12 bits for position
  //shift back 4 bits
  posbyte = posbyte >> 4;

  return posbyte;
}


//read in a byte (8bits) of data from the digital input of the board.
byte shiftIn(byte data_pin, byte clock_pin)
{
  byte data = 0;

  for (int i=7; i>=0; i--)
  {
    digitalWrite(clock_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(clock_pin, HIGH);
    delayMicroseconds(1);

    //read byte when clock signal goes from LOW to HIGH
    byte dbit = digitalRead(data_pin);

    data |= (dbit << i);
  }

  return data;
}

//struct dMotorOp2 myMOP2[3];

QueueList <dMotorOp> queue;

volatile long countA;
volatile long countB;

long oldcount;


void setup()
{  
  dMotorOp dtemp;
  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(SELECT_PIN, OUTPUT);

  pinMode(MD1_PWM, OUTPUT);
  pinMode(MD1_INA, OUTPUT);
  pinMode(MD1_INB, OUTPUT);

  pinMode(MD2_PWM, OUTPUT);
  pinMode(MD2_INA, OUTPUT);
  pinMode(MD2_INB, OUTPUT);

  //give some default values
  digitalWrite(CLOCK_PIN, HIGH);
  digitalWrite(SELECT_PIN, HIGH);

  analogWrite(MD1_PWM, 0);
  digitalWrite(MD1_INA, LOW);
  digitalWrite(MD1_INB, LOW);

  analogWrite(MD2_PWM, 0);
  digitalWrite(MD2_INA, LOW);
  digitalWrite(MD2_INB, LOW);

  //setup pins for Quadrature, not used
  //pinMode(QUAD_A_PIN, INPUT);
  //pinMode(QUAD_B_PIN, INPUT);

  pinMode(HALL_PIN, INPUT);

  pinMode(LSOUT, INPUT);
  pinMode(LSOVR, OUTPUT);



  Serial.begin(19200); //Test Code

  delay(500);
  Serial.println("3...");  
  delay(500);
  Serial.println("2..."); 
  delay(500);
  Serial.println("1..."); 
  delay(500);
  Serial.println("0..."); 

  delay(500);
  Serial.println("---==Setup Begin==---");  
  delay(500);

  //  myAngle mangle = {    
  //    60, 22, 15              };
  //  Serial.println(angleToCount (mangle));
  //
  //  angleToCount(mangle);
  //  countToAngle(686);

  //  attachInterrupt(0, incrementCount0, HIGH);  
  //  attachInterrupt(1, incrementCount1, HIGH);


  //  davmot0.motorGo(0);
  //  davmot1.motorGo(0);  
  //  delay(500);
  //
  //  dMotorOp dmop0 = {
  //    0, 0, 0.0, 0.0      };
  //  dMotorOp dmop1 = {
  //    0, 0, 0.0, 0.0      };
  //  dMotorOp dmop2 = {
  //    0, 0, 0.0, 0.0      };
  //  dMotorOp dmop3 = {
  //    0, 0, 0.0, 0.0      };
  //
  //
  //  queue.push (dmop0);  
  //  queue.push (dmop1);
  //  queue.push (dmop2);
  //  queue.push (dmop3);


  //120 bytes difference between push2 and push3
  //  queue.push (dmop3); 
  //
  //  while (!queue.isEmpty()) {
  //
  //    dtemp = queue.pop();
  //    Serial.println (printdmop(dtemp));
  //
  //  }

  Serial.println("---==Loop Begin==---");  
  Serial.println("Please enter a value between -4095 and 4095"); 
  Serial.println("A negative value signals to rotate counterclockwise");
  delay(500);
  //davmot1.motorGo(20);
}

int incomingByte = 0;


void loop()
{ 
  int temp = 0;
  int temp2 = readPosition();
  int temp3 = 0;
  Serial.print("current: ");
  Serial.println(temp2);
  delay(2000);

  if (Serial.available() > 0) {
    temp = (Serial.parseInt());

    
    Serial.print("target : ");
    Serial.println(temp);
    
    //a positive input is a signal to go clockwise
    //but a negative input is a signal to go counterclockwise

    if (temp == temp2) return;
    
    //STRANGE - cuz of pipelining, it doesn't compute -temp
    //fast enough, and I end up with a wrong value
    //temp3 = -temp;
    //THAT - or we re-order motorGOCCW and motorGOCW

    
    if (temp < 0)
      motorGOCCW (-temp);
    else if (temp > 0)
      motorGOCW (temp);
    else {
      //it sucks, but we have to make an arbitrary decision
      if (temp2 >= 2048)
        motorGOCCW (-temp);
      else
        motorGOCW (temp);
    }
  }
}


String printdmop (struct dMotorOp& dtemp) {
  String temp = "";
  //temp = temp + "-" + dtemp.moperation;
  temp = temp + "-" + dtemp.motorOPWM;
  temp = temp + "-" + dtemp.motor1PWM;
  //temp = dtemp.angle0;
  //temp = temp + "-" + dtemp.angle1; 
  return temp;
}


float baseSpeed(){
  unsigned long time0;  
  unsigned long time1;
  long count0;
  long count1; 

  time0 = millis();
  count0 = countB;

  delay(20000);

  time1 = millis();
  count1 = countB;

  //16 counts per rev, 1000 msec per sec -> 62.5 scale, to get
  //revolutions per second, return value units
  return 62.5 * ((float)(count1 - count0) / (float)(time1 - time0));
}


//WORKS
void incrementCount0(){
  countA++;
}
void incrementCount1(){
  countB++;
}
void decrementCount0(){
  countA--;
}
void decrementCount1(){
  countB--;
}











