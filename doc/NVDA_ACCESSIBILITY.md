# Windows screen reader mode

Windows tiles builds can send text directly to a running NVDA instance. Place
the official NVDA Controller Client DLL matching the game's architecture beside
`cataclysm-tiles.exe`, named `nvdaControllerClient.dll`. It is available from
<https://download.nvaccess.org/releases/stable/>. Keep the controller client's
LGPL license with any distributed DLL. NVDA itself does not have to be installed
by the game. Missing NVDA or a missing DLL must not prevent the game from running.

Enable Screen reader mode in Interface options, or press **Control+Shift+F12**
from any screen. The shortcut saves the setting and works before creating a
character. No sighted menu navigation is required. Speech is sent only while
the game is foreground on the normal Windows desktop.

| Shortcut | Command |
| --- | --- |
| Control+Shift+F12 | Toggle screen reader mode and save the setting |
| Control+Shift+F11 | Read the current screen's text |
| Control+Shift+F10 | Repeat the last announcement |
| Control+Shift+F9 | Stop game speech |
| Control+Shift+Up/Down | Review the preceding/following text entry; wraps at either end |
| Control+Shift+F8 | Open a text list of adjacent tiles and visible creatures |
| Control+Shift+F7 | Open numerical character health and condition information |

The commands besides the toggle apply only while screen reader mode is enabled.
Reading and reviewing do not consume game turns. The surroundings and status
lists use the usual list controls: up/down, filtering, and Escape to return.

## Maps and gameplay

The game announces position and tile descriptions when moving. Surroundings
lists compass directions, terrain, furniture, impassable tiles, visible items,
known traps, fields, vehicles, and visible creatures. Creature positions include
compass offsets, attitude, and level differences from the avatar, ordered nearest
first. Hidden terrain, creatures, traps, and container contents
must not be disclosed by accessibility descriptions.

The normal Look around action provides a movable cursor. Its spoken description
includes relative position and tile information. On the overmap, moving the
cursor announces the terrain known to the character, coordinates, and level.
The sidebar's descriptions, missions, notes, and key hints are available to screen
review. Map glyphs are suppressed from the speech stream.

## UI behavior

The active UI supplies a text snapshot. Entering a screen announces its text;
changing a selection announces the selected entry. Other changed text is spoken
without rereading unchanged frames or entries merely reordered by scrolling.
Text review is separate from game selection and does not activate controls.
Game messages are sent to NVDA as well as retained in the existing message log.
NVDA braille messages receive the same text where supported.

Both curses-port windows and ImGui text are captured. Custom colored-text and
list widgets supply their complete labels and selected-row descriptions. Option
names and values are available without visual truncation. Color markup and box
drawing are removed. The title screen substitutes a plain title for decorative
ASCII art only in screen reader mode. Normal visual mode keeps its existing UI.

## Validation before publishing

Run the `[screen_reader]` tests. With NVDA running, test the title screen, toggle
and persistence, every character-creation tab, menus, options, inventory and item
descriptions, confirmations, crafting, movement, combat/targeting, Look around,
overmap cursor/search/notes, health, surroundings, and message history. Test rapid
selection changes, long text, scrolling, canceling speech, focus loss, disabled
controls, hidden information, and a missing DLL/NVDA. Confirm normal visual mode
retains its controls and presentation.

This integration is not a Windows UI Automation provider. Complex custom
screens need user testing before claiming complete accessibility. Keep any
unverified or failing screens in the PR's test notes, and obtain NVDA user
feedback on the playable build before publishing the PR.
