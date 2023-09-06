'use strict';

//TempsensorClass
function getJson(url) { return fetch(url).then(function(response){if (response.status >= 200 && response.status < 300) return response.json();})};

function TempsensorClass (uid, id) {
	this._id = id;
	this.uid = uid;
	this._temperature = 0;
	this._statusFlag = 0;
	this._name = uid;
	this.render();
}

TempsensorClass.statusFlags = { INVALID: 1, DISCONNECTED: 2};

TempsensorClass.prototype.wsGotName = function (uid) {
	this._name = uid;
    this.renderName();
}

TempsensorClass.prototype.wsGotTemperature = function (temperature, statusFlag) {
	this._temperature = temperature;
	this._statusFlag = statusFlag
	this.renderTemperature();
}

TempsensorClass.prototype.render = function () {
	var t = document.querySelector('#TempsensorClass');
	var clone = document.importNode(t.content, true);
	
	clone.querySelector('#TempsensorClassDiv').id = `TempsensorClassDiv${this._id + this.uid}`;
	clone.querySelector('#TempsensorClassPanel').id = `TempsensorClassPanel${this._id + this.uid}`;
	clone.querySelector('#TempsensorClassName').id = `TempsensorClassName${this._id + this.uid}`
	clone.querySelector('#TempsensorClassTemperature').id = `TempsensorClassTemperature${this._id + this.uid}`
		
	var container = document.getElementById("Container-TempsensorClassTemperatures");

	container.appendChild(clone);	
}

TempsensorClass.prototype.renderName = function () {
	document.querySelector(`#TempsensorClassName${this._id + this.uid}`).textContent = `Temperature #${this._name}`;
}

TempsensorClass.prototype.renderTemperature = function () {
	var panel = document.querySelector(`#TempsensorClassPanel${this._id + this.uid}`);
	
	if (this._statusFlag == 0 ) {
		panel.classList.remove("panel-danger");
		panel.classList.remove("panel-warning");
		panel.classList.add("panel-default");	
	} else if (this._statusFlag & TempsensorClass.statusFlags.DISCONNECTED ) {
		panel.classList.remove("panel-default");
		panel.classList.remove("panel-warning");
		panel.classList.add("panel-danger");
	} else if (this._statusFlag & TempsensorClass.statusFlags.INVALID ) {
		panel.classList.remove("panel-default");
		panel.classList.remove("panel-danger");
		panel.classList.add("panel-warning");
	}
	
	document.querySelector(`#TempsensorClassTemperature${this._id + this.uid}`).textContent = `${this._temperature} \xB0C`;
}

TempsensorClass.prototype.remove = function () {
	var removeElement = document.querySelector(`TempsensorClassDiv${this._id + this.uid}`);
	this.removeChilds(removeElement);
	removeElement.remove();
}

TempsensorClass.prototype.removeChilds = function (node) {
	var last;
	while (last = node.lastChild) node.removeChild(last);
}

// TempsensorsClass

export default function TempsensorsClass (url, id) {
	this._id = id;
	this._url = url;
	this._tempsensorsHttp = {};
	this._tempsensorsEnable = false;
}

TempsensorsClass.prototype.enable = function( tempsensorsEnable ) {
	if ( tempsensorsEnable != this._tempsensorsEnable ) {
		this._tempsensorsEnable = tempsensorsEnable;
		if (! this._tempsensorsEnable) {
			var self = this
			Object.keys(this._tempsensorsHttp).forEach(function(uid) {
				self._tempsensorsHttp[uid].remove();
				delete self._tempsensorsHttp[uid];
			});
		} else {
			this.wsGetAllTemperatures();
		}	
	}
}

TempsensorsClass.prototype.wsGetAllTemperatures = function () {
	var self = this;
	getJson(this._url)
	.then( function(temperatures) {
		Object.keys(temperatures).forEach(function(uid) {
			if ( !self._tempsensorsHttp.hasOwnProperty(uid) ) {
				self._tempsensorsHttp[uid] = new TempsensorClass(uid,self._id);
			}
			self._tempsensorsHttp[uid].wsGotName(uid);
			self._tempsensorsHttp[uid].wsGotTemperature(temperatures[uid].temperature, temperatures[uid].statusFlag);
		});
	});	
}
