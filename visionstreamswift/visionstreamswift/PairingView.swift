//
//  PairingView.swift
//  visionstreamswift
//
//  Enter PSN ID/AccountID and PIN, trigger registration.
//

import SwiftUI

struct PairingView: View {
    @Environment(AppModel.self) private var appModel
    @State private var psnOnlineId: String = ""
    @State private var psnAccountIdBase64: String = ""
    @State private var pin: String = ""
    @State private var inProgress = false
    @Environment(\.dismiss) private var dismiss
    private let bridge: (any ChiakiRegistering) = ChiakiBridgeStub()

    private var hostBinding: Binding<String> {
        Binding(get: { appModel.configuration.host }, set: { appModel.configuration.host = $0 })
    }

    private var isPS5Binding: Binding<Bool> {
        Binding(get: { appModel.configuration.isPS5 }, set: { appModel.configuration.isPS5 = $0 })
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Console") {
                    TextField("Host or IP", text: hostBinding)
                        .textContentType(.URL)
                        .keyboardType(.URL)
                    Toggle("PS5", isOn: isPS5Binding)
                }

                Section("PSN Identity") {
                    TextField("PSN Online ID (username)", text: $psnOnlineId)
                    TextField("Chiaki-encoded Account ID (base64)", text: $psnAccountIdBase64)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                }

                Section("Registration PIN") {
                    TextField("8-digit PIN", text: $pin)
                        .keyboardType(.numberPad)
                }

                Section {
                    Button(action: register) {
                        if inProgress { ProgressView() } else { Text("Register") }
                    }
                    .disabled(!canRegister || inProgress)
                }
            }
            .navigationTitle("Pair / Register")
            .toolbar { ToolbarItem(placement: .cancellationAction) { Button("Close") { dismiss() } } }
        }
        .onAppear {
            psnOnlineId = appModel.configuration.psnOnlineId ?? ""
            psnAccountIdBase64 = appModel.configuration.psnAccountIdBase64 ?? ""
        }
    }

    var canRegister: Bool {
        guard !appModel.configuration.host.isEmpty, pin.count == 8 else { return false }
        // Require both identifiers for robustness
        return !psnOnlineId.isEmpty && !psnAccountIdBase64.isEmpty
    }

    func register() {
        inProgress = true
        appModel.configuration.psnOnlineId = psnOnlineId.isEmpty ? nil : psnOnlineId
        appModel.configuration.psnAccountIdBase64 = psnAccountIdBase64.isEmpty ? nil : psnAccountIdBase64
        bridge.register(host: appModel.configuration.host,
                        isPS5: appModel.configuration.isPS5,
                        psnOnlineId: appModel.configuration.psnOnlineId,
                        psnAccountIdBase64: appModel.configuration.psnAccountIdBase64,
                        pin: pin) { result in
            DispatchQueue.main.async {
                inProgress = false
                switch result {
                case .success(let secrets):
                    // Convert hex strings from shim to base64 for storage
                    appModel.configuration.rpRegistKeyBase64 = Data(hexString: secrets.rpRegistKeyHexPadded)?.base64EncodedString()
                    appModel.configuration.rpKeyBase64 = Data(hexString: secrets.rpKeyHex)?.base64EncodedString()
                    dismiss()
                case .failure:
                    // Keep the sheet open; in a later pass show error
                    break
                }
            }
        }
    }
}


