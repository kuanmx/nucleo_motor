#include "debugMonitor.h"
#include "EncodedMotor.h"
#include <tuple>

debugMonitor::debugMonitor(PinName knobPin, EncodedMotor* motorPtr, PinName I2C1_SDA, PinName I2C1_SDL, uint16_t lcdAddr, TextLCD::LCDType lcdtype):
	i2c(I2C1_SDA, I2C1_SDL), lcd(&i2c, lcdAddr << 1, lcdtype),
	_knob(knobPin), _motorPtr(motorPtr)
{
	lcd.cls();
	lcd.setMode(TextLCD_Base::LCDMode::DispOn);
	lcd.setBacklight(TextLCD::LCDBacklight::LightOn);

	// LCD Initialize
	lcd.printf("Initialize... ");
	wait(1.0f);
	lcd.cls();
}

void debugMonitor::printSignal() {
	float signal = _knob.read();
	
	// analogWrite value
	lcd.locate(0, 0);
	lcd.printf("PWM - %f", signal);

	// equivalent PWM voltage
	lcd.locate(0, 1);
	const float Vs_max = 10.5f; 
	lcd.printf("Out Volt - %f", signal * Vs_max);

	// motor RPM
	lcd.locate(0, 2); 
	std::tuple<double, unsigned long long> speedData = _motorPtr->getSpeed();
	double speed = std::get<0>(speedData);
	lcd.printf("Motor RPM - %lf", speed); 
}
