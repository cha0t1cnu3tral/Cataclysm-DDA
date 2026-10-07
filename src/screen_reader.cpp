#include "screen_reader.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

#include "options.h"
#include "translations.h"

#if defined(_WIN32) && defined(TILES)
#include <windows.h>
#include "cursesdef.h"
#include "cursesport.h"
#include "game.h"
#endif

namespace screen_reader
{
bool enabled()
{
    return has_option( "SCREEN_READER_MODE" ) && get_option<bool>( "SCREEN_READER_MODE" );
}

namespace
{
bool collecting = false;
const void *current_ui = nullptr;
const void *previous_ui = nullptr;
std::vector<std::string> frame;
std::vector<std::string> previous_frame;
std::vector<std::string> focused;
std::string previous_focus;
std::string last_speech;
size_t review_index = 0;
bool frame_ready = false;

std::string join( const std::vector<std::string> &lines )
{
    std::string result;
    for( const std::string &line : lines ) {
        if( !result.empty() ) {
            result += ". ";
        }
        result += line;
    }
    return result;
}

#if defined(_WIN32) && defined(TILES)
class nvda_client
{
    private:
        using text_fn = long ( WINAPI * )( const wchar_t * );
        using void_fn = long ( WINAPI * )();
        HMODULE library = nullptr;
        text_fn speak = nullptr;
        text_fn braille = nullptr;
        void_fn cancel = nullptr;
        void_fn test = nullptr;

        bool safe_to_speak() const {
            DWORD process = 0;
            GetWindowThreadProcessId( GetForegroundWindow(), &process );
            if( process != GetCurrentProcessId() ) {
                return false;
            }
            HDESK desktop = OpenInputDesktop( 0, FALSE, DESKTOP_READOBJECTS );
            if( !desktop ) {
                return false;
            }
            wchar_t name[256] = {};
            DWORD needed = 0;
            const bool safe = GetUserObjectInformationW( desktop, UOI_NAME, name,
                              sizeof( name ), &needed ) && wcscmp( name, L"Default" ) == 0;
            CloseDesktop( desktop );
            return safe;
        }

        bool load() {
            if( library ) {
                return speak && cancel && test;
            }
            // Only load the companion DLL from the executable directory, never PATH/CWD.
            std::vector<wchar_t> filename( 32768 );
            const DWORD length = GetModuleFileNameW( nullptr, filename.data(), filename.size() );
            if( length == 0 || length >= filename.size() ) {
                return false;
            }
            std::wstring path( filename.data(), length );
            const size_t slash = path.find_last_of( L"\\/" );
            if( slash == std::wstring::npos ) {
                return false;
            }
            path.resize( slash + 1 );
            path += L"nvdaControllerClient.dll";
            library = LoadLibraryExW( path.c_str(), nullptr,
                                      LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 );
            if( !library ) {
                return false;
            }
            speak = reinterpret_cast<text_fn>( GetProcAddress( library, "nvdaController_speakText" ) );
            braille = reinterpret_cast<text_fn>( GetProcAddress( library, "nvdaController_brailleMessage" ) );
            cancel = reinterpret_cast<void_fn>( GetProcAddress( library, "nvdaController_cancelSpeech" ) );
            test = reinterpret_cast<void_fn>( GetProcAddress( library, "nvdaController_testIfRunning" ) );
            return speak && cancel && test;
        }

    public:
        ~nvda_client() {
            if( library ) {
                FreeLibrary( library );
            }
        }

        void say( const std::string &value, bool interrupt ) {
            if( !safe_to_speak() || !load() || test() != 0 ) {
                return;
            }
            const int length = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                                    static_cast<int>( value.size() ), nullptr, 0 );
            if( length <= 0 ) {
                return;
            }
            std::wstring wide( length, L'\0' );
            MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                 static_cast<int>( value.size() ), wide.data(), length );
            if( interrupt ) {
                cancel();
            }
            speak( wide.c_str() );
            if( braille ) {
                braille( wide.c_str() );
            }
        }

        void silence() {
            if( safe_to_speak() && load() && test() == 0 ) {
                cancel();
            }
        }
};
nvda_client client;
#endif
} // namespace

void announce( const std::string &value, bool interrupt )
{
    if( !enabled() ) {
        return;
    }
    const std::string clean = readable_text( value );
    if( clean.empty() ) {
        return;
    }
    last_speech = clean;
#if defined(_WIN32) && defined(TILES)
    client.say( clean, interrupt );
#endif
}

void begin_ui( const void *identity, bool on_top )
{
    collecting = enabled() && on_top;
    if( collecting ) {
        current_ui = identity;
        frame.clear();
        focused.clear();
        frame_ready = true;
    }
}

void end_ui()
{
    collecting = false;
}

void text( const std::string &value, bool is_focused )
{
    if( !collecting ) {
        return;
    }
    const std::string clean = readable_text( value );
    if( clean.empty() ) {
        return;
    }
    if( std::find( frame.begin(), frame.end(), clean ) == frame.end() ) {
        frame.push_back( clean );
    }
    if( is_focused && std::find( focused.begin(), focused.end(), clean ) == focused.end() ) {
        focused.push_back( clean );
    }
}

void finish_frame()
{
    if( !enabled() || !frame_ready ) {
        return;
    }
    frame_ready = false;
    const std::string focus = join( focused );
    const bool new_ui = current_ui != previous_ui;
    const speech_update update = describe_changes( previous_frame, frame, previous_focus, focus,
                                 new_ui );
    announce( update.text, update.interrupt );
    if( new_ui ) {
        review_index = 0;
    }
    previous_ui = current_ui;
    previous_frame = frame;
    previous_focus = focus;
    if( !frame.empty() ) {
        review_index = std::min( review_index, frame.size() - 1 );
    }
}

void capture_window( const catacurses::window &window )
{
#if defined(_WIN32) && defined(TILES)
    if( !collecting ) {
        return;
    }
    const auto *win = window.get<cata_cursesport::WINDOW>();
    if( !win ) {
        return;
    }
    // Graphical map symbols are replaced by semantic descriptions, never spoken as ASCII.
    if( g && ( window == g->w_terrain || window == g->w_overmap || window == g->w_minimap ) ) {
        return;
    }
    for( const cata_cursesport::curseline &line : win->line ) {
        std::string row;
        std::string selection;
        for( const cata_cursesport::cursecell &cell : line.chars ) {
            row += cell.ch;
            if( cell.BG != catacurses::black ) {
                selection += cell.ch;
            }
        }
        text( row );
        if( !selection.empty() ) {
            text( selection, true );
        }
    }
#else
    ( void )window;
#endif
}

void toggle()
{
    const bool turn_on = !enabled();
    if( !turn_on ) {
        announce( _( "Screen reader mode off." ), true );
    }
    get_options().get_option( "SCREEN_READER_MODE" ).setValue( std::string(
                turn_on ? "true" : "false" ) );
    get_options().save();
    previous_ui = nullptr;
    previous_frame.clear();
    previous_focus.clear();
    if( turn_on ) {
        announce( _( "Screen reader mode on. Control Shift F11 reads the screen. Control Shift F10 repeats speech. Control Shift up and down review text. Control Shift F9 stops speech." ),
                  true );
    }
}

void read_screen()
{
    announce( join( previous_frame ), true );
}

void repeat()
{
    announce( last_speech, true );
}

void review( int direction )
{
    if( previous_frame.empty() ) {
        announce( _( "No screen text available." ), true );
        return;
    }
    review_index = next_review_index( review_index, direction, previous_frame.size() );
    announce( previous_frame[review_index], true );
}

void stop()
{
#if defined(_WIN32) && defined(TILES)
    client.silence();
#endif
}
} // namespace screen_reader
