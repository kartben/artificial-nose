#pragma once

#include <vector>
#include <string>

std::string GenerateEncryptedSignature(const std::string& symmetricKey, const std::vector<uint8_t>& signature);
std::string ComputeDerivedSymmetricKey(const std::string& masterKey, const std::string& registrationId);
