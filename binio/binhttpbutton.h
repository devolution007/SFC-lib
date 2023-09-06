/*
 * binhttpbutton.h
 *
 *  Created on: 16 июня 2016 г.
 *      Author: shurik
 */

#pragma once
#include <SmingCore.h>
#include <binin.h>
#include <binout.h>

class BinHttpButtonClass : public BinInClass
{
public:
	BinHttpButtonClass(HttpServer& webServer, BinStatesHttpClass& binStatesHttp, uint8_t unitNumber, /*String name = "Button",*/ BinStateClass *outputState = nullptr);
	virtual ~BinHttpButtonClass() {};
	void addOutState(BinStateClass *outState) { _binStateHttp->addOutState(outState); };
	uint8_t _readUnit() {return 0;}; //Do not used here
private:
	BinStateHttpClass* _binStateHttp;
	BinStatesHttpClass& _binStatesHttp;
};
