#include "screen_reader_tracker.h"
#include "screen_reader.h"
#include "screen_reader_game.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "avatar.h"
#include "cata_utility.h"
#include "coordinates.h"
#include "creature.h"
#include "debug.h"
#include "game.h"
#include "game_constants.h"
#include "input_context.h"
#include "input_enums.h"
#include "input_popup.h"
#include "item.h"
#include "line.h"
#include "map.h"
#include "map_iterator.h"
#include "mapdata.h"
#include "output.h"
#include "string_formatter.h"
#include "translations.h"
#include "ui_manager.h"
#include "uilist.h"
#include "vehicle.h"
#include "vpart_position.h"

namespace screen_reader
{
namespace
{
struct tracked_object {
    tripoint_bub_ms position;
    std::string name;
    std::vector<std::string> items;
    int distance;
    int approach_distance;
};
struct walk_destination {
    tripoint_abs_ms position;
    std::string name;
    int approach_distance;
};
std::optional<tripoint_abs_ms> selected_position;
std::string selected_name;
size_t selected_item = 0;
std::optional<walk_destination> walking;
std::string walk_stop_reason;

std::string bearing( const tripoint_bub_ms &position )
{
    const tripoint offset = ( position - get_avatar().pos_bub() ).raw();
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
    return result.empty() ? _( "Here" ) : result;
}

std::vector<tracked_object> visible_objects( tracker_kind kind )
{
    avatar &player = get_avatar();
    map &here = get_map();
    std::vector<tracked_object> objects;
    // Order the scan by distance before deduplicating vehicles, so their nearest
    // visible part becomes the approach target, rather than an arbitrary corner.
    std::vector<tripoint_bub_ms> positions;
    for( const tripoint_bub_ms &p : here.points_in_radius( player.pos_bub(), MAX_VIEW_DISTANCE, 0 ) ) {
        if( here.inbounds( p ) && player.sees( here, p ) ) {
            positions.push_back( p );
        }
    }
    std::stable_sort( positions.begin(), positions.end(), [&]( const auto & a, const auto & b ) {
        return rl_dist( player.pos_bub(), a ) < rl_dist( player.pos_bub(), b );
    } );
    std::unordered_set<const vehicle *> vehicles;
    for( const tripoint_bub_ms &p : positions ) {
        if( kind == tracker_kind::exits ) {
            std::string exit_name;
            if( here.has_flag( ter_furn_flag::TFLAG_GOES_UP, p ) ||
                here.has_flag( ter_furn_flag::TFLAG_GOES_DOWN, p ) ) {
                exit_name = here.has_flag( ter_furn_flag::TFLAG_GOES_UP, p ) ?
                            _( "Staircase or passage up" ) : _( "Staircase or passage down" );
            } else if( here.has_flag( ter_furn_flag::TFLAG_DOOR, p ) ) {
                exit_name = _( "Door" );
            } else if( here.is_outside( p ) && here.passable( p ) ) {
                // Only offer outdoor destinations beside a visible, walkable
                // indoor tile. Never infer a building's hidden layout.
                for( const tripoint_bub_ms &neighbor : here.points_in_radius( p, 1, 0 ) ) {
                    const tripoint offset = ( neighbor - p ).raw();
                    if( std::abs( offset.x ) + std::abs( offset.y ) == 1 &&
                        here.inbounds( neighbor ) && player.sees( here, neighbor ) &&
                        !here.is_outside( neighbor ) && here.passable( neighbor ) ) {
                        exit_name = _( "Outside of building" );
                        break;
                    }
                }
            }
            if( !exit_name.empty() ) {
                objects.push_back( { p, exit_name + ": " + describe_tile( p ), {},
                                     rl_dist( player.pos_bub(), p ), here.impassable( p ) ? 1 : 0 } );
            }
            continue;
        }
        if( kind == tracker_kind::hazards ) {
            if( here.can_see_trap_at( p, player ) || here.dangerous_field_at( p ) ) {
                objects.push_back( { p, describe_tile( p ), {}, rl_dist( player.pos_bub(), p ), 1 } );
            }
            continue;
        }
        std::vector<std::string> items;
        if( here.sees_some_items( p, player ) ) {
            for( const item &object : here.i_at( p ) ) {
                items.push_back( object.display_name() );
            }
        }
        std::string name;
        if( here.has_furn( p ) ) {
            name = here.furnname( p );
        } else if( here.has_flag( ter_furn_flag::TFLAG_DOOR, p ) ||
                   here.has_flag( ter_furn_flag::TFLAG_GOES_UP, p ) ||
                   here.has_flag( ter_furn_flag::TFLAG_GOES_DOWN, p ) ) {
            name = here.tername( p );
        } else if( !items.empty() ) {
            name = _( "Ground items" );
        }
        if( !name.empty() ) {
            objects.push_back( { p, name, items, rl_dist( player.pos_bub(), p ), here.impassable( p ) ? 1 : 0 } );
        }
        if( const optional_vpart_position vp = here.veh_at( p ) ) {
            const vehicle &veh = vp->vehicle();
            if( vehicles.insert( &veh ).second ) {
                objects.push_back( { p, veh.name, items, rl_dist( player.pos_bub(), p ), 1 } );
            }
        }
    }
    for( Creature &creature : g->all_creatures() ) {
        if( kind == tracker_kind::objects && &creature != &player && player.sees( here, creature ) &&
            creature.pos_bub().z() == player.pos_bub().z() ) {
            objects.push_back( { creature.pos_bub(), creature.disp_name() + " " +
                                 Creature::get_attitude_ui_data( creature.attitude_to( player ) ).first.translated(),
                                 {}, rl_dist( player.pos_bub(), creature.pos_bub() ), 1 } );
        }
    }
    std::stable_sort( objects.begin(), objects.end(), []( const auto & a, const auto & b ) {
        return a.distance < b.distance;
    } );
    return objects;
}
}

bool tracker_walk_active()
{
    return walking.has_value() && get_avatar().has_destination();
}

void cancel_tracker_walk( const std::string &reason )
{
    if( !enabled() || !walking ) {
        return;
    }
    get_avatar().abort_automove();
    walk_stop_reason = reason;
    g->invalidate_main_ui_adaptor();
}

void update_tracker_walk()
{
    if( !walking || !enabled() || get_avatar().has_destination() ) {
        return;
    }
    const walk_destination destination = *walking;
    walking.reset();
    const bool reached = rl_dist( get_avatar().pos_bub(), get_map().get_bub( destination.position ) ) <=
                         destination.approach_distance;
    const std::string report = !walk_stop_reason.empty() ? walk_stop_reason :
                               reached ? string_format( _( "Reached the location of %s." ), destination.name ) :
                               _( "Auto-walk stopped before reaching the target. You are back in control." );
    walk_stop_reason.clear();
    announce( report, true );
}

void object_tracker( int direction, bool items, tracker_kind kind )
{
    DebugLog( D_INFO, D_MAIN ) << "Screen reader tracker requested: direction=" << direction
                               << " items=" << items;
    if( !enabled() || !g || !g->w_terrain ) {
        return;
    }
    std::vector<tracked_object> objects = visible_objects( kind );
    if( objects.empty() ) {
        announce( kind == tracker_kind::exits ? _( "No visible doors, stairs, or outdoor exits found." ) :
                  kind == tracker_kind::hazards ? _( "No visible known traps or dangerous fields found." ) :
                  _( "No visible objects to track." ), true );
        return;
    }
    tracker_cursor cursor;
    const std::vector<tracked_object> all_objects = objects;
    std::string filter;
    std::string selection_notice;
    if( kind == tracker_kind::objects && selected_position ) {
        for( size_t i = 0; i < objects.size(); ++i ) {
            if( get_map().get_abs( objects[i].position ) == *selected_position &&
                objects[i].name == selected_name ) {
                cursor.object = i;
                if( items ) {
                    cursor.item = selected_item < objects[i].items.size() ? selected_item : 0;
                    cursor.cycle_items( direction, objects[i].items.size() );
                } else {
                    cursor.cycle_objects( direction, objects.size() );
                }
                break;
            }
        }
    }
    catacurses::window window;
    ui_adaptor ui;
    ui.on_screen_resize( [&]( ui_adaptor & adaptor ) {
        window = new_centered_win( TERMY, TERMX );
        adaptor.position_from_window( window );
    } );
    ui.on_redraw( [&]( const ui_adaptor & ) {
        const tracked_object &object = objects[cursor.object];
        const std::string summary = string_format( _( "Object %d of %d: %s. %s" ),
                                    cursor.object + 1, objects.size(), object.name, bearing( object.position ) );
        const std::string item_text = object.items.empty() ? _( "No visible ground items here." ) :
                                      string_format( _( "Item %d of %d: %s" ), cursor.item + 1, object.items.size(),
                                          object.items[cursor.item] );
        werase( window );
        draw_border( window );
        center_print( window, 0, c_white,
                      kind == tracker_kind::exits ? _( "Exit finder: stairs, doors, and outdoors" ) :
                      kind == tracker_kind::hazards ? _( "Hazard finder" ) : _( "Object tracker" ) );
        fold_and_print( window, point( 2, 2 ), getmaxx( window ) - 4, c_white, summary );
        fold_and_print( window, point( 2, 6 ), getmaxx( window ) - 4, c_white, item_text );
        fold_and_print( window, point( 2, 10 ), getmaxx( window ) - 4, c_light_gray,
                        _( "Control+[ / Control+]: previous / next object. [ / ]: previous / next item. Slash: search by object or item name. Enter: auto-walk to the selected object. Escape: close. Browsing uses no turns." ) );
        wnoutrefresh( window );
        text( summary + ". " + item_text, true );
    } );
    ui.mark_resize();
    input_context ctxt( "SCREEN_READER_TRACKER", keyboard_mode::keycode );
    ctxt.register_action( "TRACKER_PREVIOUS_OBJECT" );
    ctxt.register_action( "TRACKER_NEXT_OBJECT" );
    ctxt.register_action( "TRACKER_PREVIOUS_ITEM" );
    ctxt.register_action( "TRACKER_NEXT_ITEM" );
    ctxt.register_action( "CONFIRM" );
    ctxt.register_action( "QUIT" );
    ctxt.register_action( "HELP_KEYBINDINGS" );
    ctxt.register_action( "FILTER" );
    bool announce_selection = true;
    while( enabled() ) {
        selected_position = get_map().get_abs( objects[cursor.object].position );
        selected_name = objects[cursor.object].name;
        selected_item = cursor.item;
        ui_manager::redraw();
        if( announce_selection ) {
            const tracked_object &object = objects[cursor.object];
            const std::string item_text = object.items.empty() ? _( "No visible ground items here." ) :
                                          string_format( _( "Item %d of %d: %s" ), cursor.item + 1, object.items.size(),
                                              object.items[cursor.item] );
            const std::string selection = selection_notice + string_format( _( "Object %d of %d: %s. %s. %s" ),
                                          cursor.object + 1, objects.size(), object.name, bearing( object.position ), item_text );
            DebugLog( D_INFO, D_MAIN ) << "Screen reader tracker selection: " << selection;
            // Navigation must speak even when a renderer skips a frame, the
            // UI snapshot has not changed, or an empty item list is selected.
            announce( selection, true );
            selection_notice.clear();
            announce_selection = false;
        }
        const std::string action = ctxt.handle_input( 100 );
        if( action == "QUIT" ) {
            return;
        } else if( action == "TRACKER_PREVIOUS_OBJECT" || action == "TRACKER_NEXT_OBJECT" ) {
            cursor.cycle_objects( action == "TRACKER_PREVIOUS_OBJECT" ? -1 : 1, objects.size() );
            announce_selection = true;
        } else if( action == "TRACKER_PREVIOUS_ITEM" || action == "TRACKER_NEXT_ITEM" ) {
            cursor.cycle_items( action == "TRACKER_PREVIOUS_ITEM" ? -1 : 1,
                                objects[cursor.object].items.size() );
            announce_selection = true;
        } else if( action == "FILTER" ) {
            string_input_popup_imgui popup( FULL_SCREEN_WIDTH, filter );
            popup.set_label( _( "Search visible objects and ground items:" ) );
            popup.set_description(
                _( "Type part of a name. Leave blank to show all objects. Enter applies; Escape cancels." ) );
            const std::string query = popup.query();
            if( !popup.cancelled() ) {
                std::vector<tracked_object> matches;
                for( const tracked_object &object : all_objects ) {
                    if( query.empty() || lcmatch( object.name, query ) ||
                    std::any_of( object.items.begin(), object.items.end(), [&]( const std::string & item ) {
                    return lcmatch( item, query );
                    } ) ) {
                        matches.push_back( object );
                    }
                }
                if( matches.empty() ) {
                    selection_notice = _( "No matching visible objects. Previous list retained. " );
                } else {
                    objects = std::move( matches );
                    filter = query;
                    cursor = {};
                }
                announce_selection = true;
            }
        } else if( action == "CONFIRM" ) {
            const tracked_object &object = objects[cursor.object];
            bool reported = false;
            const auto route = g->safe_route_to( get_avatar(), object.position, object.approach_distance,
            [&]( const std::string & message ) {
                announce( message, true );
                reported = true;
            } );
            if( route && !route->empty() ) {
                get_avatar().set_destination( *route );
                walking = walk_destination{ *selected_position, object.name, object.approach_distance };
                walk_stop_reason.clear();
                announce( string_format(
                              _( "Auto-walking toward %s. Normal safe-mode and interruption rules apply." ),
                              object.name ), true );
                return;
            }
            if( !reported ) {
                announce( string_format( _( "Already within reach of %s." ), object.name ), true );
            }
        }
    }
}
}
