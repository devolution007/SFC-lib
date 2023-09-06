/*
 * AntiTheft.h
 *
 *  Created on: 21 дек. 2016 г.
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <binstate.h>
#include <binout.h>

class AntiTheftClass
{
public:
	AntiTheftClass(BinOutClass** outputs, uint8_t persistentId = 99); //provide persistent id unic by whole app
	void addOutputId(uint8_t outputId) { _outputsIds.add(outputId); };
	void start() { state.persistent(_persistentId); };
	BinStateClass state;
	static const uint8_t sysId = 5;
	static const uint8_t scATGetConfig = 1;
	static const uint8_t scATSetConfig = 2;
	void wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	void wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size);
private:
	void _turnOn();
	void _turnOff();
	void _enable(uint8_t enableState); // enable/disable checker
	void _enablerCheck(); // check if AntiTheft should be enabled
	void _enableAntiTheft(uint8_t enableState); // enable/disable AntiTheft "animation"
	void _saveBinConfig();
	void _loadBinConfig();
	Timer _timer;
	Timer _enablerTimer;
	uint16_t _enableStartTime = 0;
	uint16_t _enableStopTime = 1439;
	uint16_t _minOn = 15;
	uint16_t _maxOn = 45;
	uint16_t _minOff = 20;
	uint16_t _maxOff = 60;
	uint8_t _persistentId;
	BinOutClass** _outputs;
	Vector<uint8_t> _outputsIds = Vector<uint8_t>(0,1);
	uint8_t	_currentId = 0; //Id of currently selected output
};

