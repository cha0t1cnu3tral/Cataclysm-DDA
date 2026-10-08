#include "cata_catch.h"
#include "screen_reader_tracker.h"

TEST_CASE( "tracker_object_changes_reset_the_item_selection", "[screen_reader][tracker]" )
{
    screen_reader::tracker_cursor cursor;
    cursor.cycle_items( 1, 5 );
    cursor.cycle_items( 1, 5 );
    REQUIRE( cursor.item == 2 );
    cursor.cycle_objects( 1, 3 );
    CHECK( cursor.object == 1 );
    CHECK( cursor.item == 0 );
    cursor.cycle_objects( -1, 3 );
    cursor.cycle_objects( -1, 3 );
    CHECK( cursor.object == 2 );
}

TEST_CASE( "tracker_items_wrap_without_changing_the_object", "[screen_reader][tracker]" )
{
    screen_reader::tracker_cursor cursor;
    cursor.object = 2;
    cursor.cycle_items( -1, 3 );
    CHECK( cursor.item == 2 );
    cursor.cycle_items( 1, 3 );
    CHECK( cursor.item == 0 );
    CHECK( cursor.object == 2 );
    cursor.cycle_items( 1, 0 );
    CHECK( cursor.item == 0 );
    cursor.cycle_objects( 1, 0 );
    CHECK( cursor.object == 0 );
}
