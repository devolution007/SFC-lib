/*
 * thermo.cpp
 *
 *  Created on: 12 янв. 2016 г.
 *      Author: shurik
 */

#include <weekthermostat.h>

WeekThermostatClass::WeekThermostatClass( TempSensors &tempSensors, uint8_t sensorId, String name, uint16_t refresh)
{
	_tempSensors = &tempSensors;
	_sensorId = sensorId;
	_name = name;
	_refresh = refresh;
	state.set(false);
	_manual = false;
	_active = true;
}

void WeekThermostatClass::check()
{
	float currTemp = _tempSensors->getTemp(_sensorId);
	float targetTemp = 0;
	uint8_t currentProg = 0;
	//bool prevState = _state;

	if (_tempSensors->isValid(_sensorId))
	{
		_tempSensorValid = WeekThermostatConst::maxInvalidGetTemp;
	}
	else if (_tempSensorValid > 0)
	{
		_tempSensorValid--;
		Serial.printf(_F("Name: %s - TEMPSENSOR ERROR!, %d\n"), _name.c_str(), _tempSensorValid);
	}

	if (!_tempSensorValid)
	{
		state.set(true); // If we lost remote tempsensor we switch termostat on instantly
		Serial.printf(_F("Name: %s - TEMPSENSOR ERROR! - WE LOST IT!\n"), _name.c_str());
	}
	else
	{
		if (!_active)
		{
			Serial.println(_F("active = false"));
			targetTemp = WeekThermostatConst::antiFrozen;
		}
		else
		{
			DateTime now = SystemClock.now(eTZ_Local);
			SchedUnit daySchedule[WeekThermostatConst::maxProg] = _schedule[now.DayofWeek];
			uint16_t nowMinutes = now.Hour * 60 + now.Minute;
//			Serial.printf("Name: %s, DateTime: %s,", _name.c_str(), now.toFullDateTimeString().c_str());

			for (uint8_t i = 0; i < WeekThermostatConst::maxProg; i++)
			{
				uint8_t nextIdx = i < (WeekThermostatConst::maxProg - 1) ? i + 1 : 0;
	//			Serial.printf("I: %d, nextIdx: %d, nowMinutes: %d, daySchedule[i].minutes: %d, daySchedule[nextIdx].minutes: %d ",i, nextIdx, nowMinutes, daySchedule[i].start, daySchedule[nextIdx].start);

				bool dayTransit = daySchedule[i].start > daySchedule[nextIdx].start;
	//			Serial.printf("dayTransit: %d\n", dayTransit);

				if ( ((!dayTransit) && ((nowMinutes >= daySchedule[i].start) && (nowMinutes < daySchedule[nextIdx].start))) )
				{
//					Serial.printf("AND selected Prog: %d ", i);
					currentProg = i;
					break;
				}
				if ( ((dayTransit) && ((nowMinutes >= daySchedule[i].start) || (nowMinutes < daySchedule[nextIdx].start))) )
				{
//					Serial.printf("OR selected Prog: %d ", i);
					currentProg = i;
					break;
				}
			}

			if (_manual && !_prevManual)
			{
				Serial.println(_F("turn Manual on by user"));
				_prevManual = true;
				_manualProg = currentProg;
	//			targetTemp = float(_manualTargetTemp / 100.0);
			}
			else if (_prevManual && !_manual)
			{
				Serial.println(_F("turn Manual off by user"));
				_prevManual = false;
			}

			if (_manual)
				targetTemp = float(_manualTargetTemp / 100.0);
			else
				targetTemp = (float)daySchedule[currentProg].targetTemp / 100.0; //in-place convert to float

			if (_manual && (_manualProg != currentProg))
			{
				Serial.println(_F("turn Manual off with program change"));
				_manual = false;
				_prevManual = false;
				targetTemp = (float)daySchedule[currentProg].targetTemp / 100.0; //in-place convert to float
			}
		}

//		Serial.print("targetTemp: "); Serial.print(targetTemp); //FLOAT!!!

		if (currTemp >= targetTemp + (float)(_targetTempDelta / 100.0))
			state.set(false);
		if (currTemp <= targetTemp - (float)(_targetTempDelta / 100.0))
			state.set(true);
	}
//	Serial.printf(" State: %s\n", _state ? "true" : "false");
//	if (prevState != _state && onChangeState)
//	{
//		Serial.printf("onChangeState Delegate/CB called!\n");
//		onChangeState(_state);
//	}
}

void WeekThermostatClass::start()
{
	_refreshTimer.initializeMs(_refresh, [=](){this->check();}).start(true);
//	_tempSensor->start();
}

void WeekThermostatClass::stop()
{
	_refreshTimer.stop();
//	_tempSensor->stop();
}

uint8_t WeekThermostatClass::loadStateCfg()
{
	StaticJsonBuffer<stateJsonBufSize> jsonBuffer;

	if (fileExist(".state" + _name))
	{
		int size = fileGetSize(".state" + _name);
		char* jsonString = new char[size + 1];
		fileGetContent(".state" + _name, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		_name = String((const char*)root["name"]);
		_active = root["active"];
//		_manual = root["manual"];
		_manualTargetTemp = root["manualTargetTemp"];
		_targetTempDelta = root["targetTempDelta"];

		Serial.printf(_F("Name: %s, Active: %d, Manual: %d, ManualTT: %d, TTDelta: %d\n"),_name.c_str(),_active,_manual,_manualTargetTemp, _targetTempDelta);
		delete[] jsonString;
		return 0;
	}
	return 0;
}
void WeekThermostatClass::onStateCfg(HttpRequest &request, HttpResponse &response)
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
			StaticJsonBuffer<stateJsonBufSize> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
//			root.prettyPrintTo(Serial); //Uncomment it for debuging

			if (root["active"].success()) // Settings
			{
				_active = root["active"];
				saveStateCfg();
				return;
			}
			if (root["manual"].success()) // Settings
			{
				_manual = root["manual"];
//				saveStateCfg();
				return;
			}
			if (root["manualTargetTemp"].success()) // Settings
			{
				_manualTargetTemp = ((float)(root["manualTargetTemp"]) * 100);
				saveStateCfg();
				return;
			}
			if (root["targetTempDelta"].success()) // Settings
			{
				_targetTempDelta = ((float)(root["targetTempDelta"]) * 100);
				saveStateCfg();
				return;
			}
		}
	}
	else
	{
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();

		json["name"] = _name;
		json["active"] = _active;
		json["state"] = state.get();
		json["temperature"] = _tempSensors->getTemp(_sensorId);
		json["manual"] = _manual;
		json["manualTargetTemp"] = _manualTargetTemp;
		json["targetTempDelta"] = _targetTempDelta;

		response.setAllowCrossDomainOrigin("*");
		response.sendDataStream(stream, MIME_JSON);
	}
}

uint8_t WeekThermostatClass::saveStateCfg()
{
	StaticJsonBuffer<stateJsonBufSize> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["name"] = _name.c_str();
	root["active"] = _active;
//	root["manual"] = _manual;
	root["manualTargetTemp"] = _manualTargetTemp;
	root["targetTempDelta"] = _targetTempDelta;

//	root.prettyPrintTo(Serial);

	char buf[stateFileBufSize];
	root.printTo(buf, sizeof(buf));
	fileSetContent(".state" + _name, buf);

	return 0;
}

uint8_t WeekThermostatClass::loadScheduleCfg()
{
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;

	if (fileExist(".sched" + _name))
	{
		int size = fileGetSize(".sched" + _name);
		char* jsonString = new char[size + 1];
		fileGetContent(".sched" + _name, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		for (uint8_t day = 0; day < 7; day++)
			{
			Serial.printf(_F("%d: "), day);
				for (uint8_t prog = 0; prog < WeekThermostatConst::maxProg; prog++)
				{
					_schedule[day][prog].start = root[(String)day][prog]["s"];
					_schedule[day][prog].targetTemp = root[(String)day][prog]["tt"];
					Serial.printf(_F("{s: %d,tt: %d}"), _schedule[day][prog].start, _schedule[day][prog].targetTemp);
				}
				Serial.println();
			}
	}
	return 0;
}

void WeekThermostatClass::onScheduleCfg(HttpRequest &request, HttpResponse &response)
{
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;
	if (request.method == HTTP_POST)
	{
		String body = request.getBody();
		if (body == NULL)
		{
			Serial.println(_F("NULL bodyBuf"));
			return;
		}
		else
		{

			JsonObject& root = jsonBuffer.parseObject(body);
//			root.prettyPrintTo(Serial); //Uncomment it for debuging

			for (uint8_t day = 0; day < 7; day++)
			{
				if (root[(String)day].success())
				{
				  for (uint8_t prog = 0; prog < WeekThermostatConst::maxProg; prog++)
				  {
					  _schedule[day][prog].start = root[(String)day][prog]["s"];
					  _schedule[day][prog].targetTemp = root[(String)day][prog]["tt"];
					  Serial.printf(_F("{s: %d,tt: %d}"), _schedule[day][prog].start, _schedule[day][prog].targetTemp);
				  }
				  saveScheduleBinCfg();
				  return;
				}
			}
		}
	}
	else
	{
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();

		for (uint8_t day = 0; day < 7; day++)
		{
			JsonArray& jsonDay = root.createNestedArray((String)day);
			for (uint8_t prog = 0; prog < WeekThermostatConst::maxProg; prog++)
			{
				JsonObject& jsonProg = jsonBuffer.createObject();
				jsonProg["s"] = _schedule[day][prog].start;
				jsonProg["tt"] = _schedule[day][prog].targetTemp;
				jsonDay.add(jsonProg);
			}
		}
		String buf;

		root.printTo(buf);

		response.setAllowCrossDomainOrigin("*");
		response.setContentType(MIME_JSON);
		response.sendString(buf);
	}
}
uint8_t WeekThermostatClass::saveScheduleCfg()
{
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	for (uint8_t day = 0; day < 7; day++)
	{
		JsonArray& jsonDay = root.createNestedArray((String)day);
		for (uint8_t prog = 0; prog < WeekThermostatConst::maxProg; prog++)
		{
			JsonObject& jsonProg = jsonBuffer.createObject();
			jsonProg["s"] = _schedule[day][prog].start;
			jsonProg["tt"] = _schedule[day][prog].targetTemp;
			jsonDay.add(jsonProg);
		}
	}
	root.prettyPrintTo(Serial);

	char buf[scheduleFileBufSize];
	root.printTo(buf, sizeof(buf));
	fileSetContent(".sched" + _name, buf);
	return 0;
}

void WeekThermostatClass::saveScheduleBinCfg()
{
	file_t file = fileOpen(".schedule" + _name, eFO_CreateIfNotExist | eFO_WriteOnly);
	fileWrite(file, _schedule, sizeof(SchedUnit)*6*7);
	fileClose(file);
}

void WeekThermostatClass::loadScheduleBinCfg()
{
	file_t file = fileOpen(".schedule" + _name, eFO_ReadOnly);
	fileSeek(file, 0, eSO_FileStart);
	fileRead(file, _schedule, sizeof(SchedUnit)*6*7);
	fileClose(file);
}

//void WeekThermostatClass::onStateChange(onStateChangeDelegate delegateFunction)
//{
//	onChangeState = delegateFunction;
//}
