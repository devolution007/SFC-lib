'use strict';
//wsBinProtocol constants
var wsBin = {
	Const :{
//Frame header offsets
	wsCmd			: 0, //Command type
	wsSysId			: 1, //target sysId
	wsSubCmd		: 2, //sub-command type
	wsPayLoadStart	: 3,
	//alternatively if we need argument to Get value
	wsPayLoadStartGetSetArg	: 4,
	wsGetSetArg		: 3,

	reservedCmd		: 0,
	getCmd			: 1,
	setCmd			: 2,
	getResponse		: 3,
	setAck			: 4,
	setNack			: 5,

// sub-command
	scAppSetTime	: 1,
	scAppGetStatus	: 2,
	scAppConfigGet	: 3,
	scAppConfigSet	: 4,
// sub-commands for BinStateHttpClass sysId=2 and BinStatesHttpClass sysId=3
	scBinStateGetName	: 1,
	scBinStateGetState	: 2,
	scBinStateSetState	: 3,
// sub-commands for BinStatesHttpClass sysId=3
	scBinStatesGetAll		: 10,
	scBinStatesGetAllStates	: 20,
	scBinStatesGetAllButtons	: 30,
// BinHttp State/Buttons base numbers
	uidHttpState		: 0,
	uidHttpButton		: 127
	},
	
	Cmd: {
	Get	: function (socket, sysId, subCmd) {
		var ab = new ArrayBuffer(3);
		var bin = new DataView(ab);
		
		bin.setUint8(wsBin.Const.wsCmd, wsBin.Const.getCmd);
		bin.setUint8(wsBin.Const.wsSysId, sysId);
		bin.setUint8(wsBin.Const.wsSubCmd, subCmd);
	
		socket.send(bin.buffer);
//		console.log.bind(console)(`wsGet sysId = ${sysId}, subCmd = ${subCmd}`);
	},
	SetArg: function (socket, sysId, subCmd, setArg, setValue) {
		var ab = new ArrayBuffer(5);
		var bin = new DataView(ab);
		
		bin.setUint8(wsBin.Const.wsCmd, wsBin.Const.setCmd);
		bin.setUint8(wsBin.Const.wsSysId, sysId);
		bin.setUint8(wsBin.Const.wsSubCmd, subCmd);
		bin.setUint8(wsBin.Const.wsGetSetArg, setArg);
		bin.setUint8(wsBin.Const.wsPayLoadStartGetSetArg, setValue);
	
		socket.send(bin.buffer);
//		console.log.bind(console)(`wsGet sysId = ${sysId}, subCmd = ${subCmd}`);
	}
	}
};

export default wsBin;