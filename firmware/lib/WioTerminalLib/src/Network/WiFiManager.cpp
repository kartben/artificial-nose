#include "Network/WiFiManager.h"
#include <rpcWiFiClientSecure.h>

void WiFiManager::Connect(const char* ssid, const char* password)
{
    Ssid_ = ssid;
    Password_ = password;
}

bool WiFiManager::IsConnected(bool reconnect)
{
    if (Ssid_.empty()) return false;

    if (WiFi.status() == WL_CONNECTED) return true;

    if (reconnect) WiFi.begin(Ssid_.c_str(), Password_.c_str());

    return false;
}
