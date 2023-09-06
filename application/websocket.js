'use strict';
//Websockets
var websocket;
var wsEnablers = [];
var wsBinProcessors = {};

import ReconnectingWebSocket from 'reconnecting-websocket';
import wsBin from 'wsBin';

function onOpen(evt) {
//	console.log.bind(console)("CONNECTED");
	
	Object.keys(wsEnablers).forEach( function(Id) { 
		wsEnablers[Id](true);
	});
	
//	binStates = new BinStatesClass();
//	binStates.enableButtons(true);
//	binStates.enableStates(true);
//	setTimeout(function() { binStates.enableButtons(true); }, 500);
//	setTimeout(function() { binStates.enableStates(true); }, 850);
}

function onMessage(evt) {
//	console.log.bind(console)("Message recv: " + evt.data);
	if(evt.data instanceof ArrayBuffer) {
    	var bin = new DataView(evt.data);
    	
    	var cmd = bin.getUint8(wsBin.Const.wsCmd);
    	var sysId = bin.getUint8(wsBin.Const.wsSysId);
    	var subCmd = bin.getUint8(wsBin.Const.wsSubCmd);
//    	console.log.bind(console)(`cmd = ${cmd}, sysId = ${sysId}, subCmd = ${subCmd}`);
    	
//    	if ( cmd == wsBin.Const.getResponse && sysId == 1 ) {
//    		appStatus.wsBinProcess(bin);
//    	}
    	if ( wsBinProcessors.hasOwnProperty(sysId) ) {
    		wsBinProcessors[sysId](bin);
    	}
    	
//    	if ( cmd == wsBinConst.getResponse && ( sysId == 2 || sysId == 3) ) {
//    		binStates.wsBinProcess(bin);
//    	}
    		
  	} 
}

function onClose(evt) {
//	console.log.bind(console)("DISCONNECTED");
}

function onError(evt) {
//	console.log.bind(console)("ERROR: " + evt.data);
}

function initWS() {
	const wsOptions = { maxEnqueuedMessages: 5};
	var wsUri = "ws://" + location.host + "/ws";
	websocket = new ReconnectingWebSocket(wsUri, [], wsOptions);
	websocket.onopen = function(evt) { onOpen(evt) };
	websocket.onclose = function(evt) { onClose(evt) };
	websocket.onmessage = function(evt) { onMessage(evt) };
	websocket.onerror = function(evt) { onError(evt) };
	websocket.binaryType = 'arraybuffer';
}

export { initWS, websocket, wsEnablers, wsBinProcessors };