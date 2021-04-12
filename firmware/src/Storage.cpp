#include <Arduino.h>
#include "Storage.h"
#include <MsgPack.h>
#include <ExtFlashLoader.h>

static auto FlashStartAddress = reinterpret_cast<const uint8_t* const>(0x04000000);

static ExtFlashLoader::QSPIFlash Flash;

std::string Storage::WiFiSSID;
std::string Storage::WiFiPassword;
std::string Storage::IdScope;
std::string Storage::RegistrationId;
std::string Storage::SymmetricKey;

int Storage::Init = [] {
	Flash.initialize();    
	Flash.reset();
	Flash.enterToMemoryMode();

	WiFiSSID.clear();
	WiFiPassword.clear();
	IdScope.clear();
	RegistrationId.clear();
	SymmetricKey.clear();
	
	return 0;
}();

void Storage::Load()
{
	if (memcmp(&FlashStartAddress[0], "AZ01", 4) != 0)
	{
		Storage::WiFiSSID.clear();
		Storage::WiFiPassword.clear();
		Storage::IdScope.clear();
		Storage::RegistrationId.clear();
		Storage::SymmetricKey.clear();
	}
	else
	{
		MsgPack::Unpacker unpacker;
		unpacker.feed(&FlashStartAddress[8], *(const uint32_t*)&FlashStartAddress[4]);

		MsgPack::str_t str[5];
		unpacker.deserialize(str[0], str[1], str[2], str[3], str[4]);

		Storage::WiFiSSID = str[0].c_str();
		Storage::WiFiPassword = str[1].c_str();
		Storage::IdScope = str[2].c_str();
		Storage::RegistrationId = str[3].c_str();
		Storage::SymmetricKey = str[4].c_str();
	}
}

void Storage::Save()
{
    MsgPack::Packer packer;
	{
		MsgPack::str_t str[5];
		str[0] = Storage::WiFiSSID.c_str();
		str[1] = Storage::WiFiPassword.c_str();
		str[2] = Storage::IdScope.c_str();
		str[3] = Storage::RegistrationId.c_str();
		str[4] = Storage::SymmetricKey.c_str();
		packer.serialize(str[0], str[1], str[2], str[3], str[4]);
	}

	std::vector<uint8_t> buf(4 + 4 + packer.size());
	memcpy(&buf[0], "AZ01", 4);
	*(uint32_t*)&buf[4] = packer.size();
	memcpy(&buf[8], packer.data(), packer.size());

	ExtFlashLoader::writeExternalFlash(Flash, 0, &buf[0], buf.size(), [](std::size_t bytes_processed, std::size_t bytes_total, bool verifying) { return true; });
}

void Storage::Erase()
{
	Flash.exitFromMemoryMode();
	Flash.writeEnable();
	Flash.eraseSector(0);
	Flash.waitProgram(0);
	Flash.enterToMemoryMode();
}
