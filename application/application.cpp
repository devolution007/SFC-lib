#include <application.h>

//ApplicationClass App;

void ApplicationClass::init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	Serial.commandProcessing(true);
	HttpServerSettings webServerCfg;
	webServerCfg.keepAliveSeconds = 2;
	webServerCfg.minHeapSize = 7000;
	webServer.configure(webServerCfg);
	int slot = rboot_get_current_rom();
#ifndef DISABLE_SPIFFS
	if (slot == 0) {
#ifdef RBOOT_SPIFFS_0
//		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_0, SPIFF_SIZE);
#else
//		debugf("trying to mount spiffs at %x, length %d", 0x100000, SPIFF_SIZE);
		spiffs_mount_manual(0x100000, SPIFF_SIZE);
#endif
	} else {
#ifdef RBOOT_SPIFFS_1
//		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_1, SPIFF_SIZE);
#else
//		debugf("trying to mount spiffs at %x, length %d", 0x300000, SPIFF_SIZE);
		spiffs_mount_manual(0x300000, SPIFF_SIZE);
#endif
	}
#else
//	debugf("spiffs disabled");
#endif
//	spiffs_mount(); // Mount file system, in order to work with files

	_initialWifiConfig();

	loadConfig();

//	ntpClient = new NtpClient("pool.ntp.org", 300); //uncomment to enablentp update of system time
	SystemClock.setTimeZone(timeZone); //set time zone from config
	Serial.printf(_F("Time zone: %d\n"), timeZone);

	// Attach Wifi events handlers
	WifiEvents.onStationDisconnect(StationDisconnectDelegate(&ApplicationClass::_STADisconnect, this));
	WifiEvents.onStationConnect(StationConnectDelegate(&ApplicationClass::_STAConnect, this));
	WifiEvents.onStationAuthModeChange(StationAuthModeChangeDelegate(&ApplicationClass::_STAAuthModeChange, this));
	WifiEvents.onStationGotIP(StationGotIPDelegate(&ApplicationClass::_STAGotIP, this));

	// Web Sockets configuration
	_wsResource=new WebsocketResource();
	_wsResource->setConnectionHandler(WebsocketDelegate(&ApplicationClass::wsConnected,this));
	_wsResource->setMessageHandler(WebsocketMessageDelegate(&ApplicationClass::wsMessageReceived,this));
	_wsResource->setBinaryHandler(WebsocketBinaryDelegate(&ApplicationClass::wsBinaryReceived,this));
	_wsResource->setDisconnectionHandler(WebsocketDelegate(&ApplicationClass::wsDisconnected,this));

	webServer.paths.set("/ws", _wsResource);
//	webServer.setWebSocketConnectionHandler(WebSocketDelegate(&ApplicationClass::wsConnected,this));
//	webServer.setWebSocketMessageHandler(WebSocketMessageDelegate(&ApplicationClass::wsMessageReceived,this));
//	webServer.setWebSocketBinaryHandler(WebSocketBinaryDelegate(&ApplicationClass::wsBinaryReceived,this));
//	webServer.setWebSocketDisconnectionHandler(WebSocketDelegate(&ApplicationClass::wsDisconnected,this));

	wsAddBinSetter(sysId, WebsocketBinaryDelegate(&ApplicationClass::wsBinSetter,this));
	wsAddBinGetter(sysId, WebsocketBinaryDelegate(&ApplicationClass::wsBinGetter,this));

	startWebServer();
}

void ApplicationClass::start()
{
//	_loopTimer.initializeMs(loopInterval, TimerDelegateStdFunction(&ApplicationClass::_loop, this)).start(true);
	_loopTimer.initializeMs(loopInterval, [=](){ this->_loop();}).start(true);
	_loop();
}

void ApplicationClass::stop()
{
	_loopTimer.stop();
}

void ApplicationClass::_loop()
{
	_counter++;
}

void ApplicationClass::_initialWifiConfig()
{
// Set DHCP hostname to WebAppXXXX where XXXX is last 4 digits of MAC address
	String macDigits =  WifiStation.getMAC().substring(8,12);
	macDigits.toUpperCase();
	WifiStation.setHostname("WebApp" + macDigits);

// One-time set own soft Access Point SSID and PASSWORD and save it into configuration area
// This part of code will run ONCE after application flash into the ESP
	if(WifiAccessPoint.getSSID() != WIFIAP_SSID + macDigits)
	{
		WifiAccessPoint.config(WIFIAP_SSID + macDigits, WIFIAP_PWD, AUTH_WPA2_PSK);
		WifiAccessPoint.enable(true, true);
	}
	else
		Serial.printf(_F("AccessPoint already configured.\n"));

// One-time set initial SSID and PASSWORD for Station mode and save it into configuration area
// This part of code will run ONCE after application flash into the ESP if there is no
// pre-configured SSID and PASSWORD found in configuration area. Later you can change
// Station SSID and PASSWORD from Web UI and they will NOT overwrite by this part of code

	if (WifiStation.getSSID().length() == 0)
	{
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiStation.enable(true, true);
		WifiAccessPoint.enable(false, true);
	}
	else
		Serial.printf(_F("Station already configured.\n"));
}

void ApplicationClass::_STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	Serial.print(_F("Disconnected from \""));
	Serial.print(ssid);
	Serial.print(_F("\", reason: "));
	Serial.println(WifiEvents.getDisconnectReasonDesc(reason));
	
	_reconnectTimer.stop();
	if (!WifiAccessPoint.isEnabled())
	{
		Serial.println(_F("Starting OWN AP"));
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void ApplicationClass::_STAAuthModeChange(WifiAuthMode oldMode, WifiAuthMode newMode)
{
	//debugf("AUTH MODE CHANGE - OLD MODE: %d, NEW MODE: %d\n", oldMode, newMode);

	if (!WifiAccessPoint.isEnabled())
	{
		debugf("Starting OWN AP");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void ApplicationClass::_STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	debugf("GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(),
																mask.toString().c_str(),
																gateway.toString().c_str());
	_reconnectTimer.stop();
	if (WifiAccessPoint.isEnabled())
	{
		_reconnectTimer.initializeMs(60000, [this](){Serial.printf(_F("Shutdown OWN AP\n")); WifiAccessPoint.enable(false);}).startOnce();
//		debugf("Shutdown OWN AP");
//		WifiAccessPoint.enable(false);
	}
	// Add commands to be executed after successfully connecting to AP and got IP from it
	userSTAGotIP(ip, mask, gateway);
}

void ApplicationClass::_STAConnect(const String& ssid, MacAddress bssid, uint8_t channel)
{
	debugf("DELEGATE CONNECT - SSID: %s, CHANNEL: %d\n", ssid.c_str(), channel);

	wifi_station_dhcpc_set_maxtry(128);
//	_reconnectTimer.initializeMs(35000, TimerDelegateStdFunction(&ApplicationClass::_STAReconnect,this)).start();
	_reconnectTimer.initializeMs(35000, [=](){this->_STAReconnect();}).start();
	// Add commands to be executed after successfully connecting to AP
}

void ApplicationClass::_STAReconnect()
{
	WifiStation.disconnect();
	WifiStation.connect();
}

void ApplicationClass::startWebServer()
{
	if (_webServerStarted) return;

	webServer.listen(80);
	webServer.paths.set("/",HttpPathDelegate(&ApplicationClass::_httpOnIndex,this));
	webServer.paths.set("/ip",HttpPathDelegate(&ApplicationClass::_httpOnIp,this));
	webServer.paths.set("/config",HttpPathDelegate(&ApplicationClass::_httpOnConfiguration,this));
	webServer.paths.set("/config.json",HttpPathDelegate(&ApplicationClass::_httpOnConfigurationJson,this));
//	webServer.addPath("/state.json",HttpPathDelegate(&ApplicationClass::_httpOnStateJson,this));
	webServer.paths.set("/update",HttpPathDelegate(&ApplicationClass::_httpOnUpdate,this));
	webServer.paths.setDefault(HttpPathDelegate(&ApplicationClass::_httpOnFile,this));
	webServer.setBodyParser("application/json", bodyToStringParser);
	_webServerStarted = true;

	if (WifiStation.isEnabled())
		debugf("STA: %s", WifiStation.getIP().toString().c_str());
	if (WifiAccessPoint.isEnabled())
		debugf("AP: %s", WifiAccessPoint.getIP().toString().c_str());
}

void ApplicationClass::_httpOnFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.uri.Path;
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void ApplicationClass::_httpOnIndex(HttpRequest &request, HttpResponse &response)
{
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile("index.html");
}

void ApplicationClass::_httpOnIp(HttpRequest &request, HttpResponse &response)
{
	response.sendString(WifiStation.getIP().toString());
}

void ApplicationClass::_httpOnConfiguration(HttpRequest &request, HttpResponse &response)
{

	if (request.method == HTTP_POST)
	{
		String body = request.getBody();
		debugf("Update configuration\n");

		if (body == NULL)
		{
			debug_d("Empty Request Body!\n");
			return;
		}
		else // Request Body Not Empty
		{
//Uncomment next line for extra debuginfo
//			Serial.printf(body);
			uint8_t needSave = false;
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(body);
//Uncomment next line for extra debuginfo
//			root.prettyPrintTo(Serial);

			//Mandatory part to setup WIFI
			_handleWifiConfig(root);

			//Application config processing

			if (root["loopInterval"].success()) // There is loopInterval parameter in json
			{
				loopInterval = root["loopInterval"];
				start(); // restart main application loop with new loopInterval setting
				needSave = true;
			}


			if (root["updateURL"].success()) // There is loopInterval parameter in json
			{
				updateURL = String((const char *)root["updateURL"]);
				needSave = true;
			}

			if (needSave)
			{
				saveConfig();
			}
		} // Request Body Not Empty
	} // Request method is POST
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile("config.html");
	}
}

void ApplicationClass::_handleWifiConfig(JsonObject& root)
{
	String StaSSID = root["StaSSID"].success() ? String((const char *)root["StaSSID"]) : "";
	String StaPassword = root["StaPassword"].success() ? String((const char *)root["StaPassword"]) : "";
	uint8_t StaEnable = root["StaEnable"].success() ? root["StaEnable"] : 255;

	if (StaEnable != 255) // WiFi Settings
	{
		if (StaEnable)
		{
			if (WifiStation.isEnabled())
			{
				WifiAccessPoint.enable(false);
			}
			else
			{
				WifiStation.enable(true, true);
				WifiAccessPoint.enable(false, true);
			}
			if (WifiStation.getSSID() != StaSSID || (WifiStation.getPassword() != StaPassword && StaPassword.length() >= 8))
			{
				WifiStation.config(StaSSID, StaPassword);
				WifiStation.connect();
			}
		}
		else
		{
				WifiStation.enable(false, true);
				WifiAccessPoint.enable(true, true);
				WifiAccessPoint.config(WIFIAP_SSID, WIFIAP_PWD, AUTH_WPA2_PSK);
		}
	} //Wifi settings
}

void ApplicationClass::_httpOnConfigurationJson(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	//Mandatory part of WIFI Station SSID & Station mode enable config
	json["StaSSID"] = WifiStation.getSSID();
	json["StaEnable"] = WifiStation.isEnabled() ? 1 : 0;

	//Application configuration parameters
	json["loopInterval"] = loopInterval;
	json["updateURL"] = updateURL;

	//response.sendJsonObject(stream);
	response.sendDataStream(stream, MIME_JSON);
}

void ApplicationClass::loadConfig()
{
	uint16_t strSize;

	Serial.printf(_F("Try to load ApplicationClass bin cfg..\n"));
	if (fileExist(_fileName))
	{
		Serial.printf(_F("Will load ApplicationClass bin cfg..\n"));
		file_t file = fileOpen(_fileName, eFO_ReadOnly);
		fileSeek(file, 0, eSO_FileStart);
		fileRead(file, &strSize, sizeof(strSize));
		uint8_t* updateURLBuffer = new uint8_t[strSize+1];
		fileRead(file, updateURLBuffer, strSize);
		updateURLBuffer[strSize] = 0;
		updateURL = (const char *)updateURLBuffer;
		fileRead(file, &loopInterval, sizeof(loopInterval));
		fileRead(file, &timeZone, sizeof(timeZone));

		_loadAppConfig(file); //load additional, child class config here

		fileClose(file);

		delete [] updateURLBuffer;
	}
}

void ApplicationClass::saveConfig()
{
	uint16_t strSize = updateURL.length();

	Serial.printf(_F("Try to save ApplicationClass bin cfg..\n"));
	file_t file = fileOpen(_fileName, eFO_CreateNewAlways | eFO_WriteOnly);
	fileWrite(file, &strSize, sizeof(strSize));
	fileWrite(file, updateURL.c_str(), strSize);
	fileWrite(file, &loopInterval, sizeof(loopInterval));
	fileWrite(file, &timeZone, sizeof(timeZone));

	_saveAppConfig(file); //save additional, child class config here

	fileClose(file);
}

void ApplicationClass::OtaUpdate_CallBack(RbootHttpUpdater& client, bool result)
{
	Serial.println(_F("In callback..."));
	if(result == true) {
		// success
		uint8 slot;
		slot = rboot_get_current_rom();
		if(slot == 0)
			slot = 1;
		else
			slot = 0;
		// set to boot new rom and then reboot
		Serial.printf(_F("Firmware updated, rebooting to rom %d...\r\n"), slot);
		rboot_set_current_rom(slot);
		System.restart();
	} else {
		// fail
		Serial.println(_F("Firmware update failed!"));
	}
}

void ApplicationClass::OtaUpdate()
{

	uint8 slot;
	rboot_config bootconf;

	Serial.println(_F("Updating..."));

	// need a clean object, otherwise if run before and failed will not run again
	if(otaUpdater)
		delete otaUpdater;
	otaUpdater = new RbootHttpUpdater();

	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if(slot == 0)
		slot = 1;
	else
		slot = 0;

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(bootconf.roms[slot], updateURL + "rom0.bin");
#else
	// flash appropriate rom
	if(slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
	}
#endif

#ifndef DISABLE_SPIFFS
	// use user supplied values (defaults for 4mb flash in makefile)
	if(slot == 0) {
		otaUpdater->addItem(RBOOT_SPIFFS_0, updateURL + "spiff_rom.bin");
	} else {
		otaUpdater->addItem(RBOOT_SPIFFS_1, updateURL + "spiff_rom.bin");
	}
#endif

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback([this](RbootHttpUpdater& client, bool result){this->OtaUpdate_CallBack(client,result);});

	// start update
	otaUpdater->start();
}

void ApplicationClass::Switch()
{
	uint8 before, after;
	before = rboot_get_current_rom();
	if(before == 0)
		after = 1;
	else
		after = 0;
	Serial.printf(_F("Swapping from rom %d to rom %d.\r\n"), before, after);
	rboot_set_current_rom(after);
	Serial.println(_F("Restarting...\r\n"));
	System.restart();
}

void ApplicationClass::_httpOnUpdate(HttpRequest &request, HttpResponse &response)
{
	if (request.method == HTTP_POST)
		{
			String body = request.getBody();
			debug_d("Update POST request\n");

			if (body == NULL)
			{
				debugf("Empty Request Body!\n");
				return;
			}
			else // Request Body Not Empty
			{
	//Uncomment next line for extra debuginfo
	//			Serial.printf(request.getBody());
				DynamicJsonBuffer jsonBuffer;
				JsonObject& root = jsonBuffer.parseObject(body);
	//Uncomment next line for extra debuginfo
				root.prettyPrintTo(Serial);


				//Application config processing

				if (root["update"].success()) // There is loopInterval parameter in json
				{
					OtaUpdate();
				}
				if (root["switch"].success()) // There is loopInterval parameter in json
				{
					Switch();
				}
			} // Request Body Not Empty
		} // Request method is POST
}
//WebSocket handling
void ApplicationClass::wsConnected(WebsocketConnection& socket)
{
	Serial.printf(_F("WS CONN!\n"));
}

void ApplicationClass::wsDisconnected(WebsocketConnection& socket)
{
	Serial.printf(_F("WS DISCONN!\n"));
}

void ApplicationClass::wsMessageReceived(WebsocketConnection& socket, const String& message)
{
	Serial.printf(_F("WS msg recv: %s\n"), message.c_str());
}

void ApplicationClass::wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	Serial.printf(_F("WS bin data recv, size: %d\r\n"), size);
	Serial.printf(_F("Opcode: %d\n"), data[0]);

	if ( data[wsBinConst::wsCmd] == wsBinConst::setCmd)
	{
		Serial.printf(_F("wsSetCmd\n"));
		if (_wsBinSetters.contains(data[wsBinConst::wsSysId]))
		{
			Serial.printf(_F("wsSysId = %d setter called!\n"), data[wsBinConst::wsSysId]);
			_wsBinSetters[data[wsBinConst::wsSysId]](socket, data, size);
		}
	}

	if ( data[wsBinConst::wsCmd] == wsBinConst::getCmd)
	{
		Serial.printf(_F("wsGetCmd\n"));
		if (_wsBinGetters.contains(data[wsBinConst::wsSysId]))
		{
			Serial.printf(_F("wsSysId = %d getter called!\n"), data[wsBinConst::wsSysId]);
			_wsBinGetters[data[wsBinConst::wsSysId]](socket, data, size);
		}
	}
}

void ApplicationClass::wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	switch (data[wsBinConst::wsSubCmd])
	{
	case wsBinConst::scAppSetTime:
	{
		uint32_t timestamp = 0;
		os_memcpy(&timestamp, (&data[wsBinConst::wsPayLoadStart]), 4);
		if (timeZone != data[wsBinConst::wsPayLoadStart + 4])
		{
			timeZone = data[wsBinConst::wsPayLoadStart + 4];
			saveConfig();
			SystemClock.setTimeZone(timeZone);
		}
		SystemClock.setTime(timestamp, eTZ_UTC);
		break;
	}
	}
}

void ApplicationClass::wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	uint8_t* buffer;
	switch (data[wsBinConst::wsSubCmd])
	{
	case wsBinConst::scAppGetStatus:
	{
		buffer = new uint8_t[wsBinConst::wsPayLoadStart + 4 + 4];
		buffer[wsBinConst::wsCmd] = wsBinConst::getResponse;
		buffer[wsBinConst::wsSysId] = sysId;
		buffer[wsBinConst::wsSubCmd] = wsBinConst::scAppGetStatus;

		DateTime now = SystemClock.now(eTZ_UTC);
		uint32_t timestamp = now.toUnixTime();
		os_memcpy((&buffer[wsBinConst::wsPayLoadStart]), &_counter, sizeof(_counter));
		os_memcpy((&buffer[wsBinConst::wsPayLoadStart + 4]), &timestamp, sizeof(timestamp));
		socket.sendBinary(buffer, wsBinConst::wsPayLoadStart + 4 + 4);
		delete buffer;
		break;
	}
	}
}

void ApplicationClass::wsAddBinSetter(uint8_t sysId, WebsocketBinaryDelegate wsBinSetterDelegate)
{
	_wsBinSetters[sysId] = wsBinSetterDelegate;
}

void ApplicationClass::wsAddBinGetter(uint8_t sysId, WebsocketBinaryDelegate wsBinGetterDelegate)
{
	_wsBinGetters[sysId] = wsBinGetterDelegate;
}
