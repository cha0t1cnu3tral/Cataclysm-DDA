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
           std::vector<std::string>{ "hat" } );
    CHECK( screen_reader::changed_lines( { "Health: 80" }, { "Health: 25" } ) ==
           std::vector<std::string>{ "Health: 25" } );
}
