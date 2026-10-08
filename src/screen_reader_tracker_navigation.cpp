#include "screen_reader_tracker.h"
#include "screen_reader.h"

namespace screen_reader
{
std::string tracker_key_action( int key, bool control, bool alt, bool shift )
{
    if( alt || shift || ( key != '[' && key != ']' ) ) {
        return {};
    }
    if( control ) {
        return key == '[' ? "TRACKER_PREVIOUS_OBJECT" : "TRACKER_NEXT_OBJECT";
    }
    return key == '[' ? "TRACKER_PREVIOUS_ITEM" : "TRACKER_NEXT_ITEM";
}
void tracker_cursor::cycle_objects( int direction, size_t count )
{
    object = next_review_index( object, direction, count );
    item = 0;
}
void tracker_cursor::cycle_items( int direction, size_t count )
{
    item = next_review_index( item, direction, count );
}
}
