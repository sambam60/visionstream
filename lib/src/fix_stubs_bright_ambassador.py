#!/usr/bin/env python3
"""
Patch stubs_ios.c to use PS5 table-based bright/ambassador derivation
"""
import re

# Read rpcrypt.c to extract PS5 tables
with open('/Users/samsmith/Desktop/visionstream/lib/src/rpcrypt.c', 'r') as f:
    rpcrypt_content = f.read()

# Extract keys_a_ps5 table data (the huge hex array)
match_a = re.search(r'static const uint8_t keys_a_ps5\[0x70 \* 0x20\] = \{(.*?)\};', rpcrypt_content, re.DOTALL)
keys_a_data = match_a.group(1).strip() if match_a else ""

# Extract keys_b_ps5 table data
match_b = re.search(r'static const uint8_t keys_b_ps5\[0x70 \* 0x20\] = \{(.*?)\};', rpcrypt_content, re.DOTALL)
keys_b_data = match_b.group(1).strip() if match_b else ""

# Read current stubs file
with open('/Users/samsmith/Desktop/visionstream/lib/src/stubs_ios.c', 'r') as f:
    stubs_content = f.read()

# Replace the bright_ambassador function
new_bright_amb = '''CHIAKI_EXPORT void chiaki_rpcrypt_bright_ambassador(ChiakiTarget target, uint8_t *bright, uint8_t *ambassador, const uint8_t *nonce, const uint8_t *morning) {
    // PS5 table-based derivation (from Chiaki rpcrypt.c lines 1496-1540)
    if(target < CHIAKI_TARGET_PS4_10) {
        // Fallback for PS4 < 10
        CCHmac(kCCHmacAlgSHA256, morning, 16, nonce, 16, bright);
        CCHmac(kCCHmacAlgSHA256, nonce, 16, morning, 16, ambassador);
        return;
    }
    
    const uint8_t *keys_a = (target == CHIAKI_TARGET_PS5_1) ? keys_a_ps5 : keys_a_ps5; // Only PS5 for now
    const uint8_t *keys_b = (target == CHIAKI_TARGET_PS5_1) ? keys_b_ps5 : keys_b_ps5;
    
    // Derive ambassador from nonce using keys_a
    const uint8_t *key = &keys_a[(nonce[0] >> 3) * 0x70];
    for(size_t i=0; i<CHIAKI_RPCRYPT_KEY_SIZE; i++) {
        uint8_t v = nonce[i];
        if(target == CHIAKI_TARGET_PS5_1) {
            v -= 0x2d;
            v -= i;
        } else {
            v += 0x36;
            v += i;
        }
        v ^= key[i];
        ambassador[i] = v;
    }
    
    // Derive bright from morning using keys_b
    key = &keys_b[(nonce[7] >> 3) * 0x70];
    if(target == CHIAKI_TARGET_PS5_1) {
        for(size_t i=0; i<CHIAKI_RPCRYPT_KEY_SIZE; i++) {
            uint8_t v = morning[i];
            v += 0x18;
            v += i;
            v ^= nonce[i];
            v ^= key[i];
            bright[i] = v;
        }
    } else {
        for(size_t i=0; i<CHIAKI_RPCRYPT_KEY_SIZE; i++) {
            uint8_t v = (key[i] ^ morning[i]);
            v += 0x21;
            v += i;
            v ^= nonce[i];
            bright[i] = v;
        }
    }
}'''

# Insert the PS5 tables before the bright_ambassador function
tables_section = f'''
// PS5 key tables extracted from Chiaki rpcrypt.c
static const uint8_t keys_a_ps5[0x70 * 0x20] = {{{keys_a_data}
}};

static const uint8_t keys_b_ps5[0x70 * 0x20] = {{{keys_b_data}
}};

'''

# Find where to insert tables (right before chiaki_rpcrypt_bright_ambassador)
pattern = r'(CHIAKI_EXPORT void chiaki_rpcrypt_bright_ambassador\(.*?\n\})'
replacement = tables_section + new_bright_amb

stubs_content = re.sub(pattern, replacement, stubs_content, flags=re.DOTALL, count=1)

# Write patched file
with open('/Users/samsmith/Desktop/visionstream/lib/src/stubs_ios.c', 'w') as f:
    f.write(stubs_content)

print("✅ Patched stubs_ios.c with PS5 table-based bright/ambassador")

