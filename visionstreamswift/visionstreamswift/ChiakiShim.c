//
//  ChiakiShim.c
//  visionstreamswift
//
//  Pairing shim. Real Chiaki integration is behind USE_CHIAKI_SHIM_REAL.
//

#include "ChiakiShim.h"
#include "ChiakiShimStreaming.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef USE_CHIAKI_SHIM_REAL

// Include os_log for logging
#include <os/log.h>
#include <stdarg.h>
#include <stdio.h>

// Helper function to log to Xcode console (usable throughout file)
static void log_stream(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    os_log(OS_LOG_DEFAULT, "%{public}s", buffer);
}
// Include all necessary Chiaki headers from libchiaki.a
#include <chiaki/common.h>
#include <chiaki/ecdh.h>
#include <chiaki/rpcrypt.h>
#include <chiaki/regist.h>
#include <chiaki/session.h>
#include <chiaki/base64.h>
#include <stdlib.h>
#include <string.h>

CHIAKI_EXPORT const char *chiaki_error_string(ChiakiErrorCode code)
{
    switch(code)
    {
        case CHIAKI_ERR_SUCCESS: return "Success";
        case CHIAKI_ERR_PARSE_ADDR: return "Failed to parse host address";
        case CHIAKI_ERR_THREAD: return "Thread error";
        case CHIAKI_ERR_MEMORY: return "Memory error";
        case CHIAKI_ERR_NETWORK: return "Network error";
        case CHIAKI_ERR_CONNECTION_REFUSED: return "Connection Refused";
        case CHIAKI_ERR_HOST_DOWN: return "Host is down";
        case CHIAKI_ERR_HOST_UNREACH: return "No route to host";
        case CHIAKI_ERR_DISCONNECTED: return "Disconnected";
        case CHIAKI_ERR_INVALID_DATA: return "Invalid data";
        case CHIAKI_ERR_BUF_TOO_SMALL: return "Buffer too small";
        case CHIAKI_ERR_MUTEX_LOCKED: return "Mutex is locked";
        case CHIAKI_ERR_CANCELED: return "Canceled";
        case CHIAKI_ERR_TIMEOUT: return "Timeout";
        case CHIAKI_ERR_INVALID_RESPONSE: return "Invalid Response";
        case CHIAKI_ERR_INVALID_MAC: return "Invalid MAC";
        case CHIAKI_ERR_UNINITIALIZED: return "Uninitialized";
        case CHIAKI_ERR_FEC_FAILED: return "FEC failed";
        case CHIAKI_ERR_VERSION_MISMATCH: return "Version mismatch";
        default: return "Unknown";
    }
}

CHIAKI_EXPORT void *chiaki_aligned_alloc(size_t alignment, size_t size)
{
#if defined(_WIN32)
    return _aligned_malloc(size, alignment);
#elif __APPLE__ || __ANDROID__
    void *r = NULL;
    if(posix_memalign(&r, alignment, size) == 0)
        return r;
    return NULL;
#else
    return aligned_alloc(alignment, size);
#endif
}

CHIAKI_EXPORT void chiaki_aligned_free(void *ptr)
{
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_lib_init()
{
    // Skip galois/WSA init for pairing-only
    return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT const char *chiaki_codec_name(ChiakiCodec codec)
{
    switch(codec)
    {
        case CHIAKI_CODEC_H264: return "H264";
        case CHIAKI_CODEC_H265: return "H265";
        case CHIAKI_CODEC_H265_HDR: return "H265/HDR";
        default: return "unknown";
    }
}
// All crypto (ECDH/RPCrypt), RNG, and core functions are provided by libchiaki.a
// Do not include or reimplement any lib/src/*.c here.

// ========== Note: Full streaming implementation now lives in libchiaki.a ==========
// We just need to call chiaki_session_* functions from Swift wrapper below

typedef struct {
    int finished;
    int ok;
    ChiakiRegisteredHost host;
} ShimCtx;

// Log callback for pairing
static void pairing_log_cb(ChiakiLogLevel level, const char *msg, void *user) {
    (void)level; (void)user;
    log_stream("[ChiakiRegist] %s", msg);
}

static void shim_cb(ChiakiRegistEvent *event, void *user)
{
    ShimCtx *ctx = (ShimCtx *)user;
    if(!ctx) {
        log_stream("[Pairing] ERROR: ctx is null in callback!");
        return;
    }
    
    log_stream("[Pairing] Regist event type: %d", event->type);
    
    if(event->type == CHIAKI_REGIST_EVENT_TYPE_FINISHED_SUCCESS && event->registered_host) {
        log_stream("[Pairing] ✅ Registration finished successfully!");
        ctx->ok = 1;
        ctx->host = *event->registered_host;
    } else if(event->type == CHIAKI_REGIST_EVENT_TYPE_FINISHED_FAILED) {
        log_stream("[Pairing] ❌ Registration FAILED");
        ctx->ok = 0;
    } else if(event->type == CHIAKI_REGIST_EVENT_TYPE_FINISHED_CANCELED) {
        log_stream("[Pairing] ⚠️ Registration CANCELED");
        ctx->ok = 0;
    } else {
        log_stream("[Pairing] Event type %d, ok = 0", event->type);
        ctx->ok = 0;
    }
    ctx->finished = 1;
}

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
    log_stream("[Pairing] chiaki_regist_shim called: host=%s, ps5=%d", host, is_ps5);
    
    if(!host || !pin) {
        log_stream("[Pairing] ❌ ERROR: host or pin is null");
        return -1;
    }
    if(out_regist_key_hex_padded_len < 33 || out_rp_key_hex_len < 33) {
        log_stream("[Pairing] ❌ ERROR: output buffers too small");
        return -2;
    }

    chiaki_lib_init();

    ChiakiLog log;
    chiaki_log_init(&log, CHIAKI_LOG_ALL, pairing_log_cb, NULL);
    log_stream("[Pairing] Chiaki log initialized");

    ChiakiRegist regist;
    memset(&regist, 0, sizeof(regist));

    ChiakiRegistInfo info;
    memset(&info, 0, sizeof(info));
    info.host = host;
    info.broadcast = false;
    info.target = is_ps5 ? CHIAKI_TARGET_PS5_1 : CHIAKI_TARGET_PS4_10;
    
    log_stream("[Pairing] ChiakiRegistInfo setup:");
    log_stream("   info.host = %s (ptr=%p)", info.host, info.host);
    log_stream("   info.target = %d", info.target);
    log_stream("   info.broadcast = %d", info.broadcast);
    
    // For PS5, prefer Account ID over Online ID (PS5 ignores Online ID in payload format)
    if(is_ps5 && psn_account_id_b64 && *psn_account_id_b64) {
        log_stream("[Pairing] PS5 detected - using PSN Account ID");
        log_stream("[Pairing] Base64 input: %s", psn_account_id_b64);
        size_t out_sz = CHIAKI_PSN_ACCOUNT_ID_SIZE;
        ChiakiErrorCode derr = chiaki_base64_decode(psn_account_id_b64, strlen(psn_account_id_b64), info.psn_account_id, &out_sz);
        if(derr != CHIAKI_ERR_SUCCESS || out_sz != CHIAKI_PSN_ACCOUNT_ID_SIZE) {
            log_stream("[Pairing] ❌ ERROR: Failed to decode Account ID, err=%s, size=%zu (expected %d)", 
                      chiaki_error_string(derr), out_sz, CHIAKI_PSN_ACCOUNT_ID_SIZE);
            return -3;
        }
        log_stream("[Pairing] Account ID decoded successfully (%zu bytes)", out_sz);
        log_stream("[Pairing] Account ID bytes: %02x %02x %02x %02x %02x %02x %02x %02x",
                  info.psn_account_id[0], info.psn_account_id[1], info.psn_account_id[2], info.psn_account_id[3],
                  info.psn_account_id[4], info.psn_account_id[5], info.psn_account_id[6], info.psn_account_id[7]);
    } else if(psn_online_id && *psn_online_id) {
        log_stream("[Pairing] Using PSN Online ID: %s", psn_online_id);
        info.psn_online_id = psn_online_id;
    } else {
        log_stream("[Pairing] ❌ ERROR: Neither PSN Online ID nor Account ID provided");
        return -4;
    }
    info.pin = (uint32_t)atoi(pin);
    log_stream("[Pairing] PIN: %u (from string '%s')", info.pin, pin);

    ShimCtx ctx = {0};
    log_stream("[Pairing] Calling chiaki_regist_start...");
    ChiakiErrorCode err = chiaki_regist_start(&regist, &log, &info, shim_cb, &ctx);
    if(err != CHIAKI_ERR_SUCCESS) {
        log_stream("[Pairing] ❌ ERROR: chiaki_regist_start failed: %s", chiaki_error_string(err));
        return -5;
    }
    log_stream("[Pairing] Registration started, waiting for callback...");

    // Wait for registration to complete (with timeout)
    // Check every 100ms for up to 10 seconds
    for(int i = 0; i < 100 && !ctx.finished; ++i) {
        usleep(100000); // 100ms
    }

    log_stream("[Pairing] Wait loop finished. finished=%d, ok=%d", ctx.finished, ctx.ok);
    
    chiaki_regist_stop(&regist);
    chiaki_regist_fini(&regist);

    if(!ctx.finished || !ctx.ok) {
        log_stream("[Pairing] ❌ ERROR: Registration did not complete successfully (finished=%d, ok=%d)", 
                  ctx.finished, ctx.ok);
        return -6;
    }
    
    log_stream("[Pairing] ✅ Registration completed successfully!");

    // Hex encode rp_regist_key (16 bytes) and rp_key (16 bytes)
    static const char *hex = "0123456789abcdef";
    char buf1[33];
    char buf2[33];
    for(int i = 0; i < 16; ++i) {
        unsigned char b = (unsigned char)ctx.host.rp_regist_key[i];
        buf1[i*2] = hex[(b >> 4) & 0xF];
        buf1[i*2+1] = hex[b & 0xF];
        unsigned char k = (unsigned char)ctx.host.rp_key[i];
        buf2[i*2] = hex[(k >> 4) & 0xF];
        buf2[i*2+1] = hex[k & 0xF];
    }
    buf1[32] = '\0';
    buf2[32] = '\0';
    strncpy(out_regist_key_hex_padded, buf1, out_regist_key_hex_padded_len);
    strncpy(out_rp_key_hex, buf2, out_rp_key_hex_len);
    return 0;
}
#else
// Fallback stub implementation to avoid linker errors when Chiaki is not linked.
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
    // Deterministic dummy keys so UI can flow; replace by defining USE_CHIAKI_SHIM_REAL
    memset(out_regist_key_hex_padded, '0', 32);
    out_regist_key_hex_padded[32] = '\0';
    memset(out_rp_key_hex, '1', 32);
    out_rp_key_hex[32] = '\0';
    return 0;
}
#endif

// ========== Streaming Shim (Full Implementation using libchiaki.a) ==========

#include <chiaki/session.h>
// Note: log_stream is now defined at top of file

typedef struct {
    ChiakiSession session;
    ChiakiLog log;
    ChiakiVideoSampleCallbackSwift video_cb;
    ChiakiAudioSampleCallbackSwift audio_cb;
    ChiakiEventCallbackSwift event_cb;
    void *user;
} ChiakiSessionShim;

static void streaming_log_cb(ChiakiLogLevel level, const char *msg, void *user) {
    (void)user;
    const char *level_str = "?";
    switch(level) {
        case CHIAKI_LOG_DEBUG: level_str = "DEBUG"; break;
        case CHIAKI_LOG_VERBOSE: level_str = "VERBOSE"; break;
        case CHIAKI_LOG_INFO: level_str = "INFO"; break;
        case CHIAKI_LOG_WARNING: level_str = "WARNING"; break;
        case CHIAKI_LOG_ERROR: level_str = "ERROR"; break;
        default: break;
    }
    log_stream("[ChiakiStream-%s] %s", level_str, msg);
}

// Video callback adapter
static bool video_sample_cb_adapter(uint8_t *buf, size_t buf_size, void *user) {
    ChiakiSessionShim *shim = (ChiakiSessionShim *)user;
    if (shim->video_cb) {
        shim->video_cb(buf, buf_size, shim->user);
    }
    return true; // Always accept frames
}

// Event callback adapter
static void event_cb_adapter(ChiakiEvent *event, void *user) {
    ChiakiSessionShim *shim = (ChiakiSessionShim *)user;
    
    const char *event_type = "UNKNOWN";
    const char *event_reason = "";
    
    switch(event->type) {
        case CHIAKI_EVENT_CONNECTED:
            event_type = "CONNECTED";
            log_stream("[Streaming] *** PS5 CONNECTED! ***");
            break;
        case CHIAKI_EVENT_LOGIN_PIN_REQUEST:
            event_type = "LOGIN_PIN_REQUEST";
            log_stream("[Streaming] *** LOGIN PIN REQUESTED ***");
            break;
        case CHIAKI_EVENT_QUIT:
            event_type = "QUIT";
            event_reason = event->quit.reason_str ? event->quit.reason_str : chiaki_quit_reason_string(event->quit.reason);
            log_stream("[Streaming] *** SESSION QUIT: %s (reason: %d) ***", event_reason, event->quit.reason);
            break;
        case CHIAKI_EVENT_KEYBOARD_OPEN:
            event_type = "KEYBOARD_OPEN";
            log_stream("[Streaming] *** KEYBOARD OPEN ***");
            break;
        case CHIAKI_EVENT_RUMBLE:
            event_type = "RUMBLE";
            break;
        default:
            log_stream("[Streaming] *** Unknown event type: %d ***", event->type);
            break;
    }
    
    if (shim->event_cb) {
        shim->event_cb(event_type, event_reason, shim->user);
    }
}

ChiakiSessionHandle chiaki_session_shim_start(
    const char *host,
    int is_ps5,
    const char *regist_key_b64,
    const char *rp_key_b64,
    int resolution_width,
    int resolution_height,
    int max_fps,
    int bitrate_kbps,
    int codec,
    ChiakiVideoSampleCallbackSwift video_cb,
    ChiakiAudioSampleCallbackSwift audio_cb,
    ChiakiEventCallbackSwift event_cb,
    void *user)
{
    ChiakiSessionShim *shim = calloc(1, sizeof(ChiakiSessionShim));
    if (!shim) return NULL;
    
    shim->video_cb = video_cb;
    shim->audio_cb = audio_cb;
    shim->event_cb = event_cb;
    shim->user = user;
    
    // Initialize log (enable ALL including verbose for debugging)
    chiaki_log_init(&shim->log, CHIAKI_LOG_ALL, streaming_log_cb, NULL);
    log_stream("[Streaming] ========== SESSION START ==========");
    log_stream("[Streaming] Host: %s, PS5: %d", host, is_ps5);
    log_stream("[Streaming] Resolution: %dx%d@%dfps, Bitrate: %dkbps, Codec: %d", 
           resolution_width, resolution_height, max_fps, bitrate_kbps, codec);
    
    // Decode base64 keys
    uint8_t regist_key[CHIAKI_SESSION_AUTH_SIZE];
    memset(regist_key, 0, sizeof(regist_key));
    size_t regist_key_size = sizeof(regist_key);
    ChiakiErrorCode err = chiaki_base64_decode(regist_key_b64, strlen(regist_key_b64), regist_key, &regist_key_size);
    if (err != CHIAKI_ERR_SUCCESS) {
        log_stream("[Streaming] ERROR: Failed to decode regist_key: %s", chiaki_error_string(err));
        free(shim);
        return NULL;
    }
    log_stream("[Streaming] Decoded regist_key (%zu bytes)", regist_key_size);
    
    uint8_t rp_key[CHIAKI_RPCRYPT_KEY_SIZE];
    size_t rp_key_size = sizeof(rp_key);
    err = chiaki_base64_decode(rp_key_b64, strlen(rp_key_b64), rp_key, &rp_key_size);
    if (err != CHIAKI_ERR_SUCCESS) {
        log_stream("[Streaming] ERROR: Failed to decode rp_key: %s", chiaki_error_string(err));
        free(shim);
        return NULL;
    }
    log_stream("[Streaming] Decoded rp_key (%zu bytes)", rp_key_size);
    
    // Setup connect info
    ChiakiConnectInfo connect_info = {0};
    connect_info.ps5 = is_ps5 ? true : false;
    connect_info.host = host;
    memcpy(connect_info.regist_key, regist_key, CHIAKI_SESSION_AUTH_SIZE);
    memcpy(connect_info.morning, rp_key, sizeof(connect_info.morning));
    
    connect_info.video_profile.width = resolution_width;
    connect_info.video_profile.height = resolution_height;
    connect_info.video_profile.max_fps = max_fps;
    connect_info.video_profile.bitrate = bitrate_kbps * 1000; // Convert kbps to bps
    
    // Map codec parameter
    if (codec == 0) {
        connect_info.video_profile.codec = CHIAKI_CODEC_H264;
    } else if (codec == 1) {
        connect_info.video_profile.codec = CHIAKI_CODEC_H265;
    } else {
        connect_info.video_profile.codec = CHIAKI_CODEC_H265_HDR;
    }
    
    connect_info.video_profile_auto_downgrade = true;
    connect_info.enable_keyboard = false;
    
    // Initialize session
    log_stream("[Streaming] Calling chiaki_session_init...");
    err = chiaki_session_init(&shim->session, &connect_info, &shim->log);
    if (err != CHIAKI_ERR_SUCCESS) {
        log_stream("[Streaming] ERROR: chiaki_session_init failed: %s", chiaki_error_string(err));
        free(shim);
        return NULL;
    }
    log_stream("[Streaming] Session initialized successfully");
    
    // Set callbacks
    chiaki_session_set_event_cb(&shim->session, event_cb_adapter, shim);
    chiaki_session_set_video_sample_cb(&shim->session, video_sample_cb_adapter, shim);
    
    // Audio sink (stub for now, we'll decode audio in Swift later)
    ChiakiAudioSink audio_sink = {0};
    audio_sink.header_cb = NULL;
    audio_sink.frame_cb = NULL;
    audio_sink.user = shim;
    chiaki_session_set_audio_sink(&shim->session, &audio_sink);
    
    // Start session
    log_stream("[Streaming] Calling chiaki_session_start...");
    err = chiaki_session_start(&shim->session);
    if (err != CHIAKI_ERR_SUCCESS) {
        log_stream("[Streaming] ERROR: chiaki_session_start failed: %s", chiaki_error_string(err));
        chiaki_session_fini(&shim->session);
        free(shim);
        return NULL;
    }
    
    log_stream("[Streaming] ========== SESSION STARTED ==========");
    log_stream("[Streaming] Session thread is now running, waiting for PS5 connection...");
    return (ChiakiSessionHandle)shim;
}

void chiaki_session_shim_stop(ChiakiSessionHandle handle) {
    if (!handle) return;
    
    log_stream("[Streaming] Stopping session...");
    ChiakiSessionShim *shim = (ChiakiSessionShim *)handle;
    chiaki_session_stop(&shim->session);
    chiaki_session_join(&shim->session);
    chiaki_session_fini(&shim->session);
    free(shim);
    log_stream("[Streaming] Session stopped");
}
