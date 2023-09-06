'use strict';

//var BinStatesNames = {};

import { websocket } from 'websocket';
import wsBin from 'wsBin';
import { BinStatesNames, BinStatesGroups } from 'BinStatesNames';

//BinStateHttpClass

function BinStateClass (uid) {
	this.uid = uid;
	this._state = 0; //false
	if ( this.isState() ) {
		this._name = BinStatesNames["states"][this.uid];
	}
	if ( this.isButton() ) {
		this._name = BinStatesNames["buttons"][this.uid - wsBin.Const.uidHttpButton];
	}
	this.render();
}

BinStateClass.sysId = 2;

BinStateClass.prototype.wsGet = function (cmd) {
	var ab = new ArrayBuffer(3);
	var bin = new DataView(ab);
	
	bin.setUint8(wsBin.Const.wsCmd, wsBin.Const.getCmd);
	bin.setUint8(wsBin.Const.wsSysId, BinStateClass.sysId); //BinStateHttpClass.sysId = 2
	bin.setUint8(wsBin.Const.wsSubCmd, cmd);

	websocket.send(bin.buffer);
//	console.log.bind(console)(`wsGet cmd = ${cmd}`);
}

//BinStateClass.prototype.wsGetName = function () {
//	this.wsGet(1);
//}

BinStateClass.prototype.wsGetState = function () {
	this.wsGet(2);
}

BinStateClass.prototype.wsSetState = function (state) {
	wsBin.Cmd.SetArg(websocket, BinStateClass.sysId, wsBin.Const.scBinStateSetState, this.uid, state);
}

//BinStateClass.prototype.wsGotName = function (bin) {
//	var strBuffer = new Uint8Array(bin.byteLength -(wsBin.Const.wsPayLoadStart + 1));
//	console.log.bind(console)(`uid = ${this.uid}, bin.byteLength = ${bin.byteLength}`);
//
//    for (var i = 0; i < strBuffer.length; i++) {
//        strBuffer[i] = bin.getUint8(wsBin.Const.wsPayLoadStart + 1 + i);
////        console.log.bind(console)(`uid = ${this.uid}, strBuffer[${i}] = ${bin.getUint8(wsBin.Const.wsPayLoadStart + 1 + i)}`);
//    }
//
//    this._name = new TextDecoder().decode(strBuffer)
//    this.renderName();
//}

BinStateClass.prototype.wsGotState = function (bin) {
	this._state = bin.getUint8(wsBin.Const.wsPayLoadStart + 1, true);
	this.renderState();
}

BinStateClass.prototype.render = function () {
	if ( this.isState() ) {
		var t = document.querySelector('#BinStateHttpClass');
		var clone = document.importNode(t.content, true);
		clone.querySelector('#binState').textContent = this._name;
		clone.querySelector('#binStateDiv').id = `binStateDiv${this.uid}`;
		clone.querySelector('#binStatePanel').id = `binStatePanel${this.uid}`;
		clone.querySelector('#binState').id = `binState${this.uid}`
		var container = document.getElementById("Container-BinStateHttpClassStates");
	}
			
	if ( this.isButton() ) {
		var t = document.querySelector('#BinStateHttpClassButton');
		var clone = document.importNode(t.content, true);
		clone.querySelector('#binStateButton').textContent = this._name;
		clone.querySelector('#binStateButtonDiv').id = `binStateButtonDiv${this.uid}`
		clone.querySelector('#binStateButton').addEventListener('mousedown', this);
		clone.querySelector('#binStateButton').addEventListener('mouseup', this);
		clone.querySelector('#binStateButton').id = `binStateButton${this.uid}`
		var container = document.getElementById("Container-BinStateHttpClassButtons");
	}
	
	container.appendChild(clone);	
}

//BinStateClass.prototype.renderName = function () {
//	if ( this.isState() ) {
//		//document.querySelector(`#binState${this.uid}`).textContent = this._name;
//		document.querySelector(`#binState${this.uid}`).textContent = BinStatesNames["states"][this.uid];
//	}
//	
//	if ( this.isButton() ) {
//		//document.querySelector(`#binStateButton${this.uid}`).textContent = this._name;
//		document.querySelector(`#binStateButton${this.uid}`).textContent = BinStatesNames["buttons"][this.uid - 127];
//	}
//}

BinStateClass.prototype.renderState = function () {
	if ( this.isState()) {
	    	var panel = document.querySelector(`#binStatePanel${this.uid}`);
	    	
	    	if (this._state) {
	    		panel.classList.remove("panel-primary");
	    		panel.classList.add("panel-danger");	
	    	} else {
	    		panel.classList.remove("panel-danger");
	    		panel.classList.add("panel-primary");
	    	}
    	}
    	
    	if ( this.isButton()) {
    		var panel = document.querySelector(`#binStateButton${this.uid}`);
	    	
	    	if (this._state) {
	    		panel.classList.remove("btn-primary");
	    		panel.classList.add("btn-warning");	
	    	} else {
	    		panel.classList.remove("btn-warning");
	    		panel.classList.add("btn-primary");
	    	}
    	}	
}

BinStateClass.prototype.remove = function () {
		var selector = this.isState() ?  `#binStateDiv${this.uid}` : `#binStateButtonDiv${this.uid}`

		var removeElement = document.querySelector(selector);
		this.removeChilds(removeElement);
		removeElement.remove();
}

BinStateClass.prototype.removeChilds = function (node) {
    var last;
    while (last = node.lastChild) node.removeChild(last);
}

BinStateClass.prototype.wsBinProcess = function (bin) {
	var subCmd = bin.getUint8(wsBin.Const.wsSubCmd);
	
	if ( subCmd == wsBin.Const.scBinStateGetName) {
		this.wsGotName(bin);
	}
	
	if ( subCmd == wsBin.Const.scBinStateGetState) {
		this.wsGotState(bin);
	}
	
}

BinStateClass.prototype.isButton = function () { return this.uid >= wsBin.Const.uidHttpButton; }
BinStateClass.prototype.isState = function () { return this.uid < wsBin.Const.uidHttpButton; }

BinStateClass.prototype.handleEvent = function(event) {
	switch(event.type) {
		case 'mousedown':
	        this.wsSetState(1);
	        break;
        case 'mouseup':
            this.wsSetState(0);
            break;
	}
}

//BinStatesHttpClass
// @param states - render/process states
// @param  buttons - render/process buttons

export default function BinStatesClass () {
	this._binStatesHttp = {};
	this._statesEnable = false;
	this._buttonsEnable = false;
	this._enable = false;
	this.renderGroupsMenu();
}

BinStatesClass.sysId = 2;

BinStatesClass.prototype.enableStates = function( statesEnable ) {
	if ( statesEnable != this._statesEnable ) {
		this._statesEnable = statesEnable;
		if (! this._statesEnable) {
			var self = this
			Object.keys(this._binStatesHttp).forEach(function(uid) {
				if ( self.isState(uid) ) {
					self._binStatesHttp[uid].remove();
					delete self._binStatesHttp[uid];
				}
			});
		} else {
			this.wsGetAllStates();
		}	
	}
}

BinStatesClass.prototype.enableButtons = function( buttonsEnable ) {
	if ( buttonsEnable != this._buttonsEnable ) {
		this._buttonsEnable = buttonsEnable;
		if (! this._buttonsEnable) {
			var self = this
			Object.keys(this._binStatesHttp).forEach(function(uid) {
				if ( self.isButton(uid) ) {
					self._binStatesHttp[uid].remove();
					delete self._binStatesHttp[uid];
				}
			});
		} else {
			this.wsGetAllButtons();
		}	
	}
}

BinStatesClass.prototype.enable = function( Enable ) {
	if ( Enable != this._enable ) {
		this._enable = Enable;
		this.enableStates(this._enable);
		this.enableButtons(this._enable);
	}
}
BinStatesClass.prototype.wsGetAll = function() {
	wsBin.Cmd.Get(websocket, BinStatesClass.sysId, wsBin.Const.scBinStatesGetAll);
}

BinStatesClass.prototype.wsGetAllStates = function() {
	wsBin.Cmd.Get(websocket, BinStatesClass.sysId, wsBin.Const.scBinStatesGetAllStates);
}

BinStatesClass.prototype.wsGetAllButtons = function() {
	wsBin.Cmd.Get(websocket, BinStatesClass.sysId, wsBin.Const.scBinStatesGetAllButtons);
}

BinStatesClass.prototype.wsBinProcess = function (bin) {
	var subCmd = bin.getUint8(wsBin.Const.wsSubCmd);
	var uid = bin.getUint8(wsBin.Const.wsPayLoadStart);
	
	if (Array.isArray(this._uids) && this._uids.length) {
		if (this._uids.includes(uid)) {
			if ( (this.isState(uid) && this._statesEnable) || (this.isButton(uid) && this._buttonsEnable ) ) {
				if (subCmd == wsBin.Const.scBinStateGetState) {
					if ( !this._binStatesHttp.hasOwnProperty(uid) ) {
						this._binStatesHttp[uid] = new BinStateClass(uid);
					}
					this._binStatesHttp[uid].wsGotState(bin);
				}
				
			}
		}
	}
	else {
		if ( (this.isState(uid) && this._statesEnable) || (this.isButton(uid) && this._buttonsEnable ) ) {
			if (subCmd == wsBin.Const.scBinStateGetState) {
				if ( !this._binStatesHttp.hasOwnProperty(uid) ) {
					this._binStatesHttp[uid] = new BinStateClass(uid);
				}
				this._binStatesHttp[uid].wsGotState(bin);
			}
			
		}
	}
	
	
}

BinStatesClass.prototype.isButton = function (uid) { return uid >= wsBin.Const.uidHttpButton; }
BinStatesClass.prototype.isState = function (uid) { return uid < wsBin.Const.uidHttpButton; }

BinStatesClass.prototype.showOnlyUids =  function ( _uidsArr ) {
	this.enableStates(false);
	this.enableButtons(false);
	this._uids = _uidsArr;
	this.enableStates(true);
	this.enableButtons(true);
	document.getElementById("navbar-toggle-cbox").checked = false;
	return false;
}

BinStatesClass.prototype.renderGroupsMenu =  function ( ) {
	if (Array.isArray(BinStatesGroups) && BinStatesGroups.length) {
		const fragment = document.createDocumentFragment();
		BinStatesGroups.forEach(function(item, i, arr) {
			const li = document.createElement('li');
			li.id = `BinStatesGroupsMenuElement${i}`;
			const a = document.createElement('a');
			a.href = '#';
			a.textContent = item['name'];
			li.appendChild(a);
			fragment.appendChild(li);
		});
		const container = document.getElementById('BinStatesGroupsMenu');
		container.appendChild(fragment);
		container.addEventListener('click',(event) => {
			let uids = BinStatesGroups[(event.target.parentElement.id).replace('BinStatesGroupsMenuElement','')]['uids'];
			this.showOnlyUids(uids);
		});
		const menu = document.getElementById('navbar');
		menu.addEventListener('click',(event) => {
			let elems = menu.getElementsByClassName('active');
			[].forEach.call(elems,(item, i, arr) => {
				item.classList.remove('active');
			});
			event.target.parentElement.classList.add('active');
		});
	}
}
