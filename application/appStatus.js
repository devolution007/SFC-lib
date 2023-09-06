'use strict';

import { websocket } from 'websocket';
import wsBin from 'wsBin';


export default function AppStatusClass(needRender = true) {
	this._counter = 0;
	this._timestamp = 0;
	this._dateStr = "";
	this._timer = 0;
	this._enable = false;
	this._needRender = needRender;
}

AppStatusClass.sysId = 1;

AppStatusClass.prototype.wsGetAppStatus = function() {
	wsBin.Cmd.Get(websocket, 1, wsBin.Const.scAppGetStatus);
}

AppStatusClass.prototype.wsBinProcess = function (bin) {
	var subCmd = bin.getUint8(wsBin.Const.wsSubCmd);
	if (subCmd == wsBin.Const.scAppGetStatus) {
		this._counter = bin.getUint32(wsBin.Const.wsPayLoadStart, true);
    	this._timestamp = bin.getUint32(wsBin.Const.wsPayLoadStart + 4, true);
		var d = new Date();
		d.setTime(this._timestamp * 1000);
		this._dateStr = d.toLocaleString().replace(/,\ /,'<br>');
		if (this._needRender) { this.renderStatus(); };
	}
}

AppStatusClass.prototype.render = function () {
	var t = document.querySelector('#AppStatusClass');
	var clone = document.importNode(t.content, true);
	var container = document.getElementById("Container-AppStatusClass");
	container.appendChild(clone);
}

AppStatusClass.prototype.renderStatus = function () {
	document.querySelector('#AppStatusClassCounter').textContent = this._counter;
	document.querySelector('#AppStatusClassDateTime').innerHTML = this._dateStr;
}

AppStatusClass.prototype.remove = function () {
		var removeElement = document.querySelector('#Container-AppStatusClass');
		this.removeChilds(removeElement);
}

AppStatusClass.prototype.removeChilds = function (node) {
    var last;
    while (last = node.lastChild) node.removeChild(last);
}

AppStatusClass.prototype.enable = function( enable ) {
	if ( enable != this._enable ) {
		this._enable = enable;
		if (! this._enable) {
			clearInterval(this._timer);
			this.remove();
		} else {
			if (this._needRender) { this.render(); };
			this.wsGetAppStatus()
			this._timer = setInterval(this.wsGetAppStatus, 5000);
		}	
	}
}