#include "screen_reader.h"

#include <algorithm>
#include <cctype>

namespace screen_reader
{
std::string readable_text( const std::string &text )
{
    std::string result;
    bool space = false;
    bool has_word = false;
    for( size_t i = 0; i < text.size(); ++i ) {
        // Unicode box-drawing characters carry layout, not speech content.
        if( i + 2 < text.size() && static_cast<unsigned char>( text[i] ) == 0xe2 &&
            ( static_cast<unsigned char>( text[i + 1] ) == 0x94 ||
              static_cast<unsigned char>( text[i + 1] ) == 0x95 ) &&
            static_cast<unsigned char>( text[i + 2] ) >= 0x80 &&
            static_cast<unsigned char>( text[i + 2] ) <= 0xbf ) {
            i += 2;
            space = !result.empty();
            continue;
        }
        // Strip game color tags, preserving meaningful angle-bracket text.
        if( text.compare( i, 7, "<color_" ) == 0 || text.compare( i, 8, "</color>" ) == 0 ) {
            const size_t end = text.find( '>', i );
            if( end != std::string::npos ) {
                i = end;
                continue;
            }
        }
        const unsigned char ch = static_cast<unsigned char>( text[i] );
        if( std::isspace( ch ) ) {
            space = !result.empty();
        } else if( ch >= 32 ) {
            if( space ) {
                result += ' ';
                space = false;
            }
            result += static_cast<char>( ch );
            has_word = has_word || std::isalnum( ch ) || ch >= 128;
        }
    }
    return has_word ? result : std::string();
}

std::vector<std::string> changed_lines( const std::vector<std::string> &before,
                                      const std::vector<std::string> &after )
{
    std::vector<std::string> result;
    for( const std::string &line : after ) {
        if( !line.empty() && std::find( before.begin(), before.end(), line ) == before.end() &&
            std::find( result.begin(), result.end(), line ) == result.end() ) {
            result.push_back( line );
        }
    }
    return result;
}

} // namespace screen_reader
