#include "app_gallery.h"
#include "display.h"

void app_gallery::draw() {
    app_t::draw();

    //TODO: draw title

    //TODO: draw current image

    //TODO: draw image title

    //TODO: draw next/prev arrows

}

void app_gallery::update(state_t state) {
    
    //TODO: don't alway ask for redraw
    redraw_needed = true;

    //TODO: if tap 'back' go back to camera

    //TODO: if tap left, go to previous

    //TODO: if tap right, go to next

    app_t::update(state);
}