#include "storage.h"

#include <SD.h>
#include "pins.h"

const char* INDEX_FILE = "/index";

void storage_t::begin() {
    pinMode(SD_CS, OUTPUT);

    update();
}

void storage_t::update() {
    SD.end();
    available = SD.begin(SD_CS);
}

size_t storage_t::imagesAvailable() {
    if (!available) return 0;

    File root = SD.open("/");
    root.rewindDirectory();

    size_t images = 0;

    while (true) {
        File entry = root.openNextFile();
        if (!entry) break; //No more files

        if (strstr(entry.name(), ".thr"))
            images++;
        
        entry.close();
    }
    
    return images;
}

size_t storage_t::bytes_available(size_t bytes_needed) {
    
    //TODO: Fill this out with actual SdFat - SD.h just can't do this
    //src: https://forum.arduino.cc/t/can-arduino-read-current-size-capacity-of-one-sd-card/244927/3
    
    return available ? 1 : 0;
}


bool storage_t::writeImage(image_t image) {
    if (bytes_available(THRML_IMG_LEN) < 1) {
        return false;
    }

    size_t index = 0;
    char index_parse_buf[4];
    if (SD.exists((char*)INDEX_FILE)) {
        File index_file = SD.open(INDEX_FILE, FILE_READ);
        index_file.readBytes(index_parse_buf, (size_t)4UL);
        index = String(index_parse_buf).toInt();

        Serial.print("Read existing image index: ");
        Serial.println(index);
        index_file.close();
    }
    
    //todo: actually write image
    char image_name_buf[16] = "thrml_0000.thr";
    snprintf(image_name_buf, 16, "therml_%04u.thr", index);

    File new_image = SD.open(image_name_buf, FILE_WRITE);
    new_image.write(THRML_IMG_SIG_V1, sizeof(THRML_IMG_SIG_V1));
    new_image.write((char*)image.frame, THRML_IMG_FRAME_LEN);
    new_image.close();

    //create or edit index file to increment number
    File index_file = SD.open(INDEX_FILE, FILE_WRITE);
    index_file.print(index);
    index_file.close();

    return true;
}