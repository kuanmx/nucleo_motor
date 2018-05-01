#include "intelliSense.h"	// for IntelliSense
#include <mbed.h>
#include <vector>
#include <memory>
#include "source\EncodedMotor.h"	// motor encoder
#include "source\debugMonitor.h"	// enable LCD2004 as debug monitor
#include "source\TextLCD.h"
#include "source\MotorControl.h"
#include "source\EventVariable.h"
#include "source\ShiftReg7Seg.h"
#include "mbed_stats.h"

// set baudrate at mbed_config.h default 115200
// I2C scanner included, derived from Arduino I2C scanner
// http://www.gammon.com.au/forum/?id=10896

/////////////////////////////////
//// Declare connection//////////
/////////////////////////////////
// L298N Control Port
PinName MotorEnable = PA_15;		// connect ENA to PA_15
PinName MotorDirection1 = PA_14;	// connect IN1 to PA_14
PinName MotorDirection2 = PA_13;	// connect IN2 to PA_13
DigitalOut SolenoidEnable = PB_7;	// connect ENB to PB_7
// User Input Port
PinName WeldStartStop = PA_11; 		// connect WeldStartStop Btn to PA_11
PinName MotorStartStop = PC_13;		// connect MotorStartStop Btn to PA_12
PinName knob = PA_4;				// connect Potentionmeter to PA_4 (A2)

// LED Indicator Port
DigitalOut MotorLED = PB_15;		// connect MotorLED to PB_15
DigitalOut SolenoidOnLED = PB_14;	// connect SolenoidOnLED to PB_14
// Motor Encoder Port
PinName MotorEncoderA = PA_9;		// connect MotorEncoderA (blue) to PA_9 (D8)
PinName MotorEncoderB = PA_8;		// connect MotorEncoderB (purple) to PA_8 (D7)

//// Initiate object
EncodedMotor encoder(MotorEncoderA, MotorEncoderB, 1848*4, 10, EncodeType::X4);		// Encoded Motor object
std::unique_ptr<MotorControl> motor1 = std::make_unique<MotorControl>(MotorEnable, 
	MotorDirection1, MotorDirection2, &encoder, 0.16, 0.01);		// motor controller object, Kp and Ki specified
RawSerial pc(SERIAL_TX, SERIAL_RX, 250000);		// serial communication protocol
DebugMonitor debugger(knob, &encoder, &pc);		// update status through LCD2004 and Serial Monitor

//// Declare interrupt
Ticker statusUpdater;			// Periodic Interrupt for debugging purpose
Ticker motorBlinkLEDTicker;		// LED Blinker
InterruptIn motorBtn(MotorStartStop);		// Motor Button Interrupt
InterruptIn weldingBtn(WeldStartStop);		// Welding Button Interrupt

//// Declare thread
Thread motorLEDBlinking;		// Thread to perform LED Blinking
//// Define constants
volatile bool weldSignal = false;			// Start welding flag
// volatile bool motorStartBtnChange = false;		// Start motor flag
AnalogIn refSpeed(knob);					// Reference Speed from user through potentiometer
volatile bool prevMotorSteady = false;		// Store previous motor steady state
//// Fwd declare
void I2C_scan(); 
void statusUpdate(); 
void analyseUpdate(); 
void motorStartBtnChangeEvent(bool &);		// Determine motor start status
void motorRunner(); 
void MotorLEDBlinker(bool&); 

// Initiate EventVariable
EventVariable<bool> statusUpdateFlag(true, &statusUpdate);
//EventVariable<bool> statusUpdateFlag(true, &analyseUpdate);
EventVariable<bool> motorStartBtnChange(false, &motorStartBtnChangeEvent);
EventVariable<bool> motorSteadySignal(true, &MotorLEDBlinker);

mbed_stats_heap_t heap_stats;
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
void statusUpdate()
{
	// Output status
	debugger.printSignal();

	mbed_stats_heap_get(&heap_stats);
	printf("Current heap after: %lu\r\n", heap_stats.current_size);
	printf("Max heap size after: %lu\r\n", heap_stats.max_size);

	// Output Flags to Serial monitor
	pc.printf("motorStartBtnChange: %d\n motorSteadySignal: %d\n weldSignal: %d\n SolenoidEnable = %d\n",
		motorStartBtnChange.value, motorSteadySignal.value, (int)weldSignal, SolenoidEnable.read());
	pc.printf("Compensate: %f\n Speed: %f\n Error: %lf\n AdjError: %lf\n",
		motor1->readComp(), motor1->readSpeed(), motor1->readError(), motor1->readAdjError());
}
void analyseUpdate() { pc.printf("%f %f %f %f\n", refSpeed.read()*100, motor1->readSpeed(), motor1->readError(), motor1->readAdjError()); }
void motorStartBtnChangeEvent(bool &motorState) {
	// Active-Deactive Motor Rotation
	if (!motorState) {
		motorBlinkLEDTicker.detach(); 
		MotorLED = 0; 
	}
	else {
		motorSteadySignal = false;
	}
}
void motorRunner() 
{
	bool tempMotorSteady = motor1->run(refSpeed.read());			// run motor1 and read motor1 steady state
	if (tempMotorSteady != prevMotorSteady) motorSteadySignal = tempMotorSteady; 
	prevMotorSteady = tempMotorSteady; 
}
void motorStopper()
{
	motorSteadySignal = 0; 
	motor1->stop();
}
void MotorLEDBlinker(bool& motorSteady)			// Run motor and set motorOnLED to blinking / solid light
{
	if (motorSteady) { motorBlinkLEDTicker.detach(); MotorLED = 1; }
	else motorBlinkLEDTicker.attach([]() {MotorLED = !MotorLED; }, 0.5f);
}

SPI SPI7Seg(SPI_MOSI, SPI_MISO, SPI_SCK);
ShiftReg7Seg display7Seg(SPI7Seg, SPI_CS, &pc);

int main() {
	SPI7Seg.frequency(10000);
	//pc.printf("Initiating\n");
	//// I2C Scanner.. comment out if not used... 
	//// I2C_scan(); 

	//// Initiate Interrupt and Ticker
	//motorBtn.rise([]() {motorStartBtnChange = !motorStartBtnChange; });				// motorBtn OnChange
	//weldingBtn.rise([&]() {
	//	bool toSolenoid = motorStartBtnChange.value && motorSteadySignal.value && !weldSignal;
	//	toSolenoid ? weldSignal = true : weldSignal = false; });					// weldingBtn OnChange

	//statusUpdater.attach([]() {statusUpdateFlag = 0; }, 0.5f);						// periodic status update

	//pc.printf("Ready\n");

	//while (1) {
	//	// Active-Deactive Solenoid for start welding
	//	SolenoidEnable = (int)weldSignal;
	//	SolenoidOnLED = (int)weldSignal;

	//	if (motorStartBtnChange.value) { motorRunner(); }		// to keep feeding updated signal to run motor
	//	else { motorStopper();}		// to keep feeding updated signal to stop motor
	//}
	for(int i =0;i<2;i++){
		auto returnArr = display7Seg.display(1.111);
		pc.printf("----------------------\n");
		wait(2);
	}


}