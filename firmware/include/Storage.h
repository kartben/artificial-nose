#pragma once

#include <string>

class Storage
{
public:
	static std::string WiFiSSID;
	static std::string WiFiPassword;
	static std::string IdScope;
	static std::string RegistrationId;
	static std::string SymmetricKey;

public:
	static void Load();
	static void Save();
	static void Erase();

private:
	static int Init;

};
