#pragma once

#include <WiFiUdp.h>
#include <NTPClient.h>

class TimeManager
{
public:
    TimeManager();
    TimeManager(const TimeManager&) = delete;
    TimeManager& operator=(const TimeManager&) = delete;
    
    bool Update();
    unsigned long GetEpochTime() const;

private:
    WiFiUDP Udp_;
	NTPClient Client_;
    unsigned long CaptureTime_;
    unsigned long EpochTime_;

};
