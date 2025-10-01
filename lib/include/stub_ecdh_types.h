// Stub types for OpenSSL ECDH when building without OpenSSL
#ifndef STUB_ECDH_TYPES_H
#define STUB_ECDH_TYPES_H

#include <stdint.h>

// Opaque stub structures matching OpenSSL's EC types
struct ec_group_st {
    uint8_t dummy[256];
};

struct ec_key_st {
    uint8_t dummy[512];
    uint8_t local_pub_key[65]; // For storing public key
};

#endif

