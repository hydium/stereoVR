/*******************************************************************************

Filename    :   Main.cpp
Content     :   Base project for mobile VR samples
Created     :   February 21, 2018
Authors     :   John Carmack, J.M.P. van Waveren, Jonathan Wright
Language    :   C++

Copyright:	Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*******************************************************************************/

#include "Platform/Android/Android.h"

#include <android/window.h>
#include <android/native_window_jni.h>
#include <android_native_app_glue.h>

#include <memory>

#include "Appl.h"
#include "Vr360PhotoViewer.h"

Vr360PhotoViewer* appPtr = nullptr;


//==============================================================
// android_main
//==============================================================
void android_main(struct android_app* app) {
    appPtr = nullptr;
    std::unique_ptr<Vr360PhotoViewer> appl =
        std::unique_ptr<Vr360PhotoViewer>(new Vr360PhotoViewer(0, 0, 0, 0));
    appPtr = appl.get();
    appl->Run(app);
    appPtr = nullptr;
}
