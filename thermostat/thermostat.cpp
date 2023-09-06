/*
 * thermostat.cpp
 *
 *  Created on: 2 апр. 2016
 *      Author: shurik
 */

#include <thermostat.h>

//  ThermostatClass

ThermostatClass::ThermostatClass(TempSensors &tempSensors, uint8_t sensorId, uint8_t mode, uint8_t invalidDefaultState, uint8_t disabledDefaultState, String name, uint16_t refresh)
: _tempSensors(&tempSensors), _sensorId(sensorId), _mode(mode), _invalidDefaultState(invalidDefaultState), _disabledDefaultState(disabledDefaultState), _name(name), _refresh(refresh), _enabled(false)
{
	state.set(disabledDefaultState,true);

}

void ThermostatClass::start()
{
	_refreshTimer.initializeMs(_refresh, [=](){this->_check();}).start(true);

	Serial.printf(_F("Start - set init state via delegate\n"));
	state.set(state.get(),true);
//	_callOnStateChangeDelegates();
//	setState(_state, true);

}

void ThermostatClass::stop(uint8_t setDefaultDisabledState)
{
	_refreshTimer.stop();
	if (setDefaultDisabledState)
	{
//		_state = _disabledDefaultState;
		state.set(_disabledDefaultState, true);
		Serial.printf(_F("Stop - set default disabled state via delegate\n"));
//		_callOnStateChangeDelegates();
	}
}

void ThermostatClass::enable(uint8_t enabled)
{
	if (enabled)
	{
		start();
	}
	else
	{
		stop();
	}
}

//void ThermostatClass::onStateChange(onStateChangeDelegate delegateFunction, uint8_t directState)
//{
//	if (directState)
//	{
//		_onChangeState.add(delegateFunction);
//	}
//	else
//	{
//		_onChangeStateInverse.add(delegateFunction);
//	}
//
//}
//
//void ThermostatClass::_callOnStateChangeDelegates()
//{
//	for (uint8_t i = 0; i < _onChangeState.count(); i++)
//	{
//		_onChangeState[i](_state);
//	}
//
//	for (uint8_t i = 0; i < _onChangeStateInverse.count(); i++)
//	{
//		_onChangeStateInverse[i](!_state);
//	}
//}
//void ThermostatClass::onStateChangeInverse(onStateChangeDelegate delegateFunction)
//{
//	_onChangeStateInverse.add(delegateFunction);
//}

//void ThermostatClass::setState(uint8_t state, uint8_t forceDelegatesCall)
//{
//	uint8_t prevState = _state;
//	_state = state;
//	Serial.printf("Thermostat %s: %s\n", _name.c_str(), _state ? "true" : "false");
//	if (_state != prevState || forceDelegatesCall)
//	{
//		_callOnStateChangeDelegates();
//	}
//}

void ThermostatClass::_check()
{
	float currTemp = _tempSensors->getTemp();

	if (_tempSensors->isValid())
	{
		_tempSensorValid = maxInvalidGetTemp;
	}
	else if (_tempSensorValid > 0)
	{
		_tempSensorValid--;
		Serial.printf(_F("Name: %s - TEMPSENSOR ERROR!, %d\n"), _name.c_str(), _tempSensorValid);
	}

	if (!_tempSensorValid)
	{
		Serial.printf(_F("We lost tempsensor! - set invalidDefaultstate via delegate!\n"));
		state.set(_invalidDefaultState);
//		_state = _invalidDefaultState; // If we lost tempsensor we set thermostat to Default Invalid State
//		if (_onChangeState)
//		{
//			Serial.printf("We lost tempsensor! - set invalidDefaultstate via delegate!\n");
//			_onChangeState(_state);
//		}
		Serial.printf(_F("Name: %s - TEMPSENSOR ERROR! - WE LOST IT!\n"), _name.c_str());
	}
	else
	{
		if (currTemp >= (float)(_targetTemp / 100.0) + (float)(_targetTempDelta / 100.0))
		{
			state.set(!(_mode));
		}
		if (currTemp <= (float)(_targetTemp / 100.0) - (float)(_targetTempDelta / 100.0))
		{
			state.set((_mode));
		}
	}
}

void ThermostatClass::onHttpConfig(HttpRequest &request, HttpResponse &response)
{
	if (request.method == HTTP_POST)
		{
			String body = request.getBody();
			if (body == NULL)
			{
				debug_d("NULL bodyBuf");
				return;
			}
			else
			{
				uint8_t needSave = false;
				DynamicJsonBuffer jsonBuffer;
				JsonObject& root = jsonBuffer.parseObject(body);
//				root.prettyPrintTo(Serial); //Uncomment it for debuging

				if (root["targetTemp"].success()) // Settings
				{
//					_targetTemp = ((float)(root["targetTemp"]) * 100);
					_targetTemp = root["targetTemp"];
					needSave = true;
				}
				if (root["targetTempDelta"].success()) // Settings
				{
//					_targetTempDelta = ((float)(root["targetTempDelta"]) * 100);
					_targetTempDelta = root["targetTempDelta"];
					needSave = true;
				}
				if (needSave)
				{
					_saveBinConfig();
				}
			}
		}
		else
		{
			JsonObjectStream* stream = new JsonObjectStream();
			JsonObject& json = stream->getRoot();

//			json["name"] = _name;
//			json["active"] = _active;
//			json["state"] = _state;
			json["targetTemp"] = _targetTemp;
			json["targetTempDelta"] = _targetTempDelta;

			response.setAllowCrossDomainOrigin("*");
			response.sendDataStream(stream, MIME_JSON);
		}
}

void ThermostatClass::_saveBinConfig()
{
	Serial.printf(_F("Try to save bin cfg..\n"));
	file_t file = fileOpen("tstat" + _name, eFO_CreateIfNotExist | eFO_WriteOnly);
	fileWrite(file, &_targetTemp, sizeof(_targetTemp));
	fileWrite(file, &_targetTempDelta, sizeof(_targetTempDelta));
	fileClose(file);
}

void ThermostatClass::_loadBinConfig()
{
	Serial.printf(_F("Try to load bin cfg..\n"));
	if (fileExist("tstat" + _name))
	{
		Serial.printf(_F("Will load bin cfg..\n"));
		file_t file = fileOpen("tstat" + _name, eFO_ReadOnly);
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, &_targetTemp, sizeof(_targetTemp));
		fileRead(file, &_targetTempDelta, sizeof(_targetTempDelta));
		fileClose(file);
	}
}
