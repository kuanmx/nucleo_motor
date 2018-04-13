#include "intelliSense.h"	// for IntelliSense
#include <mbed.h>
#include <vector>
#include "source\EncodedMotor.h"	// motor encoder
#include "source\debugMonitor.h"	// enable LCD2004 as debug monitor
#include "source\TextLCD.h"
#include "mbed-os\rtos\Thread.h"

// set baudrate at mbed_config.h default 115200
// I2C scanner included, derived from Arduino I2C scanner
// http://www.gammon.com.au/forum/?id=10896

/////////////////////////////////
//// Declare connection//////////
/////////////////////////////////
// L298N Control Port
PwmOut MotorEnable = PA_15;			// connect ENA to PA_13
DigitalOut MotorDirection1 = PA_14; // connect IN1 to PA_14
DigitalOut MotorDirection2 = PA_13; // connect IN2 to PA_15
DigitalOut SolenoidEnable = PB_7;	// connect ENB to PB_7
// User Input Port
PinName WeldStartStop = PA_11; 		// connect WeldStartStop Btn to PA_11
PinName MotorStartStop = PA_12;		// connect MotorStartStop Btn to PA_12
PinName knob = PA_4;				// connect Potentionmeter to PA_4 (A2)

// LED Indicator Port
DigitalOut MotorOnLED = PC_13;		// connect MotorOnLED to PC_13
DigitalOut SolenoidOnLED = PC_14;	// connect MotorOnLED to PC_14
// Motor Encoder Port
PinName MotorEncoderA = PA_9;		// connect MotorEncoderA (blue) to PA_9 (D8)
PinName MotorEncoderB = PA_8;		// connect MotorEncoderB (purple) to PA_8 (D7)

//// Initiate object
EncodedMotor motor1(MotorEncoderA, MotorEncoderB, 1848);
debugMonitor lcd(knob, &motor1);
RawSerial pc(SERIAL_TX, SERIAL_RX); 
rtos::Thread thread; 

//// Declare interrupt
Ticker bridgeTicker;							// Periodic Interrupt for debugging purpose
Ticker LEDblinking;								// Periodic Interrupt for LED Blinking
InterruptIn motorInterrupt(MotorStartStop);		// Motor Button Interrupt
InterruptIn weldingInterrupt(WeldStartStop);	// Welding Button Interrupt

//// Define constants
volatile bool toPrintSignal = false;		// LCD signal flag
volatile bool weldSignal = false;			// Start welding flag
volatile bool motorStartSignal = false;		// Start motor flag
volatile bool motorSteadySignal = false;	// Motor in steady state flag
AnalogIn refSpeed(knob);		// Reference Speed from user through potentiometer

//// Define function
void I2C_scan()
{
	// Define I2C communication using port I2C1_SDA(D14, PB_9) and I2C1_SCL(D15, PB_8)
	I2C i2c(PB_9, PB_8); 
	// Define Serial Communication to Serial Monitor
	Serial pc(SERIAL_TX, SERIAL_RX);

	// Initializing I2C Scanner
	pc.printf("I2C Scanner initializing.... \n");
	uint8_t error, address;
	char mydata[2];
	mydata[1] = 0x00;
	std::vector<uint8_t> addrList;

	pc.printf("I2C Scanner start scanning. \n");

	// Start scanning using 8 bit address
	pc.printf("\n8-bit address.... \n");
	for (address = 1; address < 128; address++)
	{
		error = i2c.write(address, mydata, sizeof(mydata));					// error: 0 - success (ack), 1 - failure (nack)
		pc.printf("Address: %#X return %d\n", address, error);
		if (error == 0)
			addrList.push_back(address);	// add to addrList if address receive ack
	}
	for (auto &val : addrList)
	{
		pc.printf("Valid Address: %#X ", val); 		// print valid address
	}

	// Initialize and start scanning using 16-bit (8-bit shifted left) address
	addrList.clear();
	pc.printf("\n16-bit address.... \n");
	for (address = 1; address < 128; address++)
	{
		error = i2c.write((uint16_t)(address << 1), mydata, sizeof(mydata));  // error: 0 - success (ack), 1 - failure (nack)
		pc.printf("Address: %#X return %d\n", (uint16_t)(address << 1), error);
		if (error == 0)
			addrList.push_back(address);	// add to addrList if address receive ack
	}
	for (auto &val : addrList)
	{
		pc.printf("Valid Address: %#X ", val);		// print valid address
	}
}
void flipMotorSteadySignal() { motorStartSignal ? motorSteadySignal = !motorSteadySignal : 0 ; }
void motorpower(float _knobVal, PwmOut _MotorEnable) {
	// PWM of MotorEnable pin is equal to AnalogIn of potentiometer
	_MotorEnable = _knobVal;
}

void something() {

}
int main() {
	// I2C Scanner.. comment out if not used... 
	// I2C_scan(); 
	// Initiate Interrupt
	bridgeTicker.attach([]() {toPrintSignal = !toPrintSignal; }, 0.5f);					// Ticker for periodic interrupt
	motorInterrupt.rise([]() {motorStartSignal = !motorStartSignal; });					// Motor Button Intterupter
	weldingInterrupt.rise([&]() {
		bool toSolenoid = motorStartSignal && motorSteadySignal && !weldSignal;  
		toSolenoid ? weldSignal = true : weldSignal = false; });						// Welding Button Interrupter
	pc.printf("Ready\n"); 

	//// for debug purpose.... remove in actual case
	motorSteadySignal = 0; 
	while (1) {
		// Print Status Signal Interrupt
		// if (toPrintSignal) { lcd.printSignal(); []() {toPrintSignal = !toPrintSignal; }					// Output status to LCD2004
		if (toPrintSignal) { 
			pc.printf("refSpeed: %f\n motorStartSignal: %d\n motorSteadySignal: %d\n weldSignal: %d\n SolenoidEnable = %d\n", refSpeed.read(), motorStartSignal, motorSteadySignal, weldSignal, SolenoidEnable.read());  
			toPrintSignal = !toPrintSignal; 
		}	// Output status to Serial monitor

		// Active-Deactive Motor Rotation
		if (!motorStartSignal)
		{
			MotorEnable = 0.0f; 
			MotorOnLED = 0; 
		}
		else
		{
			motorpower(refSpeed.read(), MotorEnable);	// Adjust motor signal... to be replace by PID
			
			// MotorSteadyLED display routine
			if (motorSteadySignal) {
				LEDblinking.detach();   MotorOnLED.write(1); 
			} /*
			else {
				pc.printf("test1"); 
				LEDblinking.
				LEDblinking.attach([]() {MotorOnLED = !MotorOnLED; }, 0.52);
			}*/
		}
			
		// Active-Deactive Solenoid for start welding
		SolenoidEnable = (int)weldSignal; 
		SolenoidOnLED = (int)weldSignal; 
	}
}