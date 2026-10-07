#include "screen_reader_game.h"
#include "screen_reader.h"

#include <array>
#include <string>

#include "avatar.h"
#include "bodypart.h"
#include "creature.h"
#include "creature_tracker.h"
#include "field.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "map_iterator.h"
#include "string_formatter.h"
#include "translations.h"
#include "trap.h"
#include "uilist.h"
#include "vehicle.h"
#include "vpart_position.h"

namespace screen_reader
{
std::string describe_tile( const tripoint_bub_ms &position )
{
    map &here = get_map();
    avatar &player = get_avatar();
    if( position != player.pos_bub() && !player.sees( here, position ) ) {
        return _( "Not visible." );
    }
    std::string result = here.tername( position );
    if( here.has_furn( position ) ) {
        result += ", " + here.furnname( position );
    }
    if( here.impassable( position ) ) {
        result += ", " + std::string( _( "impassable" ) );
    }
    if( const optional_vpart_position vehicle = here.veh_at( position ) ) {
        result += ", " + vehicle->vehicle().name;
    }
    if( here.can_see_trap_at( position, player ) ) {
        result += ", " + here.tr_at( position ).name();
    }
    for( const auto &entry : here.field_at( position ) ) {
        result += ", " + entry.second.name();
    }
    if( const Creature *creature = get_creature_tracker().creature_at<Creature>( position ) ) {
        if( creature != &player && player.sees( here, *creature ) ) {
            result += ", " + creature->disp_name();
        }
    }
    if( here.sees_some_items( position, player ) ) {
        const map_stack items = here.i_at( position );
        size_t count = 0;
        for( const item &object : items ) {
            if( count++ == 5 ) {
                result += ", " + std::string( _( "more items" ) );
                break;
            }
            result += ", " + object.tname();
        }
    }
    return result;
}

void surroundings()
{
    if( !enabled() || !g || !g->w_terrain ) {
        announce( _( "Surroundings are available during a game." ), true );
        return;
    }
    avatar &player = get_avatar();
    map &here = get_map();
    uilist menu;
    menu.title = _( "Surroundings" );
    menu.text = _( "Use up and down to review. Escape returns to the game. These commands do not use a turn." );
    const std::array<point, 9> offsets = { point( 0, 0 ), point( 0, -1 ), point( 1, -1 ),
                                         point( 1, 0 ), point( 1, 1 ), point( 0, 1 ),
                                         point( -1, 1 ), point( -1, 0 ), point( -1, -1 ) };
    const std::array<const char *, 9> labels = { _( "Here" ), _( "North" ), _( "Northeast" ),
                                              _( "East" ), _( "Southeast" ), _( "South" ),
                                              _( "Southwest" ), _( "West" ), _( "Northwest" ) };
    for( size_t i = 0; i < offsets.size(); ++i ) {
        menu.addentry( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                       std::string( labels[i] ) + ": " + describe_tile( player.pos_bub() + offsets[i] ) );
    }
    for( Creature &creature : g->all_creatures() ) {
        if( &creature == &player || !player.sees( here, creature ) ) {
            continue;
        }
        const tripoint delta = ( creature.pos_bub() - player.pos_bub() ).raw();
        menu.addentry( static_cast<int>( menu.entries.size() ), true, MENU_AUTOASSIGN,
                       string_format( _( "%s: %d east, %d south, %d levels above" ),
                                      creature.disp_name(), delta.x, delta.y, delta.z ) );
    }
    menu.query();
}

void status()
{
    if( !enabled() || !g || !g->w_terrain ) {
        announce( _( "Character status is available during a game." ), true );
        return;
    }
    avatar &player = get_avatar();
    uilist menu;
    menu.title = _( "Character status" );
    menu.addentry( 0, true, MENU_AUTOASSIGN, string_format( _( "Stamina: %d of %d" ),
                   player.get_stamina(), player.get_stamina_max() ) );
    menu.addentry( 1, true, MENU_AUTOASSIGN, string_format( _( "Hunger: %d. Thirst: %d. Sleepiness: %d. Pain: %d" ),
                   player.get_hunger(), player.get_thirst(), player.get_sleepiness(), player.get_perceived_pain() ) );
    for( const bodypart_id &part : player.get_all_body_parts( get_body_part_flags::only_main ) ) {
        menu.addentry( static_cast<int>( menu.entries.size() ), true, MENU_AUTOASSIGN,
                       string_format( _( "%s: health %d of %d" ), part->name.translated(),
                                      player.get_part_hp_cur( part ), player.get_part_hp_max( part ) ) );
    }
    menu.query();
}
} // namespace screen_reader
