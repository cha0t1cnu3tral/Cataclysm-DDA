#include "screen_reader_tracker.h"
#include "screen_reader.h"

namespace screen_reader
{
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
