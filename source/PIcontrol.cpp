#include "..\intelliSense.h"	// for IntelliSense
#include "mbed.h"
#include "PIcontrol.h"
#include "EncodedMotor.h"

PIcontrol::PIcontrol(float Kp, float Ki, EncodedMotor* encodedMotor)
	: _Kp(Kp), _Ki(Ki), _encodedMotor(encodedMotor)
{
}

float PIcontrol::compensate_signal(float* error, float* timestep) 
{
	_thisError = *error; 
	_timestep = *timestep; 

	_compensateError += P_signal(); 
	_compensateError += I_signal();

	return _compensateError;
}

float PIcontrol::P_signal()
{
	return _Kp * _thisError; 
}

float PIcontrol::I_signal()
{
	_accError_t += _thisError * _timestep; 
	return _Ki * (_thisError - _accError_t); 
}
