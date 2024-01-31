#include "As5600.h"

//////////////////////Configuration Registers//////////////////////
#define ZMCO 0x00
#define ZPOS 0x01
#define MPOS 0x03
#define MANG 0x05
#define CONF 0x07

//////////////////////////Output Registers/////////////////////////
#define rowAngleLOW 0x0D
#define rowAngleHIGH 0x0C
#define Angle 0x0E

//////////////////////////Status Registers/////////////////////////
#define status 0x0B
#define AGC 0x1A
#define magnitude 0x1B

///////////////////////////Burn Registers//////////////////////////
#define burn 0xFF

As5600::As5600(TwoWire *wire)
{
    _wire = wire;
}

void As5600::magnetPresence()
{
    while ((int magnetStatus & 32) != 32) // while the magnet is not adjusted to the proper distance - 32: MD = 1
    {
        magnetStatus = 0; // reset reading

        Wire.beginTransmission(baseAddress); // connect to the sensor
        Wire.write(status);                  // status address
        Wire.endTransmission();
        Wire.requestFrom(baseAddress, 1); // request from the sensor

        while (Wire.available() == 0)
            ;
        magnetStatus = Wire.read();
        return magnetStatus;
    }
    // Status register output: 0 0 MD ML MH 0 0 0
    // MH: Too strong magnet - 100111 - DEC: 39
    // ML: Too weak magnet - 10111 - DEC: 23
    // MD: OK magnet - 110111 - DEC: 55
}

void As5600::rawAngle()
{
    Wire.beginTransmission(baseAddress); // connection starts at the base address
    Wire.write(rowAngleLOW);             // row angle address (7:0)
    Wire.endTransmission();              // end transmission
    Wire.requestFrom(baseAddress, 1);    // requesting one byte

    while (Wire.available() == 0)
        ;
    int lowbyte = Wire.read(); // Reading the data after the request

    Wire.beginTransmission(baseAddress);
    Wire.write(rowAngleHIGH); // row angle address (11:8)
    Wire.endTransmission();
    Wire.requestFrom(baseAddress, 1);

    while (Wire.available() == 0)
        ;
    int highbyte = Wire.read();

    // 4 bits have to be shifted to its proper place as we want to build a 12-bit number
    highbyte = highbyte << 8; // shifting to left
                              // Initial value: 00000000|00001111 (word = 16 bits or 2 bytes)
                              // Left shifting by eight bits: 00001111|00000000 so, the high byte is filled in

    // combine (bitwise OR) the two numbers:
    // High: 00001111|00000000
    // Low:  00000000|00001111
    int rawAngle = highbyte | lowbyte; // H|L:  00001111|00001111

    // 12 bit -> 4096 different levels: 360 is divided into 4096 equal parts:
    degAngle = rawAngle * 0.087890625; // 360/4096 = 0.087890625

    if (int correctedAngle < 0) // if the calculated angle is negative, we need to normalize it
    {
        correctedAngle = correctedAngle + 360;  // correction for negative numbers
        correctedAngle = degAngle - startAngle; // this tares the position
    }
    else
    {
        correctedAngle = rawAngle;
    }
    return correctedAngle;
}