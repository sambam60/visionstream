//
//  ChiakiShim.c
//  visionstreamswift
//
//  Placeholder pairing shim that returns deterministic dummy keys.
//  Replace with chiaki_regist_start integration.
//

#include "ChiakiShim.h"
#include <string.h>

int chiaki_regist_shim(const char *host,
                       int is_ps5,
                       const char *psn_online_id,
                       const char *psn_account_id_b64,
                       const char *pin,
                       char *out_regist_key_hex_padded,
                       size_t out_regist_key_hex_padded_len,
                       char *out_rp_key_hex,
                       size_t out_rp_key_hex_len)
{
    (void)host; (void)is_ps5; (void)psn_online_id; (void)psn_account_id_b64; (void)pin;
    if(out_regist_key_hex_padded_len < 33 || out_rp_key_hex_len < 33)
        return -1;
    memset(out_regist_key_hex_padded, '0', 32);
    out_regist_key_hex_padded[32] = '\0';
    memset(out_rp_key_hex, '1', 32);
    out_rp_key_hex[32] = '\0';
    return 0;
}


