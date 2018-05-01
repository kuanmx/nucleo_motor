//
// Created by poh14 on 3/29/2018.
//

#include "ShiftReg7Seg.h"

ShiftReg7Seg::ShiftReg7Seg(SPI& spiObj, PinName latchPin, unsigned int numberOfDisplay, PinName MRPin)
        :_spi(spiObj), _latchPin(latchPin), _MRPin(MRPin), _numberOfDisplay(numberOfDisplay)
{
    _numberOfDP = _numberOfDisplay -1;
    _MRPin = 1;
}
ShiftReg7Seg::ShiftReg7Seg(SPI& spiObj, PinName latchPin, RawSerial* pcSerial)
: _spi(spiObj), _latchPin(latchPin), _pcSerial(pcSerial), _MRPin(NC), _numberOfDisplay(4)
{
	_numberOfDP = _numberOfDisplay - 1;
	_MRPin = 1;
}

unsigned int ShiftReg7Seg::getNumberOfDisplay() const
{
    return _numberOfDisplay;
}
void ShiftReg7Seg::setNumberOfDisplay(unsigned int _numberOfDisplay)
{
    ShiftReg7Seg::_numberOfDisplay = _numberOfDisplay;
}
int ShiftReg7Seg::getNumberOfDP() const
{
    return _numberOfDP;
}
void ShiftReg7Seg::setNumberOfDP(int _numberOfDP)
{
    ShiftReg7Seg::_numberOfDP = _numberOfDP;
}
std::vector<int> ShiftReg7Seg::display(double value)
{
    double logVal = log10(value);
    logVal>=0 ? logVal : logVal = 0;
    auto numberOfDigits = static_cast<unsigned> (floor(logVal)+1);
    if (numberOfDigits>_numberOfDisplay) {
        std::vector<uint8_t> errorArray(_numberOfDisplay, characterMap[10]);
        return displayDigits(errorArray);  // fix by Kuan 13/4 regarding warning control reaches end of non-void function
    }
    else {
        std::vector<uint8_t> numArray(_numberOfDisplay);
        for (size_t i = 0; i<_numberOfDisplay; i++) {
            int power = i+1-numberOfDigits;
            double scaler = pow(10, power);
            double scaledVal = value*scaler;
            auto flooredScaledVal = static_cast<unsigned>(floor(scaledVal));
            unsigned digit = flooredScaledVal%10;
            numArray[i] = characterMap[digit];
        }
        numArray[numberOfDigits-1] = static_cast<uint8_t>(numArray[numberOfDigits-1] | 0b00000001);

        return displayDigits(numArray);
    }
}

std::vector<int> ShiftReg7Seg::displayDigits(std::vector<uint8_t>& digitArray)
{
    std::vector<int> returnArray;
    returnArray.reserve(digitArray.size());
	for (auto &val: digitArray) {
        _latchPin = 0;
		auto tempReturnVal = _spi.write(val);
		if (_pcSerial != nullptr) {

			_pcSerial->printf("Send: ");
			for (int i = 0; i < 8; i++) {
				auto temp = 0b10000000 >> i;
				_pcSerial->printf("%u", (val&temp) ? 1 : 0);
			}
			_pcSerial->printf("\n");

			_pcSerial->printf("Receive: ");
			for (int i = 0; i < 8; i++) {
				uint8_t temp = 0b10000000 >> i;
				_pcSerial->printf("%u", (tempReturnVal&temp) ? 1 : 0);
			}
			_pcSerial->printf("\n");
		}
		_latchPin = 1;
        returnArray.push_back(tempReturnVal);
		wait(1);
    }
    return returnArray;

}
void ShiftReg7Seg::clearAll()
{
    _MRPin = 0;
    _MRPin = 1;
}
