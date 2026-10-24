# Mythic Plus

A Wrath 3.3.5 Mythic+ module for AzerothCore. It carries its own dungeon builder. The installed Dungeon Master module is not required and is not modified.

## Install

In Yu'lon, on the Modules tab, choose **Install a module from a link** and paste:

https://github.com/DudelyStudios/mod-mythicplus

Yu'lon clones that repository into `modules/mod-mythicplus`. The folder ships `src/`, an empty `include.sh`, `conf/mod_mythicplus.conf.dist`, and the SQL under `data/sql`. Yu'lon applies that SQL on install. The module does nothing until the server is rebuilt.

Release `v1.0.0` is the first installable version. Later releases are how Yu'lon sees an update.

After install, apply:

- `data/sql/db-world/base/mp_setup.sql` on the world database
- `data/sql/db-characters/base/mp_characters.sql` on the characters database

The NPC is creature entry `500100`, script name `npc_mythic_plus`. It is not entry `500000`.

## Cutover

Uninstall Dungeon Master before players use this mode. Copy `client/MythicPlus` into each member's `Interface/AddOns` folder. Yu'lon does not copy the addon into the game client.

## Commands

Game masters use `.mp`. This module does not register `.dm`.

- `.mp status` shows the selected player's held key, season, and score.
- `.mp key <level>` sets that player's held key. A level below the floor is refused.
- `.mp reload` reads the dungeon and theme rows again.
- `.mp season` starts a new season. Scores clear. Held keys stay.
- `.mp end` sends the current run home and leaves the key where it was.

## What a run does

Five level 80 players talk to Mythic Keystone, pick a theme and a dungeon, and the player who starts the run spends that player's key. The module unbinds the map on normal and heroic, then teleports the group in with the game-master teleport flag so attunement and hourly instance limits do not stop the entry. The instance is emptied and refilled. A clear on time raises the key by 1, 2, or 3. A miss drops it by 1, never below +2. A wipe revives the group at the entrance and adds time. It does not end the run.

Copy `client/MythicPlus` to `Interface/AddOns/MythicPlus`. The frame shows the run key, the modifiers, the timer after it expires, the viewer's held key, and the viewer's vault key.
