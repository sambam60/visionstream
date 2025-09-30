//
//  ChiakiShimStreaming.h
//  visionstreamswift
//
//  Minimal streaming shim to expose chiaki session start/stop to Swift.
//

#ifndef CHIAKI_SHIM_STREAMING_H
#define CHIAKI_SHIM_STREAMING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ChiakiSessionHandle;

// Video sample callback from C to Swift: buf contains H264/H265 NAL units
typedef void (*ChiakiVideoSampleCallbackSwift)(uint8_t *buf, size_t buf_size, void *user);

// Audio sample callback from C to Swift: buf contains decoded PCM int16 samples
typedef void (*ChiakiAudioSampleCallbackSwift)(int16_t *buf, size_t samples_count, void *user);

// Event callback for quit/errors
typedef void (*ChiakiEventCallbackSwift)(const char *event_type, const char *event_reason, void *user);

// Start a session: returns handle on success, NULL on failure
ChiakiSessionHandle chiaki_session_shim_start(
    const char *host,
    int is_ps5,
    const char *regist_key_b64,
    const char *rp_key_b64,
    int resolution_width,
    int resolution_height,
    int max_fps,
    int bitrate_kbps,
    int codec, // 0=H264, 1=H265, 2=H265_HDR
    ChiakiVideoSampleCallbackSwift video_cb,
    ChiakiAudioSampleCallbackSwift audio_cb,
    ChiakiEventCallbackSwift event_cb,
    void *user
);

// Stop a session
void chiaki_session_shim_stop(ChiakiSessionHandle handle);

#ifdef __cplusplus
}
#endif

#endif // CHIAKI_SHIM_STREAMING_H

