#include "screen_reader.h"
#include "translations.h"

#if defined(_WIN32) && defined(TILES)
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace
{
unsigned int value_id = 0;
int value_frame = -1;
std::string widget_value;
}

void CataScreenReaderValue( unsigned int id, const char *begin, const char *end )
{
    if( screen_reader::enabled() && GImGui && ( GImGui->NavId == id || GImGui->ActiveId == id ) ) {
        value_id = id;
        value_frame = ImGui::GetFrameCount();
        widget_value.assign( begin, end );
    }
}

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

void CataScreenReaderState( unsigned int id, const char *label, int flags )
{
    if( !screen_reader::enabled() || !label || !GImGui ) {
        return;
    }
    const ImGuiContext &ctx = *GImGui;
    if( ctx.NavId != id && ctx.ActiveId != id ) {
        return;
    }
    std::string value( label, ImGui::FindRenderedTextEnd( label ) );
    if( flags & ImGuiItemStatusFlags_Checkable ) {
        value += " " + std::string( flags & ImGuiItemStatusFlags_Checked ? _( "checked" ) :
                                    _( "not checked" ) );
    }
    if( flags & ImGuiItemStatusFlags_Openable ) {
        value += " " + std::string( flags & ImGuiItemStatusFlags_Opened ? _( "expanded" ) :
                                    _( "collapsed" ) );
    }
    if( ImGuiInputTextState *state = ImGui::GetInputTextState( id ) ) {
        if( value.empty() ) {
            value = _( "Text input" );
        }
        if( !( state->Flags & ImGuiInputTextFlags_Password ) ) {
            value += ": " + std::string( state->GetText() );
        }
    } else if( value_id == id && value_frame == ImGui::GetFrameCount() ) {
        value += ": " + widget_value;
    }
    if( ctx.LastItemData.ItemFlags & ImGuiItemFlags_Disabled ) {
        value += " " + std::string( _( "unavailable" ) );
    }
    screen_reader::text( value, true );
}
#endif
