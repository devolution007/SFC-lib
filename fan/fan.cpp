/*
 * FanClass.cpp
 *
 *  Created on: 3 апр. 2016
 *      Author: shurik
 */

#include <fan.h>

// FanClass

FanClass::FanClass(TempSensors &tempSensor, ThermostatClass &thermostat, BinOutClass &fanRelay)
{
	_tempSensor = &tempSensor;
	_thermostat = &thermostat;
	_fanRelay = &fanRelay;
//	_startButton->state.onChange(onStateChangeDelegate(&FanClass::_modeStart, this));
//	_stopButton->state.onChange(onStateChangeDelegate(&FanClass::_modeStop, this));

	state.onChange(std::bind(&FanClass::_enable, this, std::placeholders::_1));
	active.set(false);
//	_thermostatControlState.onChange(onStateChangeDelegate(&BinStateClass::set,&_thermostat->state));
//	_fanRelay->setState(false); //No need, disabling thermostat with default stop will turn off fan
	_thermostat->state.onChange(std::bind(static_cast<void (BinStateClass::*) (uint8_t)>(&BinStateClass::set), &_fanRelay->state, std::placeholders::_1));
	_thermostat->state.onChange(std::bind(&FanClass::_checkerEnable, this, std::placeholders::_1));
	_thermostat->stop();
};

void FanClass::_enable(uint8_t enableState)
{
	if (enableState)
	{
		_modeStart(enableState);
	}
	else
	{
		_modeStop(!enableState);
	}
}

void FanClass::_modeStart(uint8_t state)
{
	if (state)
	{
		_mode = FanMode::START;
		active.set(true);
		Serial.printf(_F("START Button pressed\n"));
		_thermostat->stop(false);
		_fanRelay->state.set(true);
		//TODO: CHANGE THIS LATER FOR 60000!!!
		_fanTimer.initializeMs(_startDuration * 60000, [=](){this->_modeStartEnd();}).start(false);
	}
}

void FanClass::_modeStartEnd()
{
	Serial.printf(_F("START Finished\n"));
	_fanRelay->state.set(false);
//	use weekThermostat as source to enable-disable fan thermostat
//	_thermostat->start();
	_thermostat->enable(_thermostatControlState);
//	_fanTimer.initializeMs(_periodicInterval * 60000, TimerDelegate(&FanClass::_pereodic, this)).start(false);
	_mode = FanMode::RUN;
	periodicDisable(_thermostatControlState);
}

void FanClass::periodicDisable(uint8_t disabled)
{
	if ( _mode == FanMode::RUN ||  _mode == FanMode::PERIODIC )
	{
		if ( disabled && _fanTimer.isStarted() )
		{
			_periodicEnd();
			_fanTimer.stop();
			_thermostat->enable(_thermostatControlState); //disabled by weekthermostat
		}
		if ( !disabled )
		{
			_periodicStart();
		}
	}
}

void FanClass::_periodicStart()
{
	_fanTimer.initializeMs(_periodicInterval * 60000, [=](){this->_periodic();}).start(false);
}

void FanClass::_periodic()
{
	Serial.printf(_F("PREIODIC START\n"));
//	_thermostat->stop(false); //disabled by weekthermostat
	_fanRelay->state.set(true);
	_fanTimer.initializeMs(_periodicDuration * 60000, [=](){this->_periodicEnd();}).start(false);
	_mode = FanMode::PERIODIC;
}

void FanClass::_periodicEnd()
{
	Serial.printf(_F("PERIODIC END - GO TO RUN MODE\n"));
	_fanRelay->state.set(false);
//	use weekThermostat as source to enable-disable fan thermostat
//	_thermostat->start();
	_fanTimer.initializeMs(_periodicInterval * 60000, [=](){this->_periodic();}).start(false);
	_mode = FanMode::RUN;
}

void FanClass::_modeStop(uint8_t state)
{
	if (state)
	{
		_mode = FanMode::STOP;
		active.set(true);
		Serial.printf(_F("STOP Button pressed\n"));
		_thermostat->stop(false);
		periodicDisable(true);
		_fanRelay->state.set(true);
		_fanTimer.initializeMs(_stopDuration * 60000, [=](){this->_modeStopEnd();}).start(false);

	}
}

void FanClass::_modeStopEnd()
{
	Serial.printf(_F("STOP Finished\n"));
//	_fanRelay->setState(false); //No need, disabling thermostat with default stop will turn off fan
	_fanRelay->state.set(false);
	_fanTimer.stop();
	_mode = FanMode::IDLE;
	active.set(false);
}

void FanClass::onHttpConfig(HttpRequest &request, HttpResponse &response)
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

				if (root["startDuration"].success())
				{
					_startDuration = root["startDuration"];
					needSave = true;
				}
				if (root["stopDuration"].success())
				{
					_stopDuration = root["stopDuration"];
					needSave = true;
				}
				if (root["periodicInterval"].success())
				{
					_periodicInterval = root["periodicInterval"];
					needSave = true;
				}
				if (root["periodicDuration"].success())
				{
					_periodicDuration = root["periodicDuration"];
					needSave = true;
				}
				if (root["periodicTempDelta"].success())
				{
					_periodicTempDelta = root["periodicTempDelta"];
					needSave = true;
				}
				if (root["checkerInterval"].success())
				{
					_checkerInterval = root["checkerInterval"];
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

			json["startDuration"] = _startDuration;
			json["stopDuration"] = _stopDuration;
			json["periodicInterval"] = _periodicInterval;
			json["periodicDuration"] = _periodicDuration;
			json["periodicTempDelta"] = _periodicTempDelta;
			json["checkerInterval"] = _checkerInterval;

			response.setAllowCrossDomainOrigin("*");
			response.sendDataStream(stream, MIME_JSON);
		}
}

void FanClass::_saveBinConfig()
{
	Serial.printf(_F("Try to save bin cfg..\n"));
	file_t file = fileOpen("fan.config", eFO_CreateIfNotExist | eFO_WriteOnly);
	fileWrite(file, &_startDuration, sizeof(_startDuration));
	fileWrite(file, &_stopDuration, sizeof(_stopDuration));
	fileWrite(file, &_periodicInterval, sizeof(_periodicInterval));
	fileWrite(file, &_periodicDuration, sizeof(_periodicDuration));
	fileWrite(file, &_periodicTempDelta, sizeof(_periodicTempDelta));
	fileWrite(file, &_checkerInterval, sizeof(_checkerInterval));
	fileClose(file);
}

void FanClass::_loadBinConfig()
{
	Serial.printf(_F("Try to load bin cfg..\n"));
	if (fileExist("fan.config"))
	{
		Serial.printf(_F("Will load bin cfg..\n"));
		file_t file = fileOpen("fan.config", eFO_ReadOnly);
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, &_startDuration, sizeof(_startDuration));
		fileRead(file, &_stopDuration, sizeof(_stopDuration));
		fileRead(file, &_periodicInterval, sizeof(_periodicInterval));
		fileRead(file, &_periodicDuration, sizeof(_periodicDuration));
		fileRead(file, &_periodicTempDelta, sizeof(_periodicTempDelta));
		fileRead(file, &_checkerInterval, sizeof(_checkerInterval));
		fileClose(file);
	}
}

void FanClass::_checkerEnable(uint8_t enabled)
{
	if (enabled)
	{
		_checkerStart();
	}
	else
	{
		_checkerStop();
	}
}

void FanClass::_checkerStart()
{
	Serial.printf(_F("Checker STARTED!\n"));
	_chekerMaxTemp = _tempSensor->getTemp();
	_checkerTimer.initializeMs(_checkerInterval * 60000, [=](){this->_checkerCheck();}).start(true);
}

void FanClass::_checkerStop()
{
	Serial.printf(_F("Checker STOPPED!\n"));
	_checkerTimer.stop();
}

void FanClass::_checkerCheck()
{
	float _checkerCheckTemp = _tempSensor->getTemp();
	if (_checkerCheckTemp > _chekerMaxTemp)
	{
		_chekerMaxTemp = _checkerCheckTemp;
	}
	Serial.printf(_F("Checker CHECK- startTemp: "));Serial.print(_chekerMaxTemp);Serial.printf(_F(" endTemp "));Serial.print(_checkerCheckTemp);
	Serial.printf(_F(" _periodicTempDelta: "));Serial.println((float)(_periodicTempDelta / 100.0));
	if (_checkerCheckTemp - _chekerMaxTemp <= (float)(_periodicTempDelta / 100.0))
	{
		Serial.printf(_F("No wood left! Go to IDLE!\n"));
		_thermostat->stop();
//		_fanRelay->setState(false);
		_fanTimer.stop();
		_mode = FanMode::IDLE;
		active.set(false);
	}
	else
	{
		Serial.printf(_F("We still have wood! WORKING!\n"));
	}
}

void FanClass::setThermostatControlState(uint8_t state)
{
	_thermostatControlState = state;
	if (_mode == FanMode::RUN)
	{
		_thermostat->enable(_thermostatControlState);
	}
}
