#include "intelliSense.h"	// for IntelliSense
#include "mbed.h"
#include <vector>
#include "TextLCD.h"		// configure LCD at TextLCD_Config.h

// set baudrate at mbed_config.h default 115200
// I2C scanner included, derived from Arduino I2C scanner
// http://www.gammon.com.au/forum/?id=10896

#define I2C1_SDA  PB_9
#define I2C1_SCL  PB_8 

// Define serial
Serial serial(SERIAL_TX, SERIAL_RX);

// Host PC Communication
Serial pc(SERIAL_TX, SERIAL_RX);

// LCD I2C Communication
I2C i2c(I2C1_SDA, I2C1_SCL);

// Define interrupt
Ticker bridgeTicker; 

// Define LCD
uint16_t lcdAddr = 0X3F;   // LCD 2004 address determined as 0X3F in 16-bit
TextLCD_I2C lcd(&i2c, lcdAddr << 1, TextLCD::LCD20x4);

// Define L298N connection
PwmOut ena1(D5);		// Enable Pin 1

// Define user panel
AnalogIn knob(A0);		// Potentiometer for speed input
DigitalOut led2(LED2);	// On board User LED

// Define constants
float Vs_max = 10.5;		// max L298N output voltage (by experiment)
bool toPrintSignal = false; 


void I2C_scan()
{
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

void lcd_ini() {
	lcd.cls();
	lcd.setMode(TextLCD_Base::LCDMode::DispOn);
	lcd.setBacklight(TextLCD::LCDBacklight::LightOn);
	// LCD Initialize
	lcd.printf("try");
	wait(1.0f);
}

void flipPrintSignal() {
	toPrintSignal = !toPrintSignal; 
}

void printSignal() {
	float signal = knob.read();
	serial.printf("User Input PWM %f - %f V\n", signal, signal*Vs_max);

	// analogWrite value
	lcd.locate(0, 0);
	lcd.printf("PWM - %f", signal);

	// equivalent PWM voltage
	lcd.locate(0, 1);
	lcd.printf("Out Volt - %f", signal * Vs_max);

	flipPrintSignal();		// toogle signal flag
}

void motorpower(float analog_readPin, uint8_t enablePin) {
	enablePin = analog_readPin;
}

int main()
{
	// I2C Scanner.. comment out if not used... 
	//I2C_scan();
	
	// initiate LCD
	lcd_ini(); 
	lcd.printf("Ready"); 
	serial.printf("Ready"); 
	wait_ms(500); 									// Initialize ready
	lcd.cls();

	bridgeTicker.attach(&flipPrintSignal, 0.5);		// initiate Ticker

	while (1)
	{
		if (toPrintSignal) { printSignal(); }
		led2 = !led2; 
		motorpower(knob.read(), ena1); 
	}


}

