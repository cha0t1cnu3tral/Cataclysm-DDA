#include "screen_reader.h"
#include "translations.h"

#if defined(_WIN32) && defined(TILES)
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

// Small integration hooks keep accessibility independent of widget implementations.
void CataScreenReaderText( const char *begin, const char *end )
{
    if( !screen_reader::enabled() || !begin ) {
        return;
    }
    if( !end ) {
        end = ImGui::FindRenderedTextEnd( begin );
    }
    screen_reader::text( std::string( begin, end ) );
}

void CataScreenReaderItem( unsigned int id, const char *label, bool selected )
{
    if( !screen_reader::enabled() || !label || !GImGui ) {
        return;
    }
    const ImGuiContext &ctx = *GImGui;
    if( selected || ctx.NavId == id || ctx.ActiveId == id ) {
        const char *end = ImGui::FindRenderedTextEnd( label );
        std::string value( label, end );
        if( ctx.LastItemData.ItemFlags & ImGuiItemFlags_Disabled ) {
            value += " " + std::string( _( "unavailable" ) );
        }
        screen_reader::text( value, true );
    }
}
#endif
