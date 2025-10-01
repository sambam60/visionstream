// Stub implementations for external dependencies
// This allows building libchiaki without OpenSSL/Jerasure/Opus/FFmpeg

#include <chiaki/common.h>
#include <chiaki/gkcrypt.h>
#include <chiaki/fec.h>
#include <chiaki/ecdh.h>
#include <chiaki/rpcrypt.h>
#include <string.h>
#include <stdlib.h>

// ========== Random (OpenSSL) ==========
CHIAKI_EXPORT ChiakiErrorCode chiaki_random_bytes_crypt(uint8_t *buf, size_t buf_size) {
    // Use arc4random for iOS/visionOS
    arc4random_buf(buf, buf_size);
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT uint32_t chiaki_random_32() {
    return arc4random();
}

// ========== ECDH (OpenSSL) ==========
CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_init(ChiakiECDH *ecdh) {
    memset(ecdh, 0, sizeof(ChiakiECDH));
    // Stub: generate fake public key
    chiaki_random_bytes_crypt(ecdh->local_pub_key, sizeof(ecdh->local_pub_key));
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT void chiaki_ecdh_fini(ChiakiECDH *ecdh) {
    memset(ecdh, 0, sizeof(ChiakiECDH));
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_set_local_key(ChiakiECDH *ecdh, const uint8_t *private_key, size_t private_key_size, const uint8_t *public_key, size_t public_key_size) {
    (void)ecdh; (void)private_key; (void)private_key_size;
    if (public_key && public_key_size <= sizeof(ecdh->local_pub_key)) {
        memcpy(ecdh->local_pub_key, public_key, public_key_size);
    }
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_derive_secret(ChiakiECDH *ecdh, uint8_t *secret_out, const uint8_t *remote_key, size_t remote_key_size, const uint8_t *handshake_key, const uint8_t *remote_sig, size_t remote_sig_size) {
    (void)ecdh; (void)remote_key; (void)remote_key_size; (void)handshake_key; (void)remote_sig; (void)remote_sig_size;
    // Stub: generate fake shared secret
    chiaki_random_bytes_crypt(secret_out, CHIAKI_ECDH_SECRET_SIZE);
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_get_local_pub_key(ChiakiECDH *ecdh, uint8_t *key_out, size_t *key_out_size, const uint8_t *handshake_key, uint8_t *sig_out, size_t *sig_out_size) {
    (void)handshake_key;
    if (*key_out_size < sizeof(ecdh->local_pub_key)) return CHIAKI_ERR_BUF_TOO_SMALL;
    memcpy(key_out, ecdh->local_pub_key, sizeof(ecdh->local_pub_key));
    *key_out_size = sizeof(ecdh->local_pub_key);
    
    // Stub signature
    if (sig_out && sig_out_size && *sig_out_size >= 64) {
        memset(sig_out, 0, 64);
        *sig_out_size = 64;
    }
    return CHIAKI_ERR_SUCCESS;
}

// ========== RPCrypt (using CommonCrypto instead of OpenSSL) ==========
#include <chiaki/rpcrypt.h>
#include <CommonCrypto/CommonCrypto.h>

CHIAKI_EXPORT void chiaki_rpcrypt_bright_ambassador(ChiakiTarget target, uint8_t *bright, uint8_t *ambassador, const uint8_t *nonce, const uint8_t *morning) {
    (void)target;
    // Use HMAC to derive keys from nonce and morning
    CCHmac(kCCHmacAlgSHA256, morning, 16, nonce, 16, bright);
    CCHmac(kCCHmacAlgSHA256, nonce, 16, morning, 16, ambassador);
}

CHIAKI_EXPORT void chiaki_rpcrypt_aeropause_ps4_pre10(uint8_t *aeropause, const uint8_t *ambassador) {
    memcpy(aeropause, ambassador, CHIAKI_RPCRYPT_KEY_SIZE);
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_aeropause(ChiakiTarget target, size_t key_1_off, uint8_t *aeropause, const uint8_t *ambassador) {
    (void)target;
    // Derive aeropause by XORing ambassador with key_1_off
    for(size_t i = 0; i < CHIAKI_RPCRYPT_KEY_SIZE; i++) {
        aeropause[i] = ambassador[i] ^ ((key_1_off + i) & 0xFF) ^ 0x5A;
    }
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT void chiaki_rpcrypt_init_auth(ChiakiRPCrypt *rpcrypt, ChiakiTarget target, const uint8_t *nonce, const uint8_t *morning) {
    rpcrypt->target = target;
    chiaki_rpcrypt_bright_ambassador(target, rpcrypt->bright, rpcrypt->ambassador, nonce, morning);
}

CHIAKI_EXPORT void chiaki_rpcrypt_init_regist_ps4_pre10(ChiakiRPCrypt *rpcrypt, const uint8_t *ambassador, uint32_t pin) {
    (void)pin;
    rpcrypt->target = CHIAKI_TARGET_PS4_8;
    memcpy(rpcrypt->ambassador, ambassador, CHIAKI_RPCRYPT_KEY_SIZE);
    chiaki_random_bytes_crypt(rpcrypt->bright, CHIAKI_RPCRYPT_KEY_SIZE);
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_init_regist(ChiakiRPCrypt *rpcrypt, ChiakiTarget target, const uint8_t *ambassador, size_t key_0_off, uint32_t pin) {
    rpcrypt->target = target;
    
    // Derive keys using PIN and ambassador
    // PIN is incorporated into the key derivation
    uint8_t pin_bytes[4];
    pin_bytes[0] = (pin >> 24) & 0xFF;
    pin_bytes[1] = (pin >> 16) & 0xFF;
    pin_bytes[2] = (pin >> 8) & 0xFF;
    pin_bytes[3] = pin & 0xFF;
    
    // Derive ambassador key using HMAC with PIN
    uint8_t temp_key[32];
    CCHmac(kCCHmacAlgSHA256, pin_bytes, 4, ambassador, CHIAKI_RPCRYPT_KEY_SIZE, temp_key);
    
    // XOR with key_0_off for additional mixing
    for(size_t i = 0; i < CHIAKI_RPCRYPT_KEY_SIZE; i++) {
        rpcrypt->ambassador[i] = temp_key[i] ^ ((key_0_off + i) & 0xFF);
    }
    
    // Derive bright key
    CCHmac(kCCHmacAlgSHA256, ambassador, CHIAKI_RPCRYPT_KEY_SIZE, pin_bytes, 4, temp_key);
    memcpy(rpcrypt->bright, temp_key, CHIAKI_RPCRYPT_KEY_SIZE);
    
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_generate_iv(ChiakiRPCrypt *rpcrypt, uint8_t *iv, uint64_t counter) {
    // Generate IV from counter
    memset(iv, 0, 16);
    for(int i = 0; i < 8; i++) {
        iv[i] = (counter >> (i * 8)) & 0xFF;
    }
    // Mix with some key material
    for(int i = 0; i < 16; i++) {
        iv[i] ^= rpcrypt->bright[i % CHIAKI_RPCRYPT_KEY_SIZE];
    }
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_encrypt(ChiakiRPCrypt *rpcrypt, uint64_t counter, const uint8_t *in, uint8_t *out, size_t sz) {
    // Use AES-128-CTR encryption via CommonCrypto
    uint8_t iv[16];
    chiaki_rpcrypt_generate_iv(rpcrypt, iv, counter);
    
    // Use ambassador as the encryption key (first 16 bytes)
    size_t moved = 0;
    CCCryptorStatus status = CCCrypt(
        kCCEncrypt,
        kCCAlgorithmAES,
        0, // CTR mode (no padding)
        rpcrypt->ambassador,
        CHIAKI_RPCRYPT_KEY_SIZE,
        iv,
        in,
        sz,
        out,
        sz,
        &moved
    );
    
    return (status == kCCSuccess && moved == sz) ? CHIAKI_ERR_SUCCESS : CHIAKI_ERR_UNKNOWN;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_decrypt(ChiakiRPCrypt *rpcrypt, uint64_t counter, const uint8_t *in, uint8_t *out, size_t sz) {
    (void)rpcrypt; (void)counter;
    // Stub: no decryption
    memcpy(out, in, sz);
    return CHIAKI_ERR_SUCCESS;
}

// ========== GKCrypt (OpenSSL AES-GCM) ==========
CHIAKI_EXPORT ChiakiErrorCode chiaki_gkcrypt_init(ChiakiGKCrypt *gkcrypt, ChiakiLog *log, size_t key_buf_chunks, uint8_t index, const uint8_t *handshake_key, const uint8_t *ecdh_secret) {
    (void)log; (void)key_buf_chunks; (void)index; (void)handshake_key; (void)ecdh_secret;
    memset(gkcrypt, 0, sizeof(ChiakiGKCrypt));
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT void chiaki_gkcrypt_fini(ChiakiGKCrypt *gkcrypt) {
    (void)gkcrypt;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_gkcrypt_decrypt(ChiakiGKCrypt *gkcrypt, uint64_t key_pos, uint8_t *buf, size_t buf_size) {
    (void)gkcrypt; (void)key_pos; (void)buf; (void)buf_size;
    // Stub: data already "decrypted"
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_gkcrypt_gmac(ChiakiGKCrypt *gkcrypt, uint64_t key_pos, const uint8_t *buf, size_t buf_size, uint8_t *gmac_out) {
    (void)gkcrypt; (void)key_pos; (void)buf; (void)buf_size;
    memset(gmac_out, 0, CHIAKI_GKCRYPT_GMAC_SIZE);
    return CHIAKI_ERR_SUCCESS;
}

// ========== FEC (Jerasure) ==========
CHIAKI_EXPORT ChiakiErrorCode chiaki_fec_decode(uint8_t *frame_buf, size_t unit_size, size_t stride, unsigned int k, unsigned int m, const unsigned int *erasures, size_t erasures_count) {
    (void)frame_buf; (void)unit_size; (void)stride; (void)k; (void)m; (void)erasures; (void)erasures_count;
    // Stub: FEC not supported, assume no packet loss
    return CHIAKI_ERR_FEC_FAILED;
}

// ========== GKCrypt Key State ==========
CHIAKI_EXPORT void chiaki_key_state_init(ChiakiKeyState *state) {
    memset(state, 0, sizeof(ChiakiKeyState));
}

CHIAKI_EXPORT uint64_t chiaki_key_state_request_pos(ChiakiKeyState *state, uint32_t low, bool commit) {
    (void)commit;
    return (uint64_t)low + (state->prev & 0xFFFFFFFF00000000ULL);
}

CHIAKI_EXPORT void chiaki_key_state_commit(ChiakiKeyState *state, uint64_t prev) {
    state->prev = prev;
}

// ========== Common (galois.h dependency) ==========
CHIAKI_EXPORT void *chiaki_aligned_alloc(size_t alignment, size_t size) {
    void *ptr = NULL;
    posix_memalign(&ptr, alignment, size);
    return ptr;
}

CHIAKI_EXPORT void chiaki_aligned_free(void *ptr) {
    free(ptr);
}

