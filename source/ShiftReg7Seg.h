//
// Created by poh14 on 3/29/2018.
//

#ifndef MBEDCLI2_SHIFTREG7SEG_H
#define MBEDCLI2_SHIFTREG7SEG_H

#include "..\intelliSense.h"
#include <mbed.h>
#include <vector>

namespace{
    //bytes are in order of: a, b, c, d, e, f, g, DP
    const uint8_t characterMap[] = {0b11111100, //0
                                    0b01100000, //1
                                    0b11011010, //2
                                    0b11110010, //3
                                    0b01100110, //4
                                    0b10110110, //5
                                    0b10111110, //6
                                    0b11100000, //7
                                    0b11111110, //8
                                    0b11110110, //9
                                    0b10011110, //Error
    };
}

class ShiftReg7Seg {
public:
    ShiftReg7Seg() = delete;
    ShiftReg7Seg(SPI& spiObj, PinName latchPin, unsigned int numberOfDisplay = 4, PinName MRPin = NC);

    //public methods
    std::vector<int> display(double value);
    void clearAll();

    //getters & setters
    unsigned int getNumberOfDisplay() const;
    void setNumberOfDisplay(unsigned int _numberOfDisplay);
    int getNumberOfDP() const;
    void setNumberOfDP(int _numberOfDP);

private:
    std::vector<int> displayDigits(std::vector<uint8_t>& digitArray);

    SPI& _spi;
    DigitalOut _latchPin;
    DigitalOut _MRPin;
    unsigned int _numberOfDisplay = 4;
    int _numberOfDP = 0;


};

#endif //MBEDCLI2_SHIFTREG7SEG_H
