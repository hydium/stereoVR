/************************************************************************************

Filename    :   Vr360PhotoViewer.cpp
Content     :   Trivial game style scene viewer VR sample
Created     :   September 8, 2013
Authors     :   John Carmack

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#include "Vr360PhotoViewer.h"
#include "PackageFiles.h" // for // LOGCPUTIME

#include <sys/stat.h>
#include <errno.h>

#include <algorithm>

#include <dirent.h>
#include "unistd.h"

#include "Misc/Log.h"
#include "Render/Egl.h"
#include "Render/GlGeometry.h"

#include "JniUtils.h"
#include "OVR_Std.h"
#include "OVR_Math.h"
#include "OVR_BinaryFile2.h"

#include "stb_image.h"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

using OVR::Bounds3f;
using OVR::Matrix4f;
using OVR::Quatf;
using OVR::Vector2f;
using OVR::Vector3d;
using OVR::Vector3f;
using OVR::Vector4f;

//=============================================================================
//                             Shaders
//=============================================================================

static const char* TextureMvpVertexShaderSrc = R"glsl(
attribute vec4 Position;
attribute vec2 TexCoord;
varying highp vec2 oTexCoord;
void main()
{
   gl_Position = TransformVertex( Position );
   oTexCoord = TexCoord;
}
)glsl";

static const char* TexturedMvpFragmentShaderSrc = R"glsl(
uniform sampler2D Texture0;
varying highp vec2 oTexCoord;
void main()
{
   gl_FragColor = texture2D( Texture0, oTexCoord );
}
)glsl";
//gl_FragColor = oColor * texture2D( Texture0, oTexCoord );


GLubyte* image;
const int width = 1920;
const int height = 1080;

GLuint texture;


//GStreamer variables
GstBuffer *buffer = NULL;
GstSample *sample = NULL;
GstMapInfo map;

GstElement *pipeline, *appsink;


//=============================================================================
//                             Vr360PhotoViewer
//=============================================================================

bool Vr360PhotoViewer::AppInit(const OVRFW::ovrAppContext* appContext) {
    ALOGV("AppInit - enter");

    GError *error = NULL;

    gst_init (NULL, NULL);

    pipeline = gst_parse_launch("videotestsrc ! video/x-raw, width=1920, height=1080, format=\"RGB\" ! appsink name=appsink", &error);

    if (error != NULL) {
        ALOGV("Couldn't launch the pipeline");

        return -1;
    }

    g_object_set (appsink, "max-buffers", 1, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    sample = gst_app_sink_pull_sample (GST_APP_SINK (appsink));

    buffer = gst_sample_get_buffer (sample);
    gst_buffer_map ( buffer, &map, GST_MAP_READ);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, map.data);


    image = (GLubyte*) malloc(height * width * 3);

    int i, j;
    uint8_t c;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            c = (((i&0x8)==0^(j&0x8)==0))*255;

            *(image + ( i * width * 3 + j * 3)) = (GLubyte) c;
            *(image + ( i * width * 3 + j * 3 + 1)) = (GLubyte) c;
            *(image + ( i * width * 3 + j * 3 + 2)) = (GLubyte) c;
        }
    }


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_FILTER, GL_NEAREST);

//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);


    /// Init Rendering
    SurfaceRender.Init();
    /// Start with an empty model Scene
    SceneModel = std::unique_ptr<OVRFW::ModelFile>(new OVRFW::ModelFile("Void"));
    Scene.SetWorldModel(*(SceneModel.get()));
    Scene.SetFreeMove(FreeMove);
    Vector3f seat = {3.0f, 0.0f, 3.6f};
    Scene.SetFootPos(seat);
    /// Build Shader and required uniforms
    static OVRFW::ovrProgramParm uniformParms[] = {
//            {"UniformColor", OVRFW::ovrProgramParmType::FLOAT_VECTOR4},
            {"Texture0", OVRFW::ovrProgramParmType::TEXTURE_SAMPLED},
    };
    TexturedMvpProgram = OVRFW::GlProgram::Build(
            TextureMvpVertexShaderSrc,
            TexturedMvpFragmentShaderSrc,
//            nullptr, 0);
            uniformParms,
            sizeof(uniformParms) / sizeof(OVRFW::ovrProgramParm));

    /// Use a globe mesh for the video surface
    GlobeSurfaceDef.surfaceName = "Globe";
    GlobeSurfaceDef.geo = OVRFW::BuildGlobe();
    GlobeSurfaceDef.graphicsCommand.Program = TexturedMvpProgram;
    GlobeSurfaceDef.graphicsCommand.GpuState.depthEnable = false;
    GlobeSurfaceDef.graphicsCommand.GpuState.cullEnable = false;

    /// All done
    ALOGV("AppInit - exit");
    return true;
}

void Vr360PhotoViewer::AppShutdown(const OVRFW::ovrAppContext*) {
    ALOGV("AppShutdown - enter");
    RenderState = RENDER_STATE_ENDING;

    GlobeSurfaceDef.geo.Free();
    OVRFW::GlProgram::Free(TexturedMvpProgram);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    SurfaceRender.Shutdown();

    ALOGV("AppShutdown - exit");
}

void Vr360PhotoViewer::AppResumed(const OVRFW::ovrAppContext* /* context */) {
    ALOGV("Vr360PhotoViewer::AppResumed");
    RenderState = RENDER_STATE_RUNNING;

    // Load the file when we enter VR mode so that we ensure to have a EGL context
//    ReloadPhoto();
}

void Vr360PhotoViewer::AppPaused(const OVRFW::ovrAppContext* /* context */) {
    ALOGV("Vr360PhotoViewer::AppPaused");
}

OVRFW::ovrApplFrameOut Vr360PhotoViewer::AppFrame(const OVRFW::ovrApplFrameIn& vrFrame) {
    /// Set free move mode if the left trigger is on
    FreeMove = (vrFrame.AllButtons & ovrButton_Trigger) != 0;
    Scene.SetFreeMove(FreeMove);

    // Player movement
    Scene.Frame(vrFrame);

    return OVRFW::ovrApplFrameOut();
}

void Vr360PhotoViewer::AppRenderFrame(
        const OVRFW::ovrApplFrameIn& in,
        OVRFW::ovrRendererOutput& out) {
    switch (RenderState) {
        case RENDER_STATE_LOADING: {
            DefaultRenderFrame_Loading(in, out);
        } break;
        case RENDER_STATE_RUNNING: {
            Scene.GetFrameMatrices(
                    SuggestedEyeFovDegreesX, SuggestedEyeFovDegreesY, out.FrameMatrices);
            Scene.GenerateFrameSurfaceList(out.FrameMatrices, out.Surfaces);


//            sample = gst_app_sink_pull_sample (GST_APP_SINK (appsink));
//
//            buffer = gst_sample_get_buffer (sample);
//            gst_buffer_map ( buffer, &map, GST_MAP_READ);

//            glBindTexture(GL_TEXTURE_2D, texture);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, map.data);


            GlobeProgramTexture = OVRFW::GlTexture(texture, GL_TEXTURE_2D, width, height);
//            Vector4f GlobeProgramColor = Vector4f(1.0f);
            GlobeSurfaceDef.graphicsCommand.Program = TexturedMvpProgram;
//            GlobeSurfaceDef.graphicsCommand.UniformData[0].Data = &GlobeProgramColor;
            GlobeSurfaceDef.graphicsCommand.UniformData[0].Data = &GlobeProgramTexture;
            out.Surfaces.push_back(OVRFW::ovrDrawSurface(&GlobeSurfaceDef));

            DefaultRenderFrame_Running(in, out);


//            gst_buffer_unmap(buffer, &map);
//            gst_sample_unref(sample);

        } break;
        case RENDER_STATE_ENDING: {
            DefaultRenderFrame_Ending(in, out);
        } break;
    }
}

void Vr360PhotoViewer::AppEyeGLStateSetup(
        const OVRFW::ovrApplFrameIn& /* in */,
        const ovrFramebuffer* fb,
        int /* eye */) {
    GL(glEnable(GL_SCISSOR_TEST));
    GL(glDepthMask(GL_TRUE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LEQUAL));
    GL(glEnable(GL_CULL_FACE));
    GL(glViewport(0, 0, fb->Width, fb->Height));
    GL(glScissor(0, 0, fb->Width, fb->Height));
    // Clear the eye buffers to 0 alpha so the overlay plane shows through.
//    Vector4f clearColor = CurrentPanoIsCubeMap ? Vector4f(0.0f, 0.0f, 0.0f, 0.0f)
//                                               : Vector4f(0.125f, 0.0f, 0.125f, 1.0f);
    Vector4f clearColor = Vector4f(0.125f, 0.0f, 0.125f, 1.0f);


    GL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Vr360PhotoViewer::AppRenderEye(
        const OVRFW::ovrApplFrameIn& in,
        OVRFW::ovrRendererOutput& out,
        int eye) {
    // Render the surfaces returned by Frame.
    SurfaceRender.RenderSurfaceList(
            out.Surfaces,
            out.FrameMatrices.EyeView[0], // always use 0 as it assumes an array
            out.FrameMatrices.EyeProjection[0], // always use 0 as it assumes an array
            eye);
}
