//
//  SettingsView.swift
//  visionstreamswift
//
//  Edit bitrate, resolution, FPS, HDR, and audio.
//

import SwiftUI

struct SettingsView: View {
    @Environment(AppModel.self) private var appModel

    private var resolutionBinding: Binding<AppConfiguration.Resolution> {
        Binding(get: { appModel.configuration.resolution }, set: { appModel.configuration.resolution = $0 })
    }

    private var fpsBinding: Binding<AppConfiguration.FPS> {
        Binding(get: { appModel.configuration.fps }, set: { appModel.configuration.fps = $0 })
    }

    private var bitrateBinding: Binding<Int> {
        Binding(get: { appModel.configuration.bitrateKbps }, set: { appModel.configuration.bitrateKbps = $0 })
    }

    private var hdrBinding: Binding<Bool> {
        Binding(get: { appModel.configuration.hdrEnabled }, set: { appModel.configuration.hdrEnabled = $0 })
    }

    private var audioBinding: Binding<Bool> {
        Binding(get: { appModel.configuration.audioEnabled }, set: { appModel.configuration.audioEnabled = $0 })
    }

    private var hostBinding: Binding<String> {
        Binding(get: { appModel.configuration.host }, set: { appModel.configuration.host = $0 })
    }

    private var isPS5Binding: Binding<Bool> {
        Binding(get: { appModel.configuration.isPS5 }, set: { appModel.configuration.isPS5 = $0 })
    }

    var body: some View {
        Form {
            Section("Video") {
                Picker("Resolution", selection: resolutionBinding) {
                    ForEach(AppConfiguration.Resolution.allCases) { r in
                        Text(r.label).tag(r)
                    }
                }

                Picker("Frame Rate", selection: fpsBinding) {
                    ForEach(AppConfiguration.FPS.allCases) { f in
                        Text(f.label).tag(f)
                    }
                }

                Stepper(value: bitrateBinding, in: 1000...90000, step: 500) {
                    HStack {
                        Text("Bitrate")
                        Spacer()
                        Text("\(appModel.configuration.bitrateKbps) kbps")
                            .foregroundStyle(.secondary)
                    }
                }

                Toggle("HDR", isOn: hdrBinding)
            }

            Section("Audio") {
                Toggle("Enable Audio", isOn: audioBinding)
            }

            Section("Connection") {
                TextField("Host or IP", text: hostBinding)
                    .textContentType(.URL)
                    .keyboardType(.URL)
                Toggle("PS5", isOn: isPS5Binding)
            }
        }
        .navigationTitle("Settings")
        .presentationDetents([.medium, .large])
    }
}


