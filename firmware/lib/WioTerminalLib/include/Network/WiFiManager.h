#pragma once

#include <string>

class WiFiManager
{
public:
	void Connect(const char* ssid, const char* password);
	bool IsConnected(bool reconnect = true);

private:
	std::string Ssid_;
	std::string Password_;

};
