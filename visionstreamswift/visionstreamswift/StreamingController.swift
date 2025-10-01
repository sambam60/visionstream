//
//  StreamingController.swift
//  visionstreamswift
//
//  Bridges Swift to Chiaki C session API for streaming.
//

import Foundation

@MainActor
@Observable
final class StreamingController {
    private(set) var isStreaming: Bool = false
    private var sessionHandle: UnsafeMutableRawPointer?
    
    // Video/audio buffers for rendering
    private(set) var latestVideoFrame: (buffer: UnsafeMutablePointer<UInt8>, size: Int)?
    private(set) var latestAudioFrame: (buffer: UnsafeMutablePointer<Int16>, samples: Int)?
    
    func start(with configuration: AppConfiguration) {
        print("🎮 [Swift] StreamingController.start() called")
        
        guard !isStreaming else {
            print("⚠️ [Swift] Already streaming, ignoring start request")
            return
        }
        
        guard let rpRegistKeyB64 = configuration.rpRegistKeyBase64,
              let rpKeyB64 = configuration.rpKeyBase64 else {
            print("❌ [Swift] Missing registration keys. Please pair first.")
            print("   rpRegistKeyBase64: \(configuration.rpRegistKeyBase64 == nil ? "nil" : "present")")
            print("   rpKeyBase64: \(configuration.rpKeyBase64 == nil ? "nil" : "present")")
            return
        }
        
        let host = configuration.host
        let isPS5 = configuration.isPS5 ? 1 : 0
        let width = Int32(configuration.resolution.width)
        let height = Int32(configuration.resolution.height)
        let fps = Int32(configuration.fps.rawValue)
        let bitrate = Int32(configuration.bitrateKbps)
        
        print("🎮 [Swift] Streaming config:")
        print("   Host: \(host)")
        print("   PS5: \(isPS5)")
        print("   Resolution: \(width)x\(height)@\(fps)fps")
        print("   Bitrate: \(bitrate)kbps")
        let codec: Int32 = {
            if configuration.hdrEnabled {
                return 2 // H265_HDR
            } else if configuration.resolution == .p1080 || configuration.resolution == .p720 {
                return 1 // H265
            } else {
                return 0 // H264
            }
        }()
        
        // Video callback: store frame for renderer
        let videoCb: ChiakiVideoSampleCallbackSwift = { buf, size, user in
            guard let user = user else { return }
            let controller = Unmanaged<StreamingController>.fromOpaque(user).takeUnretainedValue()
            Task { @MainActor in
                controller.latestVideoFrame = (buf!, Int(size))
            }
        }
        
        // Audio callback: store samples for audio engine
        let audioCb: ChiakiAudioSampleCallbackSwift = { buf, samples, user in
            guard let user = user else { return }
            let controller = Unmanaged<StreamingController>.fromOpaque(user).takeUnretainedValue()
            Task { @MainActor in
                controller.latestAudioFrame = (buf!, Int(samples))
            }
        }
        
        // Event callback: handle quit/errors
        let eventCb: ChiakiEventCallbackSwift = { eventType, eventReason, user in
            guard let user = user,
                  let eventTypeStr = eventType.map({ String(cString: $0) }),
                  let eventReasonStr = eventReason.map({ String(cString: $0) }) else { return }
            let controller = Unmanaged<StreamingController>.fromOpaque(user).takeUnretainedValue()
            Task { @MainActor in
                print("🎯 [Swift] Chiaki event: \(eventTypeStr) - \(eventReasonStr)")
                if eventTypeStr == "QUIT" {
                    print("🛑 [Swift] Session quit, stopping...")
                    controller.stop()
                }
            }
        }
        
        let userPtr = Unmanaged.passUnretained(self).toOpaque()
        
        print("📞 [Swift] Calling chiaki_session_shim_start...")
        sessionHandle = chiaki_session_shim_start(
            host,
            Int32(isPS5),
            rpRegistKeyB64,
            rpKeyB64,
            width,
            height,
            fps,
            bitrate,
            codec,
            videoCb,
            audioCb,
            eventCb,
            userPtr
        )
        
        if sessionHandle != nil {
            isStreaming = true
            print("✅ [Swift] Streaming started successfully!")
            print("   Session handle: \(String(describing: sessionHandle))")
        } else {
            print("❌ [Swift] Failed to start streaming session - handle is nil")
        }
    }

    func stop() {
        print("🛑 [Swift] StreamingController.stop() called")
        guard isStreaming, let handle = sessionHandle else {
            print("⚠️ [Swift] Not streaming or handle is nil, nothing to stop")
            return
        }
        print("📞 [Swift] Calling chiaki_session_shim_stop...")
        chiaki_session_shim_stop(handle)
        sessionHandle = nil
        isStreaming = false
        latestVideoFrame = nil
        latestAudioFrame = nil
        print("✅ [Swift] Streaming stopped")
    }
}


