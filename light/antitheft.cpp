/*
 * AntiTheft.cpp
 *
 *  Created on: 21 дек. 2016 г.
 *      Author: shurik
 */

#include <antitheft.h>

AntiTheftClass::AntiTheftClass(BinOutClass** outputs, uint8_t persistentId)
: _outputs(outputs), _persistentId(persistentId)
{
	randomSeed(os_random());
	_loadBinConfig();
	state.onChange([this](uint8_t state){this->_enable(state);});
}

void AntiTheftClass::_turnOn()
{
	_currentId = random(0, _outputsIds.count());
	_outputs[_outputsIds[_currentId]]->state.set(true);
	uint8_t onTime = random(_minOn, _maxOn);
	Serial.printf("AntiTheft ON, %d: for %d seconds\n", _currentId, onTime);
	_timer.initializeMs(onTime * 60000, TimerDelegate(&AntiTheftClass::_turnOff, this)).start(true);
}

void AntiTheftClass::_turnOff()
{
	_outputs[_outputsIds[_currentId]]->state.set(false);
	uint8_t offTime = random(_minOff, _maxOff);
	Serial.printf("AntiTheft OFF, %d: wait for %d seconds\n", _currentId, offTime);
	_timer.initializeMs(offTime * 60000, TimerDelegate(&AntiTheftClass::_turnOn, this)).start(true);
}

void AntiTheftClass::_enableAntiTheft(uint8_t enableState)
{
	if (enableState)
	{
		Serial.printf("AntiTheft ON!!!");
		_turnOn();
	}
	else
	{
		Serial.printf("AntiTheft OFF!!!");
		if (_timer.isStarted())
		{
			_outputs[_outputsIds[_currentId]]->state.set(false);
			_timer.stop();
		}
	}
}

void AntiTheftClass::_saveBinConfig()
{
	Serial.printf("Try to save bin cfg..\n");
	file_t file = fileOpen("antiTheft.cfg", eFO_CreateIfNotExist | eFO_WriteOnly);
	fileWrite(file, &_enableStartTime, sizeof(_enableStartTime));
	fileWrite(file, &_enableStopTime, sizeof(_enableStopTime));
	fileWrite(file, &_minOn, sizeof(_minOn));
	fileWrite(file, &_maxOn, sizeof(_maxOn));
	fileWrite(file, &_minOff, sizeof(_minOff));
	fileWrite(file, &_maxOff, sizeof(_maxOff));
	fileClose(file);
}

void AntiTheftClass::_loadBinConfig()
{
	Serial.printf("Try to load bin cfg..\n");
	if (fileExist("antiTheft.cfg"))
	{
		Serial.printf("Will load bin cfg..\n");
		file_t file = fileOpen("antiTheft.cfg", eFO_ReadOnly);
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, &_enableStartTime, sizeof(_enableStartTime));
		fileRead(file, &_enableStopTime, sizeof(_enableStopTime));
		fileRead(file, &_minOn, sizeof(_minOn));
		fileRead(file, &_maxOn, sizeof(_maxOn));
		fileRead(file, &_minOff, sizeof(_minOff));
		fileRead(file, &_maxOff, sizeof(_maxOff));
		fileClose(file);
	}
}

void AntiTheftClass::_enable(uint8_t enableState)
{
	if (enableState)
	{
		_enablerTimer.initializeMs(60000, TimerDelegate(&AntiTheftClass::_enablerCheck, this)).start(true);
		_enablerCheck();
	}
	else
	{
		_enableAntiTheft(false);
		_enablerTimer.stop();
	}
}

void AntiTheftClass::_enablerCheck()
{
	DateTime _now = SystemClock.now(eTZ_Local);
	uint16_t _now_minutes = _now.Hour * 60 + _now.Minute;

	if ((_now_minutes >= _enableStartTime) && (_now_minutes <= _enableStopTime))
	{
		Serial.println("AntiTheftChecker in range!");
		if (!_timer.isStarted())
		{
			Serial.print(_now.toFullDateTimeString()); Serial.println(" Turn Random ON! ");
			_enableAntiTheft(true);
		}
	}
	else
	{
		Serial.println("AntiTheftChecker out of range!");
		if (_enablerTimer.isStarted())
		{
			Serial.print(_now.toFullDateTimeString()); Serial.println(" Sleep time for Random! ");
			_enableAntiTheft(false);
		}
	}
}

void AntiTheftClass::wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	uint8_t* buffer = new uint8_t[wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2 + 2 + 2];
	switch (data[wsBinConst::wsSubCmd])
	{
	case AntiTheftClass::scATGetConfig:
	{
		buffer[wsBinConst::wsCmd] = wsBinConst::getResponse;
		buffer[wsBinConst::wsSysId] = sysId;
		buffer[wsBinConst::wsSubCmd] = AntiTheftClass::scATGetConfig;

		os_memcpy(&buffer[wsBinConst::wsPayLoadStart], &_enableStartTime, sizeof(_enableStartTime));
		os_memcpy(&buffer[wsBinConst::wsPayLoadStart + 2], &_enableStopTime, sizeof(_enableStopTime));
		os_memcpy(&buffer[wsBinConst::wsPayLoadStart + 2 + 2], &_minOn, sizeof(_minOn));
		os_memcpy(&buffer[wsBinConst::wsPayLoadStart + 2 + 2 + 2], &_maxOn, sizeof(_maxOn));
		os_memcpy(&buffer[wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2], &_minOff, sizeof(_minOff));
		os_memcpy(&buffer[wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2 + 2 ], &_maxOff, sizeof(_maxOff));

		socket.sendBinary(buffer, wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2 + 2 + 2);
		break;
	}
	}
	delete buffer;
}

void AntiTheftClass::wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	switch (data[wsBinConst::wsSubCmd])
	{
	case AntiTheftClass::scATSetConfig:
	{
		os_memcpy(&_enableStartTime, &data[wsBinConst::wsPayLoadStart], sizeof(_enableStartTime));
		os_memcpy(&_enableStopTime, &data[wsBinConst::wsPayLoadStart + 2], sizeof(_enableStopTime));
		os_memcpy(&_minOn, &data[wsBinConst::wsPayLoadStart + 2 + 2], sizeof(_minOn));
		os_memcpy(&_maxOn, &data[wsBinConst::wsPayLoadStart + 2 + 2 + 2], sizeof(_maxOn));
		os_memcpy(&_minOff, &data[wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2], sizeof(_minOff));
		os_memcpy(&_maxOff, &data[wsBinConst::wsPayLoadStart + 2 + 2 + 2 + 2 + 2 ], sizeof(_maxOff));

		_saveBinConfig();
		break;
	}
	}
}
