#include "screen_reader_game.h"
#include "screen_reader.h"

#include <array>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "avatar.h"
#include "bodypart.h"
#include "creature.h"
#include "creature_tracker.h"
#include "display.h"
#include "field.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "map_iterator.h"
#include "string_formatter.h"
#include "translations.h"
#include "trap.h"
#include "uilist.h"
#include "units.h"
#include "vehicle.h"
#include "vpart_position.h"

namespace screen_reader
{
namespace
{
std::string relative_position( const tripoint &offset )
{
    std::string result;
    if( offset.x != 0 ) {
        result = string_format( offset.x > 0 ? _( "%d east" ) : _( "%d west" ), std::abs( offset.x ) );
    }
    if( offset.y != 0 ) {
        if( !result.empty() ) {
            result += ", ";
        }
        result += string_format( offset.y > 0 ? _( "%d south" ) : _( "%d north" ), std::abs( offset.y ) );
    }
    if( offset.z != 0 ) {
        if( !result.empty() ) {
            result += ", ";
        }
        result += string_format( offset.z > 0 ? _( "%d levels above" ) : _( "%d levels below" ),
                                 std::abs( offset.z ) );
    }
    return result.empty() ? _( "Here" ) : result;
}
}

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
            result += ", " + creature->disp_name() + " " +
                      Creature::get_attitude_ui_data( creature->attitude_to( player ) ).first.translated();
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
    menu.text =
        _( "Use up and down to review. Escape returns to the game. These commands do not use a turn." );
    const std::array<point, 9> offsets = { point( 0, 0 ), point( 0, -1 ), point( 1, -1 ),
                                           point( 1, 0 ), point( 1, 1 ), point( 0, 1 ),
                                           point( -1, 1 ), point( -1, 0 ), point( -1, -1 )
                                         };
    const std::array<const char *, 9> labels = { _( "Here" ), _( "North" ), _( "Northeast" ),
                                                 _( "East" ), _( "Southeast" ), _( "South" ),
                                                 _( "Southwest" ), _( "West" ), _( "Northwest" )
                                               };
    for( size_t i = 0; i < offsets.size(); ++i ) {
        menu.addentry( static_cast<int>( i ), true, MENU_AUTOASSIGN,
                       std::string( labels[i] ) + ": " + describe_tile( player.pos_bub() + offsets[i] ) );
    }
    std::vector<std::pair<int, std::string>> visible_creatures;
    for( Creature &creature : g->all_creatures() ) {
        if( &creature == &player || !player.sees( here, creature ) ) {
            continue;
        }
        const tripoint delta = ( creature.pos_bub() - player.pos_bub() ).raw();
        visible_creatures.emplace_back( std::max( std::abs( delta.x ), std::abs( delta.y ) ),
                                        creature.disp_name() + " " +
                                        Creature::get_attitude_ui_data( creature.attitude_to( player ) ).first.translated() +
                                        ": " + relative_position( delta ) );
    }
    std::sort( visible_creatures.begin(), visible_creatures.end() );
    for( const auto &entry : visible_creatures ) {
        menu.addentry( static_cast<int>( menu.entries.size() ), true, MENU_AUTOASSIGN, entry.second );
    }
    menu.query();
}

void status( bool read_all )
{
    if( !enabled() || !g || !g->w_terrain ) {
        announce( _( "Character status is available during a game." ), true );
        return;
    }
    avatar &player = get_avatar();
    uilist menu;
    menu.title = _( "Character stats" );
    const std::string hunger = display::hunger_text_color( player ).first;
    const std::string thirst = display::thirst_text_color( player ).first;
    const std::string sleepiness = display::sleepiness_text_color( player ).first;
    std::vector<std::string> stats = {
        string_format( _( "Strength: %d, base %d" ), player.get_str(), player.get_str_base() ),
        string_format( _( "Dexterity: %d, base %d" ), player.get_dex(), player.get_dex_base() ),
        string_format( _( "Intelligence: %d, base %d" ), player.get_int(), player.get_int_base() ),
        string_format( _( "Perception: %d, base %d" ), player.get_per(), player.get_per_base() ),
        string_format( _( "Stamina: %d of %d" ), player.get_stamina(), player.get_stamina_max() ),
        string_format( _( "Speed: %d. Focus: %d. Morale: %d" ), player.get_speed(),
                       player.get_focus(), player.get_morale_level() ),
        string_format( _( "Hunger: %s. Thirst: %s. Sleepiness: %s. Pain: %d" ),
                       hunger.empty() ? _( "Not hungry" ) : hunger,
                       thirst.empty() ? _( "Not thirsty" ) : thirst,
                       sleepiness.empty() ? _( "Not tired" ) : sleepiness, player.get_perceived_pain() ),
        string_format( _( "Weariness: %s. Radiation: %d" ),
                       display::weariness_text_color( player ).first, player.get_rad() ),
        string_format( _( "Bionic power: %d of %d kilojoules" ),
                       units::to_kilojoule( player.get_power_level() ), units::to_kilojoule( player.get_max_power_level() ) )
    };
    for( const bodypart_id &part : player.get_all_body_parts( get_body_part_flags::only_main ) ) {
        stats.push_back( string_format( _( "%s: health %d of %d" ), part->name.translated(),
                                        player.get_part_hp_cur( part ), player.get_part_hp_max( part ) ) );
    }
    if( read_all ) {
        std::string report = _( "Character stats." );
        for( const std::string &stat : stats ) {
            report += " " + stat + ".";
        }
        announce( report, true );
        return;
    }
    for( const std::string &stat : stats ) {
        menu.addentry( static_cast<int>( menu.entries.size() ), true, MENU_AUTOASSIGN, stat );
    }
    menu.query();
}
} // namespace screen_reader
