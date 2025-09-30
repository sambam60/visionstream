//
//  ContentView.swift
//  visionstreamswift
//
//  Created by Sam Smith on 30/09/2025.
//

import SwiftUI
import RealityKit
import RealityKitContent
import UniformTypeIdentifiers

struct ContentView: View {
    @Environment(AppModel.self) private var appModel
    @State private var isImportingConfig = false
    @State private var isExportingConfig = false
    @State private var showSettings = false
    @State private var showPairing = false

    var body: some View {
        NavigationStack {
            VStack(spacing: 24) {

                VStack(spacing: 12) {
                    Button("Settings") { showSettings = true }
                        .buttonStyle(.borderedProminent)

                    Button(appModel.streamingController.isStreaming ? "Stop Streaming" : "Start Streaming") {
                        if appModel.streamingController.isStreaming {
                            appModel.streamingController.stop()
                        } else {
                            appModel.streamingController.start(with: appModel.configuration)
                        }
                    }

                    HStack {
                        Button("Load Config") { isImportingConfig = true }
                        Button("Save Config") { isExportingConfig = true }
                    }

                    Button("Pair / Register Console") { showPairing = true }
                }

                // immersive space removed for now
            }
            .padding()
            .navigationTitle("VisionStream")
            .sheet(isPresented: $showSettings) { SettingsView() }
            .sheet(isPresented: $showPairing) { PairingView() }
            .fileImporter(isPresented: $isImportingConfig, allowedContentTypes: [.json]) { result in
                if case .success(let url) = result { appModel.configuration.load(from: url) }
            }
            .fileExporter(isPresented: $isExportingConfig, document: appModel.configuration.document(), contentType: .json, defaultFilename: "visionstream.json") { _ in }
        }
    }
}

#Preview(windowStyle: .automatic) {
    ContentView()
        .environment(AppModel())
}
