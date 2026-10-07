#include "cata_catch.h"
#include "screen_reader.h"

TEST_CASE( "screen_reader_removes_layout_and_preserves_content", "[screen_reader]" )
{
    CHECK( screen_reader::readable_text( "  <color_red>Danger</color>\n  ahead  " ) == "Danger ahead" );
    CHECK( screen_reader::readable_text( "────│┌┐" ).empty() );
    CHECK( screen_reader::readable_text( "┌ Health: 25/80 │" ) == "Health: 25/80" );
    CHECK( screen_reader::readable_text( "<unknown> café 日本語" ) == "<unknown> café 日本語" );
    CHECK( screen_reader::readable_text( "Stamina: 0" ) == "Stamina: 0" );
    CHECK( screen_reader::readable_text( "| + ---- |" ).empty() );
}

TEST_CASE( "screen_reader_does_not_repeat_unchanged_or_scrolled_text", "[screen_reader]" )
{
    const std::vector<std::string> before = { "knife", "coat", "boots" };
    CHECK( screen_reader::changed_lines( before, before ).empty() );
    CHECK( screen_reader::changed_lines( before, { "coat", "boots", "knife" } ).empty() );
    CHECK( screen_reader::changed_lines( before, { "boots", "hat", "hat", "" } ) ==
           std::vector<std::string> { "hat" } );
    CHECK( screen_reader::changed_lines( { "Health: 80" }, { "Health: 25" } ) ==
           std::vector<std::string> { "Health: 25" } );
}

TEST_CASE( "screen_reader_announces_selection_changes_even_when_labels_are_unchanged",
           "[screen_reader]" )
{
    const std::vector<std::string> labels = { "Choose an item", "knife", "coat" };
    const auto entry = screen_reader::describe_changes( {}, labels, "", "knife", true );
    CHECK( entry.text == "Choose an item. knife. coat" );
    CHECK( entry.interrupt );
    const auto selection = screen_reader::describe_changes( labels, labels, "knife", "coat", false );
    CHECK( selection.text == "coat" );
    CHECK( selection.interrupt );
    CHECK( screen_reader::describe_changes( labels, labels, "coat", "coat", false ).text.empty() );
    const auto message = screen_reader::describe_changes( { "Health: 80" }, { "Health: 25" }, "", "",
                         false );
    CHECK( message.text == "Health: 25" );
    CHECK_FALSE( message.interrupt );
}

TEST_CASE( "screen_reader_review_wraps_and_clamps_after_screen_changes", "[screen_reader]" )
{
    CHECK( screen_reader::next_review_index( 0, -1, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 2, 1, 3 ) == 0 );
    CHECK( screen_reader::next_review_index( 1, 1, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 100, 0, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 100, 1, 0 ) == 0 );
}
