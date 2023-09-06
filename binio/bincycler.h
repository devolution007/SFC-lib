/*
 * bincycler.h
 *
 *  Created on: 15 июля 2016 г.
 *      Author: shurik
 */
#pragma once
#include <SmingCore.h>
#include <binstate.h>

class BinCyclerClass {
public:
	BinCyclerClass(BinStateClass& cycleState, uint16_t duration, uint16_t interval);
	void setDuration(uint16_t duration) { _duration = duration; };
	uint16_t getDuration() { return _duration; };
	void setInterval(uint16_t interval) { _interval = interval; };
	uint16_t getinterval() { return _interval; };
	BinStateClass state;
private:
	void _enable(uint8_t enableState);
	void _setTrue();
	void _setFalse();
	BinStateClass& _cycleState;
	uint16_t	_duration; //in seconds
	uint16_t	_interval; // in seconds
	Timer	_timer;
};
