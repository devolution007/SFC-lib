/*
 * binout.cpp
 *
 *  Created on: 2 апр. 2016
 *      Author: shurik
 */

#include <binout.h>

// BinOutClass

BinOutClass::BinOutClass(uint8_t unitNumber, uint8_t polarity)
{
	_unitNumber = unitNumber;
	state.setPolarity(polarity);
	state.onSet(std::bind(&BinOutClass::_setUnitState, this, std::placeholders::_1));
}


// BinOutGPIOClass
BinOutGPIOClass::BinOutGPIOClass(uint8_t unitNumber, uint8_t polarity)
:BinOutClass(unitNumber, polarity)
{
	pinMode(_unitNumber, OUTPUT);
	state.set(false);
}

void BinOutGPIOClass::setUnitNumber(uint8_t unitNumber)
{
	BinOutClass::setUnitNumber(unitNumber);
	pinMode(unitNumber, OUTPUT);
}

void BinOutGPIOClass::_setUnitState(uint8_t state)
{
	digitalWrite(_unitNumber, this->state.getRawState());
}

// BinOutMCP23S17Class
BinOutMCP23S17Class::BinOutMCP23S17Class(MCP &mcp, uint8_t unitNumber, uint8_t polarity)
:BinOutClass(unitNumber, polarity)
{
	_mcp = &mcp;
	state.set(false);
//	_mcp->pinMode(_unitNumber, OUTPUT);
}

void BinOutMCP23S17Class::setUnitNumber(uint8_t unitNumber)
{
	BinOutClass::setUnitNumber(unitNumber);
//	_mcp->pinMode(_unitNumber, OUTPUT);
}

void BinOutMCP23S17Class::_setUnitState(uint8_t state)
{
//	Serial.printf("SetUnit: %d to %s\n", _unitNumber, this->state.getRawState() ? "true" : "false");
	_mcp->digitalWrite(_unitNumber, this->state.getRawState());
}

// BinOutMCP23017Class
BinOutMCP23017Class::BinOutMCP23017Class(MCP23017 &mcp, uint8_t unitNumber, uint8_t polarity)
:BinOutClass(unitNumber, polarity)
{
	_mcp = &mcp;
	state.set(false);
//	_mcp->pinMode(_unitNumber, OUTPUT);
}

void BinOutMCP23017Class::setUnitNumber(uint8_t unitNumber)
{
	BinOutClass::setUnitNumber(unitNumber);
//	_mcp->pinMode(_unitNumber, OUTPUT);
}

void BinOutMCP23017Class::_setUnitState(uint8_t state)
{
//	Serial.printf("SetUnit: %d to %s\n", _unitNumber, this->state.getRawState() ? "true" : "false");
	_mcp->digitalWrite(_unitNumber, this->state.getRawState());
}

