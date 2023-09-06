/*
 * wsbinconst.h
 *
 *  Created on: 6 июля 2016
 *      Author: shurik
 */

#pragma once

namespace wsBinConst
{
//Frame header offsets
	const uint8_t wsCmd = 0; //Command type
	const uint8_t wsSysId = 1; //target sysId
	const uint8_t wsSubCmd = 2; //sub-command type
	const uint8_t wsPayLoadStart = 3;
	//alternatively if we need argument to Get value
	const uint8_t wsPayLoadStartGetSetArg = 4;
	const uint8_t wsGetSetArg = 3;

	const uint8_t reservedCmd = 0;
	const uint8_t getCmd = 1;
	const uint8_t setCmd = 2;
	const uint8_t getResponse = 3;
	const uint8_t setAck = 4;
	const uint8_t setNack =5;

// sub-commands for App sysId=1
	const uint8_t scAppSetTime = 1;
	const uint8_t scAppGetStatus = 2;
	const uint8_t scAppConfigGet = 3;
	const uint8_t scAppConfigSet = 4;
// sub-commands for BinStateHttpClass sysId=2 and BinStatesHttpClass sysId=3
	const uint8_t scBinStateGetName = 1;
	const uint8_t scBinStateGetState = 2;
	const uint8_t scBinStateSetState = 3;
// sub-commands for BinStatesHttpClass sysId=3
	const uint8_t scBinStatesGetAll = 10;
	const uint8_t scBinStatesGetAllStates = 20;
	const uint8_t scBinStatesGetAllButtons = 30;
// BinHttp State/Buttons base uid
	const uint8_t uidHttpState = 0;
	const uint8_t uidHttpButton = 127;
};
