#pragma once
#ifndef CATA_SRC_SCREEN_READER_TRACKER_H
#define CATA_SRC_SCREEN_READER_TRACKER_H
#include <cstddef>
#include <string>

namespace screen_reader
{
struct tracker_cursor {
    size_t object = 0;
    size_t item = 0;
    void cycle_objects( int direction, size_t count );
    void cycle_items( int direction, size_t count );
};
std::string tracker_key_action( int key, bool control, bool alt, bool shift );
void object_tracker( int direction, bool items = false );
void update_tracker_walk();
bool tracker_walk_active();
}
#endif
