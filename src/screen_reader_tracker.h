#pragma once
#ifndef CATA_SRC_SCREEN_READER_TRACKER_H
#define CATA_SRC_SCREEN_READER_TRACKER_H
#include <cstddef>

namespace screen_reader
{
struct tracker_cursor {
    size_t object = 0;
    size_t item = 0;
    void cycle_objects( int direction, size_t count );
    void cycle_items( int direction, size_t count );
};
void object_tracker( int direction );
void update_tracker_walk();
bool tracker_walk_active();
}
#endif
