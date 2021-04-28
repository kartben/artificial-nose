#pragma once

#include <string>

namespace ExtFlashLoader
{
	class QSPIFlash;
}

class Storage
{
private:
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

public:
	std::string WiFiSSID;
	std::string WiFiPassword;
	std::string IdScope;
	std::string RegistrationId;
	std::string SymmetricKey;

    Storage(ExtFlashLoader::QSPIFlash& flash);
	void Load();
	void Save();
	void Erase();

private:
	ExtFlashLoader::QSPIFlash& Flash_;

};
