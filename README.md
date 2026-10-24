# Mythic Plus

A Wrath 3.3.5 Mythic+ module for AzerothCore. It carries its own dungeon builder. The installed Dungeon Master module is not required and is not modified.

## Install

Yu'lon installs this folder as `modules/mod-mythicplus`. The folder needs `src/`, an empty `include.sh`, `conf/mod_mythicplus.conf.dist`, and the SQL under `data/sql`.

After install, apply:

- `data/sql/db-world/base/mp_setup.sql` on the world database
- `data/sql/db-characters/base/mp_characters.sql` on the characters database, once that script exists

The NPC is creature entry `500100`, script name `npc_mythic_plus`. It is not entry `500000`.

## Cutover

Uninstall Dungeon Master before players use this mode. Copy `client/MythicPlus` into each member's `Interface/AddOns` folder. Yu'lon does not copy the addon into the game client.

## Commands

Game masters use `.mp`. This module does not register `.dm`.
