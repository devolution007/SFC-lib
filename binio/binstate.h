/*
 * binstate.h
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <wsbinconst.h>
#include <vector>

#ifndef ONSTATECHANGEDELEGATE_TYPE_DEFINED
#define ONSTATECHANGEDELEGATE
typedef std::function<void(uint8_t state)> onStateChangeDelegate;
#endif

namespace BinState
{
//Bitmask of _state field
	const uint8_t stateBit = 1u;
	const uint8_t prevStateBit = 2u;
	const uint8_t polarityBit = 4u;
	const uint8_t toggleActiveBit = 8u;
	const uint8_t persistentBit = 16u;
	const uint8_t deferredSetBit = 32u;
}

struct OnStateChange
{
	onStateChangeDelegate delegateFunction;
	uint8_t polarity;
};

class BinStateClass
{
public:
	BinStateClass(uint8_t polarity = true, uint8_t toggleActive = true);
	virtual ~BinStateClass() {};
	uint8_t get() { return _state & BinState::stateBit ? getPolarity() : !getPolarity(); };
	uint8_t getRawState() { return (_state & BinState::stateBit) != 0; };
	uint8_t getPrev() { return (_state & BinState::prevStateBit) != 0; };
	uint8_t getPolarity() { return (_state & BinState::polarityBit) != 0; }
	uint8_t getToggleActive() { return (_state & BinState::toggleActiveBit) != 0; };
	void persistent(uint8_t uid); //Call this after constructor to save state in file, uid = UNIC on device id
	virtual void set(uint8_t state, uint8_t forceDelegatesCall);
	virtual void set(uint8_t state) { set(state, false); };
	virtual void setTrue(uint8_t state) { if (state) set(true); };
	virtual void setTrue() { set(true); };
	virtual void setFalse(uint8_t state) { if (state) set(false); };
	virtual void setFalse() { set(false); };
	void setPolarity(uint8_t polarity) { polarity ? _state |= BinState::polarityBit : _state &= ~(BinState::polarityBit); };
	void setToggleActive(uint8_t toggleActive) { toggleActive ? _state |= BinState::toggleActiveBit : _state &= ~(BinState::toggleActiveBit); };
	void toggle(uint8_t state = true); //there is argument for use this method in tru/false delegates, can use it without arguments too
	void invert(uint8_t state = true);
	void onSet(onStateChangeDelegate delegateFunction);
	void onChange(onStateChangeDelegate delegateFunction, uint8_t polarity = true);
protected:
	void _saveBinConfig();
	void _loadBinConfig();
	void _setState(uint8_t state) { state ? _state |= BinState::stateBit : _state &= ~(BinState::stateBit);};
	void _setPrev(uint8_t state) { state ? _state |= BinState::prevStateBit : _state &= ~(BinState::prevStateBit);};
	uint8_t _state; //BITMASK FIELD!!!
	uint8_t _uid; // unic id used if persistent enabled as file name uid, must be unic on device
	void _callOnChangeDelegates();
	onStateChangeDelegate _onSet; // call this with _state as argument
	std::vector<OnStateChange> _onChange;// = Vector<OnStateChange>(1,1); // call them with _state as argument
};

class BinStateHttpClass
{
public:
	BinStateHttpClass(HttpServer& webServer, BinStateClass* outState, /*String name,*/ uint8_t uid, BinStateClass* inState = nullptr);
//	: _webServer(webServer), _state(state), _name(name), _uid(uid) { _updateLength(); };
	void wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	void wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	void wsSendStateAll(uint8_t state);
	void wsSendState(WebsocketConnection& socket);
//	void wsSendName(WebsocketConnection& socket);
	void addOutState(BinStateClass *outState) { if (outState) { _outState = outState; }; };
	void setState(uint8_t state);
	uint8_t getState() { return _outState->get(); };
	uint8_t getUid() { return _uid; };
	static const uint8_t sysId = 2;
private:
	void _updateLength();
//	void _fillNameBuffer(uint8_t* buffer);
	void _fillStateBuffer(uint8_t* buffer);

	HttpServer& _webServer;
	BinStateClass* _outState;
//	String _name;
	uint8_t _uid;
	BinStateClass* _inState = nullptr;
};

class BinStatesHttpClass
{
public:
	void wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	void wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	void add(BinStateHttpClass* binStateHttp) { _binStatesHttp[binStateHttp->getUid()] = binStateHttp; };
	static const uint8_t sysId = 2;
private:
	HashMap<uint8_t,BinStateHttpClass*> _binStatesHttp;
};

class BinStateSharedDeferredClass : public BinStateClass
{
public:
	BinStateSharedDeferredClass(uint8_t trueDelay, uint8_t falseDelay) : _consumers{0}, _trueDelay{trueDelay}, _falseDelay{falseDelay}{};
	BinStateSharedDeferredClass() : BinStateSharedDeferredClass(0,0) {};
	virtual void set(uint8_t state);
	void setNow(uint8_t state);
	void setTrueDelay(uint16_t trueDelay) { _trueDelay = trueDelay; };
	void setFalseDelay(uint16_t falseDelay) { _falseDelay = falseDelay; };
private:
	void _setDeferredState(uint8_t state) { state ? _state |= BinState::deferredSetBit : _state &= ~(BinState::deferredSetBit);};
	uint8_t _getDefferedState() { return (_state & BinState::deferredSetBit) != 0; };
	void _deferredSet();
	uint8_t _consumers;
	uint16_t _trueDelay;
	uint16_t _falseDelay;
	uint8_t _nodelay = false;
	Timer _delayTimer;
};

class BinStateAndClass : public BinStateClass
{
public:
	void addState(BinStateClass* binState);
	void onChangeProcessor(uint8_t state);
private:
	std::vector<BinStateClass*> _states; // = Vector<BinStateClass*>(1,1);
};
