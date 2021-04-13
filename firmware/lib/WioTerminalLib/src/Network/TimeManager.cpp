#include "Network/TimeManager.h"
#include <Arduino.h>

TimeManager::TimeManager() :
    Client_(Udp_)
{
}

bool TimeManager::Update()
{
    bool result = false;

	Client_.begin();
	if (Client_.forceUpdate())
    {
        CaptureTime_ = millis();
        EpochTime_ = Client_.getEpochTime();
        result = true;
    }
	Client_.end();

    return result;
}

unsigned long TimeManager::GetEpochTime() const
{
    return EpochTime_ + (millis() - CaptureTime_) / 1000;
}
