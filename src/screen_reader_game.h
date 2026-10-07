#pragma once
#ifndef CATA_SRC_SCREEN_READER_GAME_H
#define CATA_SRC_SCREEN_READER_GAME_H
#include <string>
#include "coordinates.h"

namespace screen_reader
{
// These descriptions expose only information the avatar can perceive.
std::string describe_tile( const tripoint_bub_ms &position );
}
#endif
