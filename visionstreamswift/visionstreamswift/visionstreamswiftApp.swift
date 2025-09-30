//
//  visionstreamswiftApp.swift
//  visionstreamswift
//
//  Created by Sam Smith on 30/09/2025.
//

import ARKit
import CompositorServices
import SwiftUI

struct ImmersiveSpaceContent: CompositorContent {

    var appModel: AppModel

    var body: some CompositorContent {
        CompositorLayer(configuration: self) { @MainActor layerRenderer in
            Renderer.startRenderLoop(layerRenderer, appModel: appModel, arSession: ARKitSession())
        }
    }
}

extension ImmersiveSpaceContent: CompositorLayerConfiguration {
    func makeConfiguration(capabilities: LayerRenderer.Capabilities, configuration: inout LayerRenderer.Configuration) {
        // Use a conservative raster sample count for broad compatibility
        configuration.drawableRenderContextRasterSampleCount = 1

        if capabilities.drawableRenderContextSupportedStencilFormats.contains(.stencil8) {
            configuration.drawableRenderContextStencilFormat = .stencil8
        } else {
            configuration.drawableRenderContextStencilFormat = .depth32Float_stencil8
            configuration.depthFormat = .depth32Float_stencil8
        }

        let foveationEnabled = capabilities.supportsFoveation
        configuration.isFoveationEnabled = foveationEnabled

        let options: LayerRenderer.Capabilities.SupportedLayoutsOptions = foveationEnabled ? [.foveationEnabled] : []
        let supportedLayouts = capabilities.supportedLayouts(options: options)

        configuration.layout = supportedLayouts.contains(.layered) ? .layered : .dedicated

        // Remove unsupported property for simulator toolchains
    }
}

@main
struct visionstreamswiftApp: App {

    @State private var appModel = AppModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(appModel)
        }

        ImmersiveSpace(id: appModel.immersiveSpaceID) {
            ImmersiveSpaceContent(appModel: appModel)
        }
        .immersionStyle(selection: .constant(.progressive), in: .progressive)
    }
}