/*
 * binstate.cpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: shurik
 */
#include <binstate.h>


BinStateClass::BinStateClass(uint8_t polarity, uint8_t toggleActive)
: _state{0}, _uid{0}, _onSet{nullptr}
{
	polarity ? _state |= BinState::polarityBit : _state &= ~(BinState::polarityBit);
	toggleActive ? _state |= BinState::toggleActiveBit : _state &= ~(BinState::toggleActiveBit);
}

void BinStateClass::set(uint8_t state, uint8_t forceDelegatesCall)
{
//	uint8_t prevState = _state;

	_setPrev(get());

//	_state = state;
	state ? _setState(getPolarity()) : _setState(!getPolarity());

	Serial.printf(_F("State set to: %s\n"), get() ? _F("true") : _F("false"));
	Serial.printf(_F("prevState set to: %s\n"), getPrev() ? _F("true") : _F("false"));
	if (_onSet)
	{
		_onSet(get()); //Call some external delegate on setting ANY state
	}

	if (get() != getPrev() || forceDelegatesCall)
	{
		_callOnChangeDelegates(); //Call external delegates on state CHANGE

		if ( (_state & BinState::persistentBit) != 0 )
		{
			_saveBinConfig();
		}
	}
}

void BinStateClass::toggle(uint8_t state)
{
	Serial.println(_F("Toggle called\n"));
	if (state == getToggleActive())
	{
		set(!get());
		Serial.println(_F("Toggle TOGGLED!\n"));
	}
}

void BinStateClass::invert(uint8_t state)
{
	set(!get());
	Serial.println(_F("Flip FLIPPED!\n"));
}

void BinStateClass::onSet(onStateChangeDelegate delegateFunction)
{
	_onSet = delegateFunction;
}

void BinStateClass::onChange(onStateChangeDelegate delegateFunction, uint8_t polarity)
{
//	OnStateChange onStateChange = {delegateFunction, polarity};
	_onChange.push_back({delegateFunction, polarity});
}

void BinStateClass::_callOnChangeDelegates()
{
	for (uint8_t i = 0; i < _onChange.size(); i++)
	{
		_onChange[i].delegateFunction(get() ? _onChange[i].polarity : !_onChange[i].polarity);
	}
}

void BinStateClass::_saveBinConfig()
{
	Serial.printf(_F("Try to save bin cfg..\n"));
	file_t file = fileOpen(String(".state" + _uid), eFO_CreateIfNotExist | eFO_WriteOnly);
	fileWrite(file, &_state, sizeof(_state));
	fileClose(file);
}

void BinStateClass::_loadBinConfig()
{
	uint8_t tempState = 0;
	Serial.printf(_F("Try to load bin cfg..\n"));
	if (fileExist(String(".state" + _uid)))
	{
		Serial.printf(_F("Will load bin cfg..\n"));
		file_t file = fileOpen(String(".state" + _uid), eFO_ReadOnly);
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, &tempState, sizeof(tempState));
		fileClose(file);

		set( tempState & BinState::stateBit ? getPolarity() : !getPolarity() ); //To properly call all delegates and so on
	}
}

void BinStateClass::persistent(uint8_t uid)
{
	_uid = uid;
	_loadBinConfig();
	_state |= BinState::persistentBit;
}

// BinStateHttpClass
BinStateHttpClass::BinStateHttpClass(HttpServer& webServer, BinStateClass* outState, /*String name,*/ uint8_t uid, BinStateClass* inState)
: _webServer(webServer), _outState(outState), /*_name(name),*/ _uid(uid), _inState(inState)
{
	_outState->onChange(std::bind(&BinStateHttpClass::wsSendStateAll, this, std::placeholders::_1));
};

void BinStateHttpClass::wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	switch (data[wsBinConst::wsSubCmd])
	{
/*	case wsBinConst::scBinStateGetName:
		wsSendName(socket);
		break;*/
	case wsBinConst::scBinStateGetState:
		wsSendState(socket);
		break;
	}
}

/*void BinStateHttpClass::_fillNameBuffer(uint8_t* buffer)
{
	buffer[wsBinConst::wsCmd] = wsBinConst::getResponse;
	buffer[wsBinConst::wsSysId] = sysId;
	buffer[wsBinConst::wsSubCmd] = wsBinConst::scBinStateGetName;

	os_memcpy((&buffer[wsBinConst::wsPayLoadStart]), &_uid, sizeof(_uid));
	os_memcpy((&buffer[wsBinConst::wsPayLoadStart + 1]), _name.c_str(), _name.length());

}*/

void BinStateHttpClass::_fillStateBuffer(uint8_t* buffer)
{
	buffer[wsBinConst::wsCmd] = wsBinConst::getResponse;
	buffer[wsBinConst::wsSysId] = sysId;
	buffer[wsBinConst::wsSubCmd] = wsBinConst::scBinStateGetState;

	uint8_t tmpState = getState();

	os_memcpy((&buffer[wsBinConst::wsPayLoadStart]), &_uid, sizeof(_uid));
	os_memcpy((&buffer[wsBinConst::wsPayLoadStart + 1]), &tmpState, sizeof(tmpState));
}

/*void BinStateHttpClass::wsSendName(WebsocketConnection& socket)
{
	uint16_t bufferLength = wsBinConst::wsPayLoadStart + 1 + _name.length();
	uint8_t* buffer = new uint8_t[bufferLength];
	_fillNameBuffer(buffer);

	socket.sendBinary(buffer, bufferLength);
	delete buffer;
}*/

void BinStateHttpClass::wsSendState(WebsocketConnection& socket)
{
	uint8_t* buffer = new uint8_t[wsBinConst::wsPayLoadStart + 1 + 1];

	_fillStateBuffer(buffer);

	socket.sendBinary(buffer, wsBinConst::wsPayLoadStart + 1 + 1);

	delete buffer;
}

void BinStateHttpClass::wsSendStateAll(uint8_t state)
{
	uint8_t* buffer = new uint8_t[wsBinConst::wsPayLoadStart + 1 + 1];

	_fillStateBuffer(buffer);

//	WebSocketsList &clients = _webServer.getActiveWebSockets();
//	for (uint8_t i = 0; i < clients.count(); i++)
//	{
//		clients[i].sendBinary(buffer, wsBinConst::wsPayLoadStart + 1 + 1);
//	}
	WebsocketConnection::broadcast((const char*)buffer, wsBinConst::wsPayLoadStart + 1 + 1, WS_FRAME_BINARY);

	delete buffer;
}

void BinStateHttpClass::setState(uint8_t state)
{
//	Serial.printf("Inside setState!!!\n");

	if (_inState)
	{
//		Serial.printf("inState set\n");
		_inState->set(state);
	}
//	else
//	{
////		Serial.printf("outState set\n");
//		_outState->set(state);
//	}
};

void BinStatesHttpClass::wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	uint8_t* buffer = nullptr;
	switch (data[wsBinConst::wsSubCmd])
	{
	case wsBinConst::scBinStatesGetAll:
		for (uint8_t i = 0; i < _binStatesHttp.count(); i++)
		{
//			_binStatesHttp.valueAt(i)->wsSendName(socket);
			_binStatesHttp.valueAt(i)->wsSendState(socket);
		}
		break;
	case wsBinConst::scBinStatesGetAllStates:
		for (uint8_t i = 0; i < _binStatesHttp.count(); i++)
		{
			if ( _binStatesHttp.keyAt(i) < wsBinConst::uidHttpButton )
			{
//				_binStatesHttp.valueAt(i)->wsSendName(socket);
				_binStatesHttp.valueAt(i)->wsSendState(socket);
			}
		}
		break;
	case wsBinConst::scBinStatesGetAllButtons:
		for (uint8_t i = 0; i < _binStatesHttp.count(); i++)
		{
			if ( _binStatesHttp.keyAt(i) >= wsBinConst::uidHttpButton )
			{
//				_binStatesHttp.valueAt(i)->wsSendName(socket);
				_binStatesHttp.valueAt(i)->wsSendState(socket);
			}
		}
		break;
	case wsBinConst::scBinStateGetState:
		if (_binStatesHttp.contains(data[wsBinConst::wsGetSetArg]))
		{
			_binStatesHttp[data[wsBinConst::wsGetSetArg]]->wsSendState(socket);
		}
		break;
	}
}

void BinStatesHttpClass::wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	uint8_t* buffer = nullptr;
//	Serial.printf("BinStatesHttp -> wsBinSetter -> wsGetSetArg = %d\n", data[wsBinConst::wsGetSetArg]);
	switch (data[wsBinConst::wsSubCmd])
	{
	case wsBinConst::scBinStateSetState:
		if (_binStatesHttp.contains(data[wsBinConst::wsGetSetArg]))
		{
//			Serial.printf("BinStatesHttp -> wsBinSetter -> wsGetSetArg -> ELEMENT FOUND!\n");
			_binStatesHttp[data[wsBinConst::wsGetSetArg]]->setState(data[wsBinConst::wsPayLoadStartGetSetArg]);
		}
		break;
	}
}
//BinStateSharedDeferredClass

void BinStateSharedDeferredClass::set(uint8_t state)
{
	if ( !state && _consumers > 0 )
	{
		_consumers--;
		Serial.printf(_F("Decrease consumers = %d\n"), _consumers);
	}

	if ( _consumers == 0 && ( ((state && _trueDelay > 0) || (!state && _falseDelay > 0)) && !_nodelay) )
	{
		_setDeferredState(state);
		_delayTimer.initializeMs( (state ? _trueDelay : _falseDelay) * 60000, [=](){this->_deferredSet();}).start(false);
		Serial.printf(_F("Arm deferred %s\n"), state ? _F("True") : _F("False"));
	}

	if ( (_consumers == 0 && ( (state && _trueDelay == 0) || (!state && _falseDelay == 0) || _nodelay)) || ( state && _consumers > 0 && _nodelay && _delayTimer.isStarted()) )
	{
		Serial.printf(_F("Fire nodelay %s\n"), state ? _F("true") : _F("false"));
		_delayTimer.stop();
		BinStateClass::set(state, false);
	}

	if ( state )
	{
		_consumers++;
		Serial.printf(_F("Increase consumers = %d\n"), _consumers);
	}
}

void BinStateSharedDeferredClass::setNow(uint8_t state)
{
	_nodelay = true;
	set(state);
	_nodelay = false;
}
void BinStateSharedDeferredClass::_deferredSet()
{
	Serial.printf(_F("Fire deferred %s\n"), _getDefferedState() ? _F("true") : _F("false"));
	BinStateClass::set(_getDefferedState(), false);
	_delayTimer.stop();
}

//BinStateAndClass

void BinStateAndClass::addState(BinStateClass* binState)
{
	_states.push_back(binState);
	binState->onChange(std::bind(&BinStateAndClass::onChangeProcessor, this, std::placeholders::_1));
}

void BinStateAndClass::onChangeProcessor(uint8_t state)
{
	uint8_t result = true;
	for (uint8_t i = 0; i < _states.size(); i++)
	{
		result &= _states[i]->get();
	}
	BinStateClass::set(result, false);
}
