/*
 * thermostat.h
 *
 *  Created on: 2 апр. 2016
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <tempsensors.h>
#include <binstate.h>

#ifndef ONSTATECHANGEDELEGATE_TYPE_DEFINED
#define ONSTATECHANGEDELEGATE
typedef std::function<void(uint8_t state)> onStateChangeDelegate;
#endif

namespace ThermostatMode
{
	const uint8_t HEATING=1;
	const uint8_t COOLING=0;
}

const uint8_t maxInvalidGetTemp = 10; // max unhealthy getTemp before we assume tempsensor lost

class ThermostatClass
{
public:
	ThermostatClass(TempSensors &tempSensors, uint8_t sensorId, uint8_t mode = ThermostatMode::HEATING, uint8_t invalidDefaultState = true, uint8_t disabledDefaulrState = false, String name = "Thermostat", uint16_t refresh = 4000);
	void start();
	void stop(uint8_t setDefaultDisabledState = true);
	void enable(uint8_t enabled);
	void disable(uint8_t disabled) { enable(!disabled); };
	float getTargetTemp() { return _targetTemp / 100; };
	void setTargetTemp(float targetTemp) { _targetTemp = (uint16_t)targetTemp * 100; };
	float getTargetTempDelta() { return _targetTempDelta / 100; };
	void setTargetTempDelta(float targetTempDelta) { _targetTempDelta = (uint16_t)targetTempDelta / 100; };
//	uint8_t get() { return _state.get(); };
//	void set(uint8_t state, uint8_t forceDelegatesCall = false) { _state.set(state, forceDelegatesCall); };
//	void onChange(onStateChangeDelegate delegateFunction, uint8_t directState = true) { _state.onChange(delegateFunction, directState); };
//	void onStateChangeInverse(onStateChangeDelegate delegateFunction);
	void onHttpConfig(HttpRequest &request, HttpResponse &response);
	void _saveBinConfig();
	void _loadBinConfig();
	BinStateClass state;
private:
	void _check();
//	void _callOnStateChangeDelegates();
//	void _saveBinConfig();
//	void _loadBinConfig();
	String _name; // some text description of thermostat
	uint8_t _enabled; //thermostat active (true), ON,  works, updates, changes its _state or turned OFF
//	uint8_t _state; // thermostat state on (true) or off (false)
	uint8_t _mode; // thermostat mode HEATING = true or COOLING = false
	uint16_t _targetTemp = 2500; //target temperature for manual mode MULTIPLE BY 100
	uint16_t _targetTempDelta = 50; //delta +- for both _targetTemp and manualTargetTemp MULTIPLE BY 100
	uint16_t _refresh; // thermostat update interval
	Timer _refreshTimer; // timer for thermostat update
	TempSensors *_tempSensors;
	uint8_t	_sensorId;
	Vector<onStateChangeDelegate> _onChangeState; // call them with _state as argument
	Vector<onStateChangeDelegate> _onChangeStateInverse; // call them with !_state as argument
	uint8_t _invalidDefaultState = true;
	uint8_t _disabledDefaultState = false;
	uint8_t _tempSensorValid = maxInvalidGetTemp; // if more than zero we STILL trust tempSensor temperature if less zero NOT trust

};
