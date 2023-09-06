/*
 * FanClass.h
 *
 *  Created on: 3 апр. 2016
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <binin.h>
#include <binout.h>
#include <thermostat.h>
#include <tempsensors.h>

namespace FanMode
{
	const uint8_t IDLE=0;
	const uint8_t START=1;
	const uint8_t RUN=2;
	const uint8_t PERIODIC=3;
	const uint8_t STOP=4;
}

//const uint8_t _maxLowTempCount = 3;

class FanClass
{
public:
	FanClass(TempSensors &tempSensor, ThermostatClass &thermostat, BinOutClass &fanRelay);
	virtual ~FanClass() {};
	void start();
	void stop();
	uint8_t getMode() const { return _mode;	}
	void setMode(uint8_t mode) { _mode = mode; }
	uint16_t getStartDuration() const { return _startDuration; }
	void setStartDuration(uint16_t startDuration) { _startDuration = startDuration; }
	uint16_t getStopDuration() const { return _stopDuration; };
	void setStopDuration(uint16_t stopDuration) { _stopDuration = stopDuration; };
	uint16_t getPeriodicInterval() const { return _periodicInterval; };
	void setPeriodicInterval(uint16_t periodicInterval) { _periodicInterval = periodicInterval; };
	uint16_t getPeriodicDuration() const { return _periodicDuration; };
	void setPeriodicDuration(uint16_t periodicDuration) { _periodicDuration = periodicDuration; };
	uint16_t getPeriodicTempDelta() const { return _periodicTempDelta; };
	void setPeriodicTempDelta(uint16_t periodicTempDelta) { _periodicTempDelta = periodicTempDelta; };
	void onHttpConfig(HttpRequest &request, HttpResponse &response);
	void _saveBinConfig();
	void _loadBinConfig();
	BinStateClass state;
	BinStateClass active;
	void _modeStart(uint8_t state);
	void _modeStop(uint8_t state);
	void setThermostatControlState(uint8_t state);
	void periodicDisable(uint8_t enabled);
private:
	uint8_t _thermostatControlState = false;
	void _modeStartEnd();
	void _modeStopEnd();
	void _periodicDisable(uint8_t enabled);
	void _periodicStart();
	void _periodic();
	void _periodicEnd();
	void _checkerEnable(uint8_t enabled);
	void _checkerStart();
	void _checkerStop();
	void _checkerCheck();
	void _enable(uint8_t enableState);
	float _chekerMaxTemp = 0; //Temperature at checkerStart
	uint16_t _checkerInterval = 5; // Checker run interval in minutes
	Timer _checkerTimer;
	uint16_t _timerInterval = 0;
	Timer _fanTimer;
	TempSensors* _tempSensor;
	BinOutClass* _fanRelay;
	ThermostatClass* _thermostat;
	uint8_t _mode = FanMode::IDLE;
	uint16_t _startDuration = 1; // turn Fan on for this duration when START button pressed, Minutes
	uint16_t _stopDuration = 3; // turn Fan on for this duration when STOP button pressed, Minutes
	uint16_t _periodicInterval = 60; // interval to turn on Fan in RUN mode, Minutes
	uint16_t _periodicDuration = 5; // duration to turn on Fan in RUN mode, Minutes
//	float _periodicStartTemp = 0; // Temperature at periodic start, will be compared to temperature at periodic and
	int16_t _periodicTempDelta = -1000; // minimum temperature difference after periodic fan turn on, if less go to IDLE mode
	uint8_t _maxLowTempCount = 3;
};
