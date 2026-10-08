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
    CHECK( entry.text == "knife" );
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

TEST_CASE( "screen_reader_menu_focus_ignores_surrounding_layout_changes", "[screen_reader]" )
{
    const std::vector<std::string> before = { "Choose", "Knife", "Coat", "Description" };
    const std::vector<std::string> after = { "Choose", "Knife", "Coat", "A longer description" };
    CHECK( screen_reader::describe_changes( before, after, "Knife", "Knife", false ).text.empty() );
    CHECK( screen_reader::describe_changes( {}, after, "", "Coat. 2 of 2",
                                            true ).text == "Coat. 2 of 2" );
    CHECK( screen_reader::describe_changes( before, after, "Knife", "Coat", false ).text == "Coat" );
    CHECK( screen_reader::describe_changes( {}, { "Confirm quitting?" }, "", "", true ).text ==
           "Confirm quitting?" );
}

TEST_CASE( "screen_reader_narration_precedes_response_choices_without_repeating",
           "[screen_reader]" )
{
    const screen_reader::speech_update quiet;
    const auto next_line = screen_reader::dialogue_speech( quiet, "Alex: Please help me.", "Yes",
                           true );
    CHECK( next_line.text == "Alex: Please help me.. Yes" );
    CHECK( next_line.interrupt );
    CHECK( screen_reader::dialogue_speech( quiet, "Alex: Please help me.", "Yes",
                                           false ).text.empty() );
    const screen_reader::speech_update choice { "No", true };
    CHECK( screen_reader::dialogue_speech( choice, "Alex: Please help me.", "No",
                                           false ).text == "No" );
    CHECK( screen_reader::dialogue_speech( quiet, "Alex: Goodbye", "", true ).text == "Alex: Goodbye" );
    CHECK( screen_reader::dialogue_speech( choice, "", "No", true ).text == "No" );
}

TEST_CASE( "screen_reader_popup_instructions_do_not_repeat_wrapped_fragments", "[screen_reader]" )
{
    const std::vector<std::string> lines = { "ESC, q, Q, or SPACE to go back.",
                                             "ESC, q, Q, or SPACE", "RETURN or KEYPAD_ENTER to move anyway.", "RETURN or KEYPAD_ENTER"
                                           };
    CHECK( screen_reader::describe_changes( {}, lines, "", "", true ).text ==
           "ESC, q, Q, or SPACE to go back.. RETURN or KEYPAD_ENTER to move anyway." );
    CHECK( screen_reader::describe_changes( { lines[0], lines[2] }, lines, "", "",
                                            false ).text.empty() );
    CHECK( screen_reader::describe_changes( {}, { "Health 86", "8" }, "", "",
                                            true ).text == "Health 86. 8" );
    CHECK( screen_reader::describe_changes( {}, { "Active", "Actively moving" }, "", "", true ).text ==
           "Active. Actively moving" );
}

TEST_CASE( "screen_reader_review_wraps_and_clamps_after_screen_changes", "[screen_reader]" )
{
    CHECK( screen_reader::next_review_index( 0, -1, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 2, 1, 3 ) == 0 );
    CHECK( screen_reader::next_review_index( 1, 1, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 100, 0, 3 ) == 2 );
    CHECK( screen_reader::next_review_index( 100, 1, 0 ) == 0 );
}
