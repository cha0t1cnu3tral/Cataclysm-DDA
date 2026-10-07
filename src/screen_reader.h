#pragma once
#ifndef CATA_SRC_SCREEN_READER_H
#define CATA_SRC_SCREEN_READER_H

#include <string>
#include <vector>

namespace catacurses
{
class window;
}

namespace screen_reader
{
// The presentation layer is independent of NVDA so it can be tested on all platforms.
std::string readable_text( const std::string &text );
std::vector<std::string> changed_lines( const std::vector<std::string> &before,
                                      const std::vector<std::string> &after );
bool enabled();
void begin_ui( const void *identity, bool on_top );
void end_ui();
void finish_frame();
void capture_window( const catacurses::window &window );
void text( const std::string &value, bool focused = false );
void announce( const std::string &value, bool interrupt = false );
void toggle();
void read_screen();
void repeat();
void review( int direction );
void stop();
void surroundings();
void status();
}
#endif
