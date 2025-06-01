#include "CryptoUtils.h"
#include <mbedtls/md.h>
#include <string.h> // For strlen
#include <vector>
#include <iomanip>
#include <sstream>

// Include Arduino.h for Serial output if needed for error logging
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

std::string compute_hmac_sha256(const char* secret_key, const char* message) {
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        Serial.println("Error: Failed to get mbedtls_md_info_from_type for SHA256.");
        return "";
    }

    unsigned char hmac_output[32]; // SHA256 output is 32 bytes

    int ret = mbedtls_md_hmac(
        md_info,
        (const unsigned char*)secret_key,
        strlen(secret_key),
        (const unsigned char*)message,
        strlen(message),
        hmac_output
    );

    if (ret != 0) {
        Serial.print("Error: mbedtls_md_hmac failed with error code: ");
        Serial.println(ret);
        return "";
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < sizeof(hmac_output); ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(hmac_output[i]);
    }

    return oss.str();
}
