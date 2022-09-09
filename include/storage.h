#pragma once

#include <Arduino.h>

#include "image.h"

class storage_t {
    public:
        bool available;

        void begin();
        void update();

        //returns the number of times bytes_needed would fit into the available space. 
        // (For remaining image storage).
        //get available space in bytes with bytes_available(1).
        size_t bytes_available(size_t bytes_needed = 1);
        bool write();
        bool writeImage(image_t);

        size_t imagesAvailable();
        void readImage();

    private:
        size_t next_image_index();
};

inline storage_t storage;