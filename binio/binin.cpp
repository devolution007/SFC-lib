/*
 * binaryinput.cpp
 *
 *  Created on: 1 апр. 2016 г.
 *      Author: shurik
 */

#include <binin.h>

// BinInClass

BinInClass::BinInClass(uint8_t unitNumber, uint8_t polarity)
: _unitNumber{unitNumber}, state{polarity}
{
//	state.setPolarity(polarity);
	state.set(false);
}

void BinInClass::_readState()
{
	uint8_t readUnit = _readUnit() ? state.getPolarity() : !(state.getPolarity());
//	Serial.printf("readUnit: %d, get: %d\n", readUnit, (uint8_t)state.get());
	if ( readUnit != (uint8_t)state.get())
	{
		state.set(readUnit);
	}
}

//BinInGPIOClass
BinInGPIOClass::BinInGPIOClass(uint8_t unitNumber, uint8_t polarity)
:BinInClass(unitNumber, polarity)
{
	pinMode(_unitNumber, INPUT);
	_readState();
}

void BinInGPIOClass::setUnitNumber(uint8_t unitNumber)
{
	BinInClass::setUnitNumber(unitNumber);
	pinMode(_unitNumber, INPUT);
}

uint8_t BinInGPIOClass::_readUnit()
{
	return digitalRead(_unitNumber);
}

//BinInMCP23S17Class
BinInMCP23S17Class::BinInMCP23S17Class(MCP &mcp, uint8_t unitNumber, uint8_t polarity)
:BinInClass(unitNumber, polarity), _mcp{&mcp}
{
	_readState();
//	_mcp->pinMode(_unitNumber, INPUT);
}

void BinInMCP23S17Class::setUnitNumber(uint8_t unitNumber)
{
	BinInClass::setUnitNumber(unitNumber);
//	pinMode(_unitNumber, INPUT);
}

uint8_t BinInMCP23S17Class::_readUnit()
{
	return _mcp->digitalRead(8 + _unitNumber);
}

//BinInMCP23017Class
BinInMCP23017Class::BinInMCP23017Class(MCP23017 &mcp, uint8_t unitNumber, uint8_t polarity)
:BinInClass(unitNumber, polarity), _mcp{&mcp}
{
	_readState();
//	_mcp->pinMode(_unitNumber, INPUT);
}

void BinInMCP23017Class::setUnitNumber(uint8_t unitNumber)
{
	BinInClass::setUnitNumber(unitNumber);
//	pinMode(_unitNumber, INPUT);
}

uint8_t BinInMCP23017Class::_readUnit()
{
	return _mcp->digitalRead(8 + _unitNumber);
}

// BinInPollerClass

void BinInPollerClass::start()
{
	_refreshTimer.initializeMs(_refresh, [this](){this->_pollState();}).start(true);
}

void BinInPollerClass::stop()
{
	_refreshTimer.stop();
}

void BinInPollerClass::add(BinInClass * binIn)
{
	_binIn.add(binIn);
}

void BinInPollerClass::_pollState()
{
	for (uint8_t id=0; id < _binIn.count(); id++)
	{
		_binIn[id]->_readState();
	}
}
