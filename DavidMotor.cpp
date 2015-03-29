#include "DavidMotor.h"

DavidMotor::DavidMotor(int ppinPWM, int ppinInputA, int ppinInputB){
		pinPWM = ppinPWM;
		pinInputA = ppinInputA;
		pinInputB = ppinInputB;
		setPWM(0);
		setClockwise(true);
		currentPWM = 0;
}

DavidMotor::~DavidMotor(){
		setPWM(0);
}

//change it to STRING LATER
long DavidMotor::pinOut(){
	long temp = pinPWM * 10000L + pinInputA * 100L + pinInputB;
	return temp;
}

//unregulated PWM input
void DavidMotor::setPWM(int intPWM){
	analogWrite(pinPWM, intPWM);
}

//sets motor direction using booleans
void DavidMotor::setClockwise(bool cclockwise){
	clockwise = cclockwise;
	digitalWrite(pinInputA, clockwise);
	digitalWrite(pinInputB, !clockwise);
}

//returns the state of member variable
bool DavidMotor::isClockwise(){
	return clockwise;
}

//============================================

//regulated PWM input
//accepts a number between 0 to 255 for clockwise rotation
//also accepts number between (-255)to 0 for counterclockwise
//******we'll need faster more specialized versions of this******
void DavidMotor::motorGo(int intPWM){
	if ((intPWM < 256) && (intPWM >= 0)) {
		setClockwise(true);
		setPWM(intPWM);
	}
	else if ((intPWM > -256) && (intPWM <= 0)) {
		setClockwise(false);		
		setPWM(-intPWM);
	}
	else if (intPWM == 0){
		digitalWrite(pinInputA, HIGH);
		digitalWrite(pinInputB, HIGH);
		setPWM(0);
	}
}

//fewer checks, does it really help? not really
//void DavidMotor::motorGo2(int intPWM){
//	if (intPWM >= 0) {
//		setClockwise(true);
//		setPWM(intPWM);
//	}
//	else {
//		setClockwise(false);		
//		setPWM(-intPWM);
//	}
//}
