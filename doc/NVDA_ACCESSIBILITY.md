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
| Control+Shift+F7 | Browse character stats, needs, stamina, bionic power, and body-part health |
| Control+Shift+F5 | Exit finder: browse stairs, doors, and outdoor destinations |
| Control+Shift+F4 | Hazard finder: browse visible known traps and dangerous fields |
| Control+Shift+F6 | Read the complete character stats report without opening a menu |
| Control+[ / Control+] | Open the object tracker from gameplay; browse the previous/next object inside it |
| [ / ] | Open the tracker from gameplay; browse previous/next visible ground item at the selected object |
| Enter / Escape | Inside the tracker, start auto-walking to the selected object / close without walking |

The commands besides the toggle apply only while screen reader mode is enabled.
Reading and reviewing do not consume game turns. The surroundings and status
lists use the usual list controls: up/down, filtering, and Escape to return.

## Maps and gameplay

The object tracker lists visible furniture, doors, stairs, ground-item piles,
vehicles (once each, at the nearest visible part), and creatures on the current
level, ordered nearest first. It announces the selected object's bearing and the
selected item's name and quantity. Object and item browsing wrap independently;
switching objects resets the item selection. Slash opens a search by object or ground-item name; an empty search restores all visible objects. Searches never expose hidden objects. Browsing does not consume turns.
In screen reader mode, these four shortcuts take priority over conflicting
gameplay bindings. Other menus and text entry retain their bracket controls.
Both character and keycode keyboard input are supported. Turning screen reader
mode off restores the existing gameplay bindings.

Enter uses the game's existing safe auto-move route. Accessible targets are
approached directly; impassable objects, vehicles, and creatures are approached
to within one tile. Unreachable routes are reported without moving. Walking uses
normal turns, safe-mode interruptions, and cancellation controls. Selecting an
item walks to its object's location; it does not automatically pick it up.
The game announces arrival or a stopped route. Moving creatures are tracked to
their selected location rather than chased. Turning screen reader mode off closes
the tracker. No tracker controls are registered in normal visual mode.

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
Health and stamina bars become readable percentages in screen reader mode;
the character-status command additionally provides exact health values.
Game messages are sent to NVDA as well as retained in the existing message log.
NVDA braille messages receive the same text where supported.

Both curses-port windows and ImGui text are captured. Custom colored-text and
list widgets supply their complete labels and selected-row descriptions. Option
names and values are available without visual truncation. Color markup and box
drawing are removed. The title screen substitutes a plain title for decorative
ASCII art only in screen reader mode. Normal visual mode keeps its existing UI.

## Validation before publishing

Character creation uses Tab and Shift+Tab to switch tabs, Up/Down to browse,
Enter to choose or toggle, and plus/minus to adjust stats and skills. F2 opens
a keyboard list of name, gender, outfit, age, height, blood type, starting
location, calendar, randomization, and template controls. Current values and
selected states are spoken. Tab announcements are brief; the F2 menu includes keyboard help on demand. Control+Shift+F11 reads the detail panel as well as
the list; line review includes scenario/profession descriptions and summary
information. On Summary, Tab opens the finish confirmation.

Conversations announce newly added dialogue, including the speaker, before
the selected response. Moving between responses reads the response alone;
it does not repeat the conversation history. Full-screen review retains that
history.

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

Menu navigation speaks the selected option and its state, rather than reading every visible row or changing tooltip. List menus include the selected position. Control+Shift+F11 reads the full screen, including longer descriptions; Control+Shift+Up/Down reviews it line by line.

Movement speech describes the current tile without rereading the sidebar. Game messages still speak as they occur. Control+Shift+F6 reads current/base attributes, stamina, speed, focus, morale, needs, pain, weariness, radiation, bionic power, and body-part health. Control+Shift+F7 opens the same stats as a browsable list. Neither stats command consumes a turn.

Confirmation dialogs speak the current Yes, No, or other button with its key hint. Left/Right changes the spoken selection; hovering a button reads it too. Activating a button by Enter, mouse click, or its shortcut announces the chosen response. This applies only in screen reader mode.

## Exit and hazard finders

During gameplay, Control+Shift+F5 opens visible doors, stairs, and outdoor destinations on the current
level, nearest first. Outdoor destinations are walkable outdoor tiles beside a
visible walkable indoor tile; Enter auto-walks onto the outdoor destination. Each candidate includes a tile description and compass
offsets. Doors are possible exits: a door can lead to another room, be locked,
or require opening. The finder does not claim a route leads outdoors and does
not reveal hidden exits. Staircases announce whether they lead up or down. Enter uses normal safe auto-walking; closed doors are
approached without automatically opening them. Stairs are not automatically used.

Control+Shift+F4 lists known visible traps and tiles with dangerous fields,
nearest first. Enter approaches within one tile using the normal safe route
checks. It never disarms a trap or deliberately steps onto the selected hazard.
This list is a snapshot, not a guarantee that an area is safe; unseen hazards
remain hidden and hazards may change after a turn.

Both finders use the tracker controls: Control+[ / Control+] browse targets,
slash searches descriptions, Enter approaches, and Escape closes. Brackets
review ground items (these focused lists have no item entries). Browsing costs
no turns. Control+Shift+F11 reads the finder title and instructions. These
shortcuts and all new dialog announcements are active only in screen reader
mode. Turning the mode off closes an open finder.

### Exit-finder update validation (2026-10-08)

The Windows tiles build passed with warnings treated as errors. The existing
screen reader regression executable passed 53 assertions in 10 cases. A live
NVDA test using copied saves delivered outdoor-boundary and downward-staircase
selections, started a staircase auto-walk, reported an interrupted walk, and
reported an empty hazard list. Controller calls returned success. Actual arrival
at stairs/outdoors, populated hazard lists, and the new confirmation selection
and activation speech still need listening verification. Visual screenshot
capture was black in this test session; no visual validation is claimed.

### Stopping accessible auto-walking

In screen reader mode, Escape or the normal Pause command cancels a tracker or
finder auto-walk immediately, without a confirmation. Safe-mode movement blocks
also cancel these routes and return to normal gameplay input. The game speaks
that walking stopped and gives the current bindings for ignoring the spotted
enemy or switching safe mode off. Safe mode stays enabled until you choose to
change it. The warning is a movement restriction, not a dialog to dismiss:
other game commands remain available after the route is canceled. Arrival and
stop reports are delivered after frame speech so position text cannot cut them
off in the same frame. This behavior applies only in screen reader mode.

When a move reveals a new enemy, screen reader auto-walking returns to the
previous tile and stops automatically. It does not open the graphical peek
screen. Manual movement uses a spoken Yes/No confirmation: Yes keeps the new
position and ignores the newly spotted enemies; No or Escape returns to the
previous tile. No is selected initially. Normal visual mode keeps its peek UI.
