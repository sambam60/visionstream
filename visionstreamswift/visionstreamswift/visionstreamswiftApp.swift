//
//  visionstreamswiftApp.swift
//  visionstreamswift
//
//  Created by Sam Smith on 30/09/2025.
//

import SwiftUI

@main
struct visionstreamswiftApp: App {

    @State private var appModel = AppModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(appModel)
        }
    }
}