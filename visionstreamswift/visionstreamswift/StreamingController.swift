//
//  StreamingController.swift
//  visionstreamswift
//
//  Temporary stub. Will bridge to Chiaki C API for pairing/streaming.
//

import Foundation

@MainActor
@Observable
final class StreamingController {
    private(set) var isStreaming: Bool = false

    func start(with configuration: AppConfiguration) {
        // TODO: Connect via chiaki_session using configuration
        isStreaming = true
    }

    func stop() {
        // TODO: Stop chiaki_session
        isStreaming = false
    }
}


