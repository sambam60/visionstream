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
        print("🔐 [PairingView] Starting registration...")
        print("   Host: \(appModel.configuration.host)")
        print("   PS5: \(appModel.configuration.isPS5)")
        print("   PSN Online ID: \(psnOnlineId)")
        print("   PSN Account ID (first 10 chars): \(String(psnAccountIdBase64.prefix(10)))...")
        
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
                    print("✅ [PairingView] Registration successful!")
                    print("   rpRegistKeyHexPadded: \(secrets.rpRegistKeyHexPadded)")
                    print("   rpKeyHex: \(secrets.rpKeyHex)")
                    
                    // Convert hex strings from shim to base64 for storage
                    let registKeyData = Data(hexString: secrets.rpRegistKeyHexPadded)
                    let rpKeyData = Data(hexString: secrets.rpKeyHex)
                    
                    print("   Converted to Data:")
                    print("   - registKeyData: \(registKeyData != nil ? "\(registKeyData!.count) bytes" : "nil")")
                    print("   - rpKeyData: \(rpKeyData != nil ? "\(rpKeyData!.count) bytes" : "nil")")
                    
                    appModel.configuration.rpRegistKeyBase64 = registKeyData?.base64EncodedString()
                    appModel.configuration.rpKeyBase64 = rpKeyData?.base64EncodedString()
                    
                    print("   Saved to config:")
                    print("   - rpRegistKeyBase64: \(appModel.configuration.rpRegistKeyBase64 ?? "nil")")
                    print("   - rpKeyBase64: \(appModel.configuration.rpKeyBase64 ?? "nil")")
                    
                    dismiss()
                case .failure(let error):
                    print("❌ [PairingView] Registration failed: \(error)")
                    // Keep the sheet open; in a later pass show error
                    break
                }
            }
        }
    }
}


