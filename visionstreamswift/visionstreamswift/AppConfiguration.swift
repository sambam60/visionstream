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
    var rpRegistKeyBase64: String?
    var rpKeyBase64: String?

    // Derived convenience
    var targetValue: Int { isPS5 ? 1_000_100 : 1_000 }

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
              rpRegistKeyBase64: nil,
              rpKeyBase64: nil)
    }

    mutating func load(from url: URL) {
        do {
            let data = try Data(contentsOf: url)
            let decoder = JSONDecoder()
            if let decoded = try? decoder.decode(AppConfiguration.self, from: data) {
                self = decoded
                return
            }
            // Fallback: parse legacy Chiaki/FlipScreen-style host map format
            if let fallback = try? AppConfiguration.parseLegacyConfig(data: data) {
                self = fallback
                return
            }
        } catch {
            // Keep current config on error
        }
    }

    func document() -> AppConfigurationDocument {
        AppConfigurationDocument(configuration: self)
    }
}

extension AppConfiguration {
    private struct LegacyEntry: Decodable {
        let psnAccountId: String?
        let hostName: String?
        let hostAddr: String?
        let rpKey: String?
        let rpKeyType: Int?
        let psnOnlineId: String?
        let videoWidth: Int?
        let videoResolution: String?
        let target: String?
        let sessionPort: String?
        let connectPort: String?
        let videoHeight: Int?
        let rpRegistKey: String?
        let wakeUpPort: String?
        let videoFps: Int?
        let bitRate: Int?
    }

    static func parseLegacyConfig(data: Data) throws -> AppConfiguration {
        let raw = try JSONSerialization.jsonObject(with: data) as? [String: Any]
        guard let (_, value) = raw?.first else { throw NSError(domain: "config", code: 1) }
        let valueData = try JSONSerialization.data(withJSONObject: value)
        let entry = try JSONDecoder().decode(LegacyEntry.self, from: valueData)

        var cfg = AppConfiguration.default
        cfg.host = entry.hostAddr ?? entry.hostName ?? cfg.host
        cfg.isPS5 = { if let t = entry.target, let tv = Int(t) { return tv >= 1_000_000 } else { return cfg.isPS5 } }()
        cfg.psnOnlineId = entry.psnOnlineId
        cfg.psnAccountIdBase64 = entry.psnAccountId
        if let br = entry.bitRate { cfg.bitrateKbps = br }
        if let vf = entry.videoFps, let fps = AppConfiguration.FPS(rawValue: vf) { cfg.fps = fps }
        if let res = entry.videoResolution?.lowercased() {
            switch res {
            case "360p": cfg.resolution = .p360
            case "540p": cfg.resolution = .p540
            case "720p": cfg.resolution = .p720
            case "1080p": cfg.resolution = .p1080
            default: break
            }
        } else if let w = entry.videoWidth, let h = entry.videoHeight {
            if w >= 1920 || h >= 1080 { cfg.resolution = .p1080 }
            else if w >= 1280 || h >= 720 { cfg.resolution = .p720 }
            else if w >= 960 || h >= 540 { cfg.resolution = .p540 }
            else { cfg.resolution = .p360 }
        }
        cfg.rpKeyBase64 = entry.rpKey
        cfg.rpRegistKeyBase64 = entry.rpRegistKey
        return cfg
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


