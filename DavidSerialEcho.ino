#include <DavidMotor.h>

#define bufflen 8
#define codelen 16

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

DavidMotor davmotAlt (MD1_PWM, MD1_INA, MD1_INB);
DavidMotor davmotAzm (MD2_PWM, MD2_INA, MD2_INB);

//=============================================================//

// 00000;11111;22222;33333;44444;55555;66666;77777;88888;99999;

// queue works, Arduino Serial buffer still limited at 62 bytes
byte byteArray[bufflen][codelen] = { 
};

int stringCount = 0;
int i = 0;
int current = 0;
int nextIn = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  pinMode(13, OUTPUT);
  digitalWrite(13,LOW);

  delay(1000);

  Serial.println("start");
}


//round robin index
void currentPlus() {
  current = current + 1;
  if (current >= bufflen)
    current = 0;
}


void nextInPlus() {
  nextIn = nextIn + 1;
  if (nextIn >= bufflen)
    nextIn = 0;
}


void timeCheck() {
  unsigned long timestart = micros();
  unsigned long timeend = micros() - timestart;
  Serial.print("TIME : ");
  Serial.print(timeend);
  Serial.print(" microseconds.\n"); 
}


void loop() {
  if (current!=nextIn)
  {     
    Serial.write("instructions left : ");
    Serial.print(stringCount);
    Serial.write('\t');
    
    switch (byteArray[current][0])
    {
    case 48:
      Serial.write("You sent a char '0'.\t");
      break;
    case 49:
      Serial.write("You sent a char '1'.\t");
      break;
    case 50:
      Serial.write("You sent a char '2'.\t");
      break;
    case 51:
      Serial.write("You sent a char '3'.\t");
      break;
    case 52:
      Serial.write("You sent a char '4'.\t");
      break;
    case 53:
      Serial.write("You sent a char '5'.\t");
      break;
    case 54:
      Serial.write("You sent a char '6'.\t");
      break;
    case 55:
      Serial.write("You sent a char '7'.\t");
      break;
    case 56:
      Serial.write("You sent a char '8'.\t");
      break;
    case 57:
      Serial.write("You sent a char '9'.\t");
      break;
    default:
      Serial.write("You sent a non-cmd : ");
      Serial.write(byteArray[current][0]);
      Serial.write('\t');
      break; 
    }

    ///
    
    Serial.print ("Param1 : ");
    Serial.print (getParam1(byteArray[current]));
    Serial.print ("\tParam2 : ");
    Serial.print (getParam2(byteArray[current]));  
    
    ///
    
    //clear out the instruction code
    for (int j = 0; j < codelen; j++) byteArray[current][j] = 0;

    currentPlus();
    stringCount--;
    
    Serial.write(";\n");
  }
  delay(500);
}


//void serialReady()
//{
//  Serial.write(Serial.available());
//    Serial.write(stringCount);
//    Serial.write(';');
//}


void serialEvent()
{
  byte inByte;

  //keep adding instructions as long as there are bytes in serial available
  //and while there is at least one empty spot for an instruction in the buffer
  //this stringCount+1, because nextIn points to an empty spot in the buffer
  while (Serial.available() && ((stringCount+1) < bufflen)){

    inByte = Serial.read();

    if (inByte == ';') {
      nextInPlus();
      stringCount++;
      i = 0;
    }
    else {
      byteArray[nextIn][i++] = inByte;
    }
  }
}


//ENDIAN-NESS IS IMPORTANT
//These work, serial input "22323;"
//output is "12851;"
//which is 00110010 00110011
//corresponding to "50" "51"

unsigned long getParam1(byte bytesIn[])
{
    unsigned long retval = bytesIn[2];
    retval += bytesIn[1] << 8;
    return (retval);
}


//ENDIAN-NESS IS IMPORTANT
unsigned long getParam2(byte bytesIn[])
{
    unsigned long retval = bytesIn[4];
    retval += bytesIn[3] << 8;
    return (retval);
}
