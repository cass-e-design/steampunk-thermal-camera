#pragma once

#include "app.h"
#include "app_camera.h"
#include "app_gallery.h"

//keep these up to date with apps[]....
#define APP_CAMERA 0
#define APP_GALLERY 1

//allow global direct access to the instances in addition to the array
app_camera camera_app;
app_gallery gallery_app;

app_t* apps[] {
    &camera_app,
    &gallery_app
};