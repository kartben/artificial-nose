#include "Signature.h"
#include <string.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>

std::string GenerateEncryptedSignature(const std::string& symmetricKey, const std::vector<uint8_t>& signature)
{
    unsigned char base64DecodedSymmetricKey[symmetricKey.size() + 1];

    // Base64-decode device key
    // <-- symmetricKey
    // --> base64DecodedSymmetricKey
    size_t base64DecodedSymmetricKeyLength;
    if (mbedtls_base64_decode(base64DecodedSymmetricKey, sizeof(base64DecodedSymmetricKey), &base64DecodedSymmetricKeyLength, (unsigned char*)&symmetricKey[0], symmetricKey.size()) != 0) abort();
    if (base64DecodedSymmetricKeyLength == 0) abort();

    // SHA-256 encrypt
    // <-- base64DecodedSymmetricKey
    // <-- signature
    // --> encryptedSignature
    uint8_t encryptedSignature[32]; // SHA-256
    mbedtls_md_context_t ctx;
    const mbedtls_md_type_t mdType{ MBEDTLS_MD_SHA256 };
    if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(mdType), 1) != 0) abort();
    if (mbedtls_md_hmac_starts(&ctx, base64DecodedSymmetricKey, base64DecodedSymmetricKeyLength) != 0) abort();
    if (mbedtls_md_hmac_update(&ctx, &signature[0], signature.size()) != 0) abort();
    if (mbedtls_md_hmac_finish(&ctx, encryptedSignature) != 0) abort();

    // Base64 encode encrypted signature
    // <-- encryptedSignature
    // --> b64encHmacsha256Signature
    char b64encHmacsha256Signature[(size_t)(sizeof(encryptedSignature) * 1.5f) + 1];
    size_t b64encHmacsha256SignatureLength;
    if (mbedtls_base64_encode((unsigned char*)b64encHmacsha256Signature, sizeof(b64encHmacsha256Signature), &b64encHmacsha256SignatureLength, encryptedSignature, mbedtls_md_get_size(mbedtls_md_info_from_type(mdType))) != 0) abort();

    return std::string(b64encHmacsha256Signature, b64encHmacsha256SignatureLength);
}

std::string ComputeDerivedSymmetricKey(const std::string& masterKey, const std::string& registrationId)
{
    unsigned char base64DecodedMasterKey[masterKey.size() + 1];

    // Base64-decode device key
    // <-- masterKey
    // --> base64DecodedMasterKey
    size_t base64DecodedMasterKeyLength;
    if (mbedtls_base64_decode(base64DecodedMasterKey, sizeof(base64DecodedMasterKey), &base64DecodedMasterKeyLength, (unsigned char*)&masterKey[0], masterKey.size()) != 0) abort();
    if (base64DecodedMasterKeyLength == 0) abort();

    // SHA-256 encrypt
    // <-- base64DecodedMasterKey
    // <-- registrationId
    // --> derivedSymmetricKey
    uint8_t derivedSymmetricKey[32]; // SHA-256
    mbedtls_md_context_t ctx;
    const mbedtls_md_type_t mdType{ MBEDTLS_MD_SHA256 };
    if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(mdType), 1) != 0) abort();
    if (mbedtls_md_hmac_starts(&ctx, base64DecodedMasterKey, base64DecodedMasterKeyLength) != 0) abort();
    if (mbedtls_md_hmac_update(&ctx, (const unsigned char*)&registrationId[0], registrationId.size()) != 0) abort();
    if (mbedtls_md_hmac_finish(&ctx, derivedSymmetricKey) != 0) abort();

    // Base64 encode encrypted signature
    // <-- derivedSymmetricKey
    // --> b64encDerivedSymmetricKey
    char b64encDerivedSymmetricKey[(size_t)(sizeof(derivedSymmetricKey) * 1.5f) + 1];
    size_t b64encDerivedSymmetricKeyLength;
    if (mbedtls_base64_encode((unsigned char*)b64encDerivedSymmetricKey, sizeof(b64encDerivedSymmetricKey), &b64encDerivedSymmetricKeyLength, derivedSymmetricKey, mbedtls_md_get_size(mbedtls_md_info_from_type(mdType))) != 0) abort();

    return std::string(b64encDerivedSymmetricKey, b64encDerivedSymmetricKeyLength);
}
