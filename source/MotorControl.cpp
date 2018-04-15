#include "MotorControl.h"
#include "EncodedMotor.h"
#include "PIcontrol.h"
#include "..\rtos\Thread.h"
#include "..\rtos\EventFlags.h"

rtos::Thread thread; 
rtos::EventFlags toslowdown; 

MotorControl::MotorControl(PinName motorEnablePin, PinName motorDirectionPin1, PinName motorDirectionPin2, EncodedMotor * encodedMotor, float Kp, float Ki, uint16_t ratedRPM) :
	_motorEnablePin(motorEnablePin), _motorDirectionPin1(motorDirectionPin1), _motorDirectionPin2(motorDirectionPin2), _encodedMotor(encodedMotor), 
	_picontrol(new PIcontrol(Kp, Ki)), _ratedRPM(ratedRPM)
{
	// make sure enable pin is LOW at initialize
	stop(); 

	// default direction clockwise
	setDirection(0); 

	thread.start([=](){toslowdown.wait_all(1U); _motorEnable = _speedVolt - (double)(factor * 0.01); wait(0.1); });
}

MotorControl::~MotorControl() {
	delete _picontrol;
}

/* start motor
* start motor gradually by factor set in MotorControl.h
* default factor is 2V / s
* max volt per step at 1.2V (0.1f)
* start from 1V (PwmOut 0.1f)
*/
void MotorControl::run(double refVolt)
{
	_speedData = _encodedMotor->getSpeed();
	_speed = std::get<0>(_speedData);			// get speed in RPM
	_speedVolt = _speed *100 / 24;				// 24rpm rated speed
	_thisTime = std::get<1>(_speedData);		// get current timestep (us)
	
	/* Saturate error voltage to factor
	* speed up motor gradually by factor set in MotorControl.h
	* default factor is 2V / s (0.02f per 100ms, refresh rate)
	*/
	_errorVolt = refVolt * 100 - _speedVolt;
	if (_errorVolt > 5) _adjerrorVolt = 5; 
	else if (_errorVolt < -5) _adjerrorVolt = -5; 
	else if (_errorVolt < 0.4 && _errorVolt > -0.4) _adjerrorVolt = 0; 
	else _adjerrorVolt = _errorVolt; 

	// run only if latest data (data at new timestep) is provided
	if (_thisTime > _prevTime)
	{
		unsigned long timestep = _thisTime - _prevTime;	// unit us

		// PI Controller
		_compVolt += _picontrol->compensateSignal(_adjerrorVolt, timestep);

		_compVolt > 100 ? _compVolt = 100 : 0; 

		power(_compVolt/100);		// output to Motor
		_prevTime = _thisTime;		// update Timestep
	}
}

/* Stop motor
* slow down motor gradually by factor set in MotorControl.h
* default factor is 2V / s (0.02f per 100ms)
* cut off at 1V (analogRead 0.1f)
*/
void MotorControl::stop()
{
	_compVolt = 0; 
	power(_compVolt); 
//	_speedData = _encodedMotor->getSpeed();
//	_speed = std::get<0>(_speedData);			// get speed in RPM
//	_speedVolt = _speed / 24;					// 24rpm rated speed
//	if (_speedVolt > 0.1f)
//	{
//		// Gradual slow procedure
//		toslowdown.set(1U); 
//	}
//	else
//	{
//		_motorEnable = 0;
//	}
}

void MotorControl::setDirection(MotorControl::Direction direction = MotorControl::Direction::Clockwise)
{
	if (direction == MotorControl::Direction::Clockwise) { _motorDirection1 = 0; _motorDirection2 = 1; }
	else { _motorDirection1 = 1; _motorDirection2 = 0; }
}

void MotorControl::setDirection(bool direction)
{
	direction == 0 ? setDirection(MotorControl::Direction::Clockwise) : setDirection(MotorControl::Direction::C_Clockwise); 
}

float MotorControl::readComp() { return _compVolt; }

float MotorControl::readSpeed() { return _speedVolt;  }

double MotorControl::readError() { return _errorVolt;  }

double MotorControl::readAdjError() { return _adjerrorVolt; }

void MotorControl::power(float powerIn) { _motorEnable = powerIn; }
