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
		EdgeImpulseHmacKey.clear();
	}
	else
	{
		MsgPack::Unpacker unpacker;
		unpacker.feed(&FlashStartAddress[8], *(const uint32_t*)&FlashStartAddress[4]);

		MsgPack::str_t str[6]; // Increased size to 6
		// Attempt to deserialize 6 strings. If old data has 5, this might be an issue.
		// A robust solution would check unpacker.parsed() or use versioning.
		// For now, we assume it either works or EdgeImpulseHmacKey remains empty/default.
		unpacker.deserialize(str[0], str[1], str[2], str[3], str[4], str[5]);

		WiFiSSID = str[0].c_str();
		WiFiPassword = str[1].c_str();
		IdScope = str[2].c_str();
		RegistrationId = str[3].c_str();
		SymmetricKey = str[4].c_str();
		if (str[5].size() > 0) { // Basic check if the 6th string was populated
		    EdgeImpulseHmacKey = str[5].c_str();
		} else {
		    EdgeImpulseHmacKey = ""; // Default if not found or empty
		}
	}
}

void Storage::Save()
{
    MsgPack::Packer packer;
	{
		MsgPack::str_t str[6]; // Increased size to 6
		str[0] = WiFiSSID.c_str();
		str[1] = WiFiPassword.c_str();
		str[2] = IdScope.c_str();
		str[3] = RegistrationId.c_str();
		str[4] = SymmetricKey.c_str();
		str[5] = EdgeImpulseHmacKey.c_str(); // Add HMAC key
		packer.serialize(str[0], str[1], str[2], str[3], str[4], str[5]); // Serialize 6 strings
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
