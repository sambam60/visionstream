//
//  ChiakiBridge.swift
//  visionstreamswift
//
//  Swift-facing protocol to abstract Chiaki C API. We'll implement using
//  a modulemap/bridging header later.
//

import Foundation
import SwiftUI

enum ChiakiCodec: Int {
    case h265 = 1
    case h265HDR = 2
}

struct ChiakiVideoProfile {
    var width: Int
    var height: Int
    var maxFPS: Int
    var bitrateKbps: Int
    var codec: ChiakiCodec
}

struct ChiakiConnectSecrets {
    var rpRegistKeyHexPadded: String // 16 bytes, zero-padded, hex string
    var rpKeyHex: String             // 16 bytes, hex string
}

enum ChiakiEvent {
    case connected
    case loginPinRequest(pinIncorrect: Bool)
    case quit(reason: String)
}

protocol ChiakiSessionHandle {}

protocol ChiakiRegistering {
    func register(host: String,
                  isPS5: Bool,
                  psnOnlineId: String?,
                  psnAccountIdBase64: String?,
                  pin: String,
                  onComplete: @escaping (Result<ChiakiConnectSecrets, Error>) -> Void)
}

protocol ChiakiStreaming {
    func start(host: String,
               isPS5: Bool,
               secrets: ChiakiConnectSecrets,
               profile: ChiakiVideoProfile,
               events: @escaping (ChiakiEvent) -> Void) throws -> ChiakiSessionHandle
    func stop(_ handle: ChiakiSessionHandle)
}

// Placeholder implementation used until the C bridge is wired
final class ChiakiBridgeStub: ChiakiRegistering, ChiakiStreaming {
    func register(host: String, isPS5: Bool, psnOnlineId: String?, psnAccountIdBase64: String?, pin: String, onComplete: @escaping (Result<ChiakiConnectSecrets, Error>) -> Void) {
        // Call C shim for now
        var registKey = [CChar](repeating: 0, count: 33)
        var rpKey = [CChar](repeating: 0, count: 33)
        let rc = chiaki_regist_shim(host, isPS5 ? 1 : 0, psnOnlineId, psnAccountIdBase64, pin, &registKey, 33, &rpKey, 33)
        if rc == 0 {
            let r1 = String(cString: registKey)
            let r2 = String(cString: rpKey)
            onComplete(.success(.init(rpRegistKeyHexPadded: r1, rpKeyHex: r2)))
        } else {
            onComplete(.failure(NSError(domain: "ChiakiShim", code: Int(rc), userInfo: nil)))
        }
    }

    private final class Handle: ChiakiSessionHandle {}

    func start(host: String, isPS5: Bool, secrets: ChiakiConnectSecrets, profile: ChiakiVideoProfile, events: @escaping (ChiakiEvent) -> Void) throws -> ChiakiSessionHandle {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.2) { events(.connected) }
        return Handle()
    }

    func stop(_ handle: ChiakiSessionHandle) {}
}


