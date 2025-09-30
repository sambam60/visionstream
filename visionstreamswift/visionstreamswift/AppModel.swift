//
//  AppModel.swift
//  visionstreamswift
//
//  Created by Sam Smith on 30/09/2025.
//

import SwiftUI

/// Maintains app-wide state
@MainActor
@Observable
class AppModel {
    let immersiveSpaceID = "ImmersiveSpace"
    enum ImmersiveSpaceState {
        case closed
        case inTransition
        case open
    }
    var immersiveSpaceState = ImmersiveSpaceState.closed

    // App configuration loaded from JSON or edited in-app
    var configuration: AppConfiguration = .default

    // Streaming controller stub (to be backed by Chiaki)
    var streamingController = StreamingController()
}
