#pragma once
#include <SmingCore.h>
#include <esp_spi_flash.h>
#include <Network/Http/Websocket/WebsocketResource.h>
#include <JsonObjectStream.h>
#include <wsbinconst.h>

// If you want, you can define settings globally in Operation System ENV
// or better in Eclipse Environment Variables

//Wifi Station mode SSID and PASSWORD
#ifndef WIFI_SSID
        #define WIFI_SSID "PleaseEnterSSID" // Put you Station mode SSID and Password here
        #define WIFI_PWD "PleaseEnterPass"
#endif
//OWN Access Point mode SSID and PASSWORD
#ifndef WIFIAP_SSID
        #define WIFIAP_SSID "WebApp" // Put you Station mode SSID and Password here
        #define WIFIAP_PWD "smingweb"
#endif
//Main application config file name
#ifndef APP_CONFIG_FILE
	#define APP_CONFIG_FILE "_app.conf" // leading point for security reasons :)
#endif

//class ApplicationConfig
//{
//public:
//	void load();
//	void save();
//	// Configuration parameters goes next
//	uint32_t loopInterval = 0; // loop interval in ms
//	String updateURL; // Firmware update URL
//	uint8_t timeZone = 2;
//private:
//	String _fileName = APP_CONFIG_FILE;
//};

class ApplicationClass
{
public:
	ApplicationClass() {};
	virtual ~ApplicationClass() {};
	virtual void init(); // Application initialization
	virtual void start(); // Application main-loop start/restart
	virtual void stop(); // Application main-loop stop
//	ApplicationConfig Config; // Instance of Configuration for application
	HttpServer webServer; // instance of web server for application
	void startWebServer(); // Start Application WebServer
	RbootHttpUpdater* otaUpdater = 0;
	static const uint8_t sysId = 1;
	void wsAddBinSetter(uint8_t sysId, WebsocketBinaryDelegate wsBinSetterDelegate);
	void wsAddBinGetter(uint8_t sysId, WebsocketBinaryDelegate wsBinGetterDelegate);
	virtual void userSTAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway) {}; // Runs when Station got ip from access-point
	void OtaUpdate_CallBack(RbootHttpUpdater& client, bool result);
	void OtaUpdate();
	void Switch();
protected:
	virtual void _loop(); // Application main loop function goes here
	void _initialWifiConfig(); // Doing initial configuration of both Station and AccessPoint
	void _STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason); // Runs when Station disconnects
	void _STAConnect(const String& ssid, MacAddress bssid, uint8_t channel); // Runs when Station connects
	void _STAAuthModeChange(WifiAuthMode oldMode, WifiAuthMode newMode); // Runs when Station auth mode changes
	void _STAGotIP(IpAddress ip, IpAddress netmask, IpAddress gateway); // Runs when Station got ip from access-point
	void _STAReconnect();
	void _httpOnFile(HttpRequest &request, HttpResponse &response);
	void _httpOnIndex(HttpRequest &request, HttpResponse &response);
	void _httpOnIp(HttpRequest &request, HttpResponse &response);
	void _httpOnConfiguration(HttpRequest &request, HttpResponse &response);
	void _httpOnConfigurationJson(HttpRequest &request, HttpResponse &response);
	void _httpOnUpdate(HttpRequest &request, HttpResponse &response);
	void _handleWifiConfig(JsonObject& root);
	void wsConnected(WebsocketConnection& socket);
	void wsDisconnected(WebsocketConnection& socket);
	void wsMessageReceived(WebsocketConnection& socket, const String& message);
	void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size);
	virtual void wsBinSetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	virtual void wsBinGetter(WebsocketConnection& socket, uint8_t* data, size_t size);
	uint32_t _counter = 0; // Kind of heartbeat counter
	Timer _loopTimer; // Timer for serving loop
	Timer _reconnectTimer; // Timer for STA reconnect routine
	uint8_t _webServerStarted = false;

	HashMap<uint8_t,WebsocketBinaryDelegate> _wsBinSetters;
	HashMap<uint8_t,WebsocketBinaryDelegate> _wsBinGetters;
	// Configuration parameters goes next
	uint32_t loopInterval = 1000; // loop interval in ms
	String updateURL = "http://192.168.31.181/"; // Firmware update URL
	uint8_t timeZone = 2;
	//Binary configuration file name
	String _fileName = APP_CONFIG_FILE;
	WebsocketResource* _wsResource;
	void loadConfig();
	void saveConfig();
	virtual void _loadAppConfig(file_t file) {}; //override this in child class to load additional config values
	virtual void _saveAppConfig(file_t file) {}; //override this in child class to save additional config values
};
