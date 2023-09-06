/*
 * binout.h
 *
 *  Created on: 2 апр. 2016
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <binstate.h>
#include <Libraries/MCP23S17/MCP23S17.h>
#include <Libraries/MCP23017/MCP23017.h>

class BinOutClass
{
public:
	BinOutClass(uint8_t unitNumber, uint8_t polarity);
	virtual ~BinOutClass() {};
	void setUnitNumber(uint8_t unitNumber) { _unitNumber = unitNumber; };
	BinStateClass state;
protected:
	uint8_t _unitNumber = 0;
	virtual void _setUnitState(uint8_t state) = 0;
};

class BinOutGPIOClass : public BinOutClass
{
public:
	BinOutGPIOClass(uint8_t unitNumber, uint8_t polarity);
	virtual ~BinOutGPIOClass() {};
	void setUnitNumber(uint8_t unitNumber);
protected:
	virtual void _setUnitState(uint8_t state);
};

class BinOutMCP23S17Class : public BinOutClass
{
public:
	BinOutMCP23S17Class(MCP &mcp, uint8_t unitNumber, uint8_t polarity);
	virtual ~BinOutMCP23S17Class() {};
	void setUnitNumber(uint8_t unitNumber);
protected:
	virtual void _setUnitState(uint8_t state);
	MCP *_mcp;
};

class BinOutMCP23017Class : public BinOutClass
{
public:
	BinOutMCP23017Class(MCP23017 &mcp, uint8_t unitNumber, uint8_t polarity);
	virtual ~BinOutMCP23017Class() {};
	void setUnitNumber(uint8_t unitNumber);
protected:
	virtual void _setUnitState(uint8_t state);
	MCP23017 *_mcp;
};

