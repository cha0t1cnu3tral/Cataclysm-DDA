#pragma once
#ifndef CATA_SRC_SCREEN_READER_H
#define CATA_SRC_SCREEN_READER_H

#include <cstddef>
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
struct speech_update {
    std::string text;
    bool interrupt = false;
};
speech_update describe_changes( const std::vector<std::string> &before,
                                const std::vector<std::string> &after,
                                const std::string &old_focus, const std::string &new_focus,
                                bool entering );
size_t next_review_index( size_t current, int direction, size_t count );
bool enabled();
void begin_ui( const void *identity, bool on_top );
void end_ui();
void finish_frame();
void capture_window( const catacurses::window &window );
void text( const std::string &value, bool focused = false );
// Prefer one complete semantic selection over fragments captured by widgets.
void selection( const std::string &value );
class capture_pause
{
    public:
        capture_pause();
        ~capture_pause();
        capture_pause( const capture_pause & ) = delete;
        capture_pause &operator=( const capture_pause & ) = delete;
};
void announce( const std::string &value, bool interrupt = false );
void toggle();
void read_screen();
void repeat();
void review( int direction );
void stop();
void surroundings();
void status( bool read_all = false );
}
#endif
