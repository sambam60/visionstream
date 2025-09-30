//
//  AppConfiguration.swift
//  visionstreamswift
//
//  Defines user-editable streaming settings and JSON load/save helpers.
//

import Foundation
import SwiftUI
import UniformTypeIdentifiers

struct AppConfiguration: Codable, Equatable {
    enum Resolution: String, Codable, CaseIterable, Identifiable {
        case p360
        case p540
        case p720
        case p1080
        var id: String { rawValue }
        var label: String {
            switch self {
            case .p360: return "360p"
            case .p540: return "540p"
            case .p720: return "720p"
            case .p1080: return "1080p"
            }
        }
    }

    enum FPS: Int, Codable, CaseIterable, Identifiable {
        case fps30 = 30
        case fps60 = 60
        var id: Int { rawValue }
        var label: String { "\(rawValue) FPS" }
    }

    // Connection
    var host: String
    var isPS5: Bool
    var psnOnlineId: String?
    var psnAccountIdBase64: String?

    // Streaming
    var bitrateKbps: Int
    var resolution: Resolution
    var fps: FPS
    var hdrEnabled: Bool
    var audioEnabled: Bool

    // Registered secrets
    var rpRegistKeyHexPadded: String?
    var rpKeyHex: String?

    static var `default`: AppConfiguration {
        .init(host: "",
              isPS5: true,
              psnOnlineId: nil,
              psnAccountIdBase64: nil,
              bitrateKbps: 15000,
              resolution: .p1080,
              fps: .fps60,
              hdrEnabled: false,
              audioEnabled: true,
              rpRegistKeyHexPadded: nil,
              rpKeyHex: nil)
    }

    mutating func load(from url: URL) {
        do {
            let data = try Data(contentsOf: url)
            let decoded = try JSONDecoder().decode(AppConfiguration.self, from: data)
            self = decoded
        } catch {
            // Keep current config on error
        }
    }

    func document() -> AppConfigurationDocument {
        AppConfigurationDocument(configuration: self)
    }
}

struct AppConfigurationDocument: FileDocument {
    static var readableContentTypes: [UTType] { [.json] }
    var configuration: AppConfiguration

    init(configuration: AppConfiguration) { self.configuration = configuration }

    init(configuration: ReadConfiguration) throws {
        if let data = configuration.file.regularFileContents {
            self.configuration = (try? JSONDecoder().decode(AppConfiguration.self, from: data)) ?? .default
        } else {
            self.configuration = .default
        }
    }

    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        let data = try JSONEncoder().encode(self.configuration)
        return .init(regularFileWithContents: data)
    }
}


