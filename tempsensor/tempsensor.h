#pragma once
#include <SmingCore.h>
#include <JsonObjectStream.h>
#include <Libraries/OneWire/OneWire.h>

namespace TempsensorConnectionStatus
{
	const uint8_t CONNECTED = 1;
	const uint8_t DISCONNECTED = 0;
}

class TempSensor
{
public:
	TempSensor(uint16_t refresh = 4000);
	virtual ~TempSensor() {};
	void start();
	void stop();
	float getTemp() { return _temperature; };
	uint8_t isHealthy() { return _healthy; };
protected:
	float _temperature;
	uint16_t _refresh;
	uint8_t _healthy; // is tempsensor value good, correct, healthy, 1 - yes, 0 - no;
	Timer _refreshTimer;
	virtual void _temp_start();
};

class TempSensorOW : public TempSensor
{
public:
	TempSensorOW(OneWire &ds, uint16_t refresh = 4000, uint8_t tries = 5);
	virtual ~TempSensorOW() {};
private:
	virtual void _temp_start();
	void _temp_read();
	OneWire *_ds;
	uint8_t _tries;
	Timer _temp_readTimer;
//	uint8_t _temp_data[12];
	float _temp_accum;
	uint8_t _temp_counter;
};

class TempSensorHttp : public TempSensor
{
public:
	TempSensorHttp(String url, uint16_t refresh = 4000);
	virtual ~TempSensorHttp() {};
private:
	virtual void _temp_start();
	int _temp_read(HttpConnection& connection, bool successful);
	HttpClient _httpClient;
	String _url;
	uint8_t _connectionStatus; // Tempsensor connection status
};
