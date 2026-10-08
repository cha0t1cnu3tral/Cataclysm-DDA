#include "cata_catch.h"
#include "screen_reader_tracker.h"

TEST_CASE( "tracker_brackets_distinguish_categories_entries_and_other_modifiers",
           "[screen_reader][tracker]" )
{
    using screen_reader::tracker_key_action;
    CHECK( tracker_key_action( '[', true, false, false ) == "TRACKER_PREVIOUS_OBJECT" );
    CHECK( tracker_key_action( ']', true, false, false ) == "TRACKER_NEXT_OBJECT" );
    CHECK( tracker_key_action( '[', false, false, false ) == "TRACKER_PREVIOUS_ITEM" );
    CHECK( tracker_key_action( ']', false, false, false ) == "TRACKER_NEXT_ITEM" );
    CHECK( tracker_key_action( '[', true, true, false ).empty() );
    CHECK( tracker_key_action( ']', false, false, true ).empty() );
    CHECK( tracker_key_action( 27, true, false, false ).empty() );
    CHECK( tracker_key_action( 'a', false, false, false ).empty() );
}

TEST_CASE( "tracker_category_changes_reset_the_entry_selection", "[screen_reader][tracker]" )
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

TEST_CASE( "tracker_entries_wrap_without_changing_the_category", "[screen_reader][tracker]" )
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

TEST_CASE( "tracker_changing_to_a_shorter_category_keeps_selection_in_bounds",
           "[screen_reader][tracker]" )
{
    screen_reader::tracker_cursor cursor;
    // A category with several entries followed by a category with one entry.
    cursor.cycle_items( -1, 5 );
    REQUIRE( cursor.item == 4 );
    cursor.cycle_objects( 1, 3 );
    CHECK( cursor.object == 1 );
    CHECK( cursor.item == 0 );
    cursor.cycle_items( 1, 1 );
    CHECK( cursor.object == 1 );
    CHECK( cursor.item == 0 );
    cursor.cycle_objects( -1, 3 );
    CHECK( cursor.object == 0 );
    cursor.cycle_objects( -1, 3 );
    CHECK( cursor.object == 2 );
    CHECK( cursor.item == 0 );
}
