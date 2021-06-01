/************************************************************************************

Filename    :   Vr360PhotoViewer.h
Content     :   Trivial game style scene viewer VR sample
Created     :   September 8, 2013
Authors     :   John Carmack

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

************************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Appl.h"
#include "Model/SceneView.h"
#include "Render/SurfaceRender.h"
#include "Render/SurfaceTexture.h"

class Vr360PhotoViewer : public OVRFW::ovrAppl {
public:
    Vr360PhotoViewer(
            const int32_t mainThreadTid,
            const int32_t renderThreadTid,
            const int cpuLevel,
            const int gpuLevel)
            : ovrAppl(mainThreadTid, renderThreadTid, cpuLevel, gpuLevel, true /* useMutliView */),
              FreeMove(false),
              SceneModel(nullptr),
              RenderState(RENDER_STATE_LOADING) {}
    virtual ~Vr360PhotoViewer() {}

    // Called when the application initializes.
    // Must return true if the application initializes successfully.
    virtual bool AppInit(const OVRFW::ovrAppContext* context) override;
    // Called when the application shuts down
    virtual void AppShutdown(const OVRFW::ovrAppContext* context) override;
    // Called when the application is resumed by the system.
    virtual void AppResumed(const OVRFW::ovrAppContext* contet) override;
    // Called when the application is paused by the system.
    virtual void AppPaused(const OVRFW::ovrAppContext* context) override;
    // Called once per frame when the VR session is active.
    virtual OVRFW::ovrApplFrameOut AppFrame(const OVRFW::ovrApplFrameIn& in) override;
    // Called once per frame to allow the application to render eye buffers.
    virtual void AppRenderFrame(const OVRFW::ovrApplFrameIn& in, OVRFW::ovrRendererOutput& out)
    override;
    // Called once per eye each frame for default renderer
    virtual void
    AppRenderEye(const OVRFW::ovrApplFrameIn& in, OVRFW::ovrRendererOutput& out, int eye) override;
    // Called once per eye each frame for default renderer
    virtual void
    AppEyeGLStateSetup(const OVRFW::ovrApplFrameIn& in, const ovrFramebuffer* fb, int eye) override;


private:
    bool FreeMove;
    OVRFW::OvrSceneView Scene;
    OVRFW::ovrSurfaceRender SurfaceRender;
    std::unique_ptr<OVRFW::ModelFile> SceneModel;
    OVRFW::ovrAppl::ovrRenderState RenderState;

    // panorama vars
    OVRFW::GlProgram TexturedMvpProgram;
    OVRFW::ovrSurfaceDef GlobeSurfaceDef;
    OVRFW::GlTexture GlobeProgramTexture;
};
