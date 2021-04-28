#include <Arduino.h>
#include "Storage.h"

#include <MsgPack.h>
#include <ExtFlashLoader.h>

static auto FlashStartAddress = reinterpret_cast<const uint8_t* const>(0x04000000);

Storage::Storage(ExtFlashLoader::QSPIFlash& flash) :
	Flash_(flash)
{
	Flash_.initialize();    
	Flash_.reset();
	Flash_.enterToMemoryMode();
}

void Storage::Load()
{
	if (memcmp(&FlashStartAddress[0], "AZ01", 4) != 0)
	{
		WiFiSSID.clear();
		WiFiPassword.clear();
		IdScope.clear();
		RegistrationId.clear();
		SymmetricKey.clear();
	}
	else
	{
		MsgPack::Unpacker unpacker;
		unpacker.feed(&FlashStartAddress[8], *(const uint32_t*)&FlashStartAddress[4]);

		MsgPack::str_t str[5];
		unpacker.deserialize(str[0], str[1], str[2], str[3], str[4]);

		WiFiSSID = str[0].c_str();
		WiFiPassword = str[1].c_str();
		IdScope = str[2].c_str();
		RegistrationId = str[3].c_str();
		SymmetricKey = str[4].c_str();
	}
}

void Storage::Save()
{
    MsgPack::Packer packer;
	{
		MsgPack::str_t str[5];
		str[0] = WiFiSSID.c_str();
		str[1] = WiFiPassword.c_str();
		str[2] = IdScope.c_str();
		str[3] = RegistrationId.c_str();
		str[4] = SymmetricKey.c_str();
		packer.serialize(str[0], str[1], str[2], str[3], str[4]);
	}

	std::vector<uint8_t> buf(4 + 4 + packer.size());
	memcpy(&buf[0], "AZ01", 4);
	*(uint32_t*)&buf[4] = packer.size();
	memcpy(&buf[8], packer.data(), packer.size());

	ExtFlashLoader::writeExternalFlash(Flash_, 0, &buf[0], buf.size(), [](std::size_t bytes_processed, std::size_t bytes_total, bool verifying) { return true; });
}

void Storage::Erase()
{
	Flash_.exitFromMemoryMode();
	Flash_.writeEnable();
	Flash_.eraseSector(0);
	Flash_.waitProgram(0);
	Flash_.enterToMemoryMode();
}
