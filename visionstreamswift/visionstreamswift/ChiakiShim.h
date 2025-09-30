//
//  ChiakiShim.h
//  visionstreamswift
//
//  Temporary C shim to expose pairing to Swift. This is a placeholder and
//  should be replaced with real calls into Chiaki's regist API.
//

#ifndef CHIAKI_SHIM_H
#define CHIAKI_SHIM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns 0 on success, non-zero on failure.
// Fills out_regist_key_hex_padded with 32 hex chars and out_rp_key_hex with 32 hex chars.
int chiaki_regist_shim(const char *host,
                       int is_ps5,
                       const char *psn_online_id,
                       const char *psn_account_id_b64,
                       const char *pin,
                       char *out_regist_key_hex_padded,
                       size_t out_regist_key_hex_padded_len,
                       char *out_rp_key_hex,
                       size_t out_rp_key_hex_len);

#ifdef __cplusplus
}
#endif

#endif // CHIAKI_SHIM_H


