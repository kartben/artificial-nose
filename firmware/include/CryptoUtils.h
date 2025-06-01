#pragma once

#include <string>

/**
 * @brief Computes the HMAC SHA256 hash of a message using a secret key.
 *
 * @param secret_key The secret key for the HMAC operation.
 * @param message The message to hash.
 * @return std::string The computed HMAC SHA256 hash as a lowercase hexadecimal string.
 *         Returns an empty string if an error occurs during computation.
 */
std::string compute_hmac_sha256(const char* secret_key, const char* message);
