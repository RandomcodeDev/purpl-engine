# False King Game Design Version 1 2023-07-17

## Revisions

| Version | Author(s)    | Date       | Comments                    |
|---------|--------------|------------|-----------------------------|
| 1       | MobSlicer152 | 2023-07-17 | Initial version             |
| 2       | MobSlicer152 | 2023-11-25 | New version after 3D switch |

## Introduction

This document serves as a general concept draft that the game will follow. It
should evolve as the vision of the game evolves.

### Note on the use of the word element

Depending on context, this either refers to the components of the game's
design/gameplay/etc or the in-game elements likely to be fire, water, air, and
earth.

## Table of Contents

- [False King Game Design Version 2 2023-11-25](#false-king-game-design-version-1-2023-11-25)
  - [Revisions](#revisions)
  - [Introduction](#introduction)
    - [Note on the use of the word element](#note-on-the-use-of-the-word-element)
  - [Table of Contents](#table-of-contents)
  - [Concept](#concept)
  - [Story](#story)
  - [Game structure](#game-structure)
  - [Availability](#availability)
  - [Gameplay](#gameplay)
  - [Objectives](#objectives)
  - [Level design](#level-design)
  - [Graphics](#graphics)
  - [Audio](#audio)
  - [Player input](#player-input)
  - [User interface](#user-interface)
  - [Data Storage](#data-storage)
  - [Development](#development)
  - [Target platforms](#target-platforms)
  - [Rough timeline](#rough-timeline)

## Concept

The False King game will include elements of various genres such as roguelite,
roleplaying, puzzles, and also feature a 3D world made to look 2D.

## Story

The general idea of the story is that the player is on a quest to become the
king of the land, requiring them to defeat the rulers of different regions
which are themed after different elements likely to be fire, water, air, and earth.
The morality of the characters will not be a large part of the story, but may
be hinted at for flavour (i.e. who's the good guy isn't relevant to the story).

## Game structure

The regions will be playable in any order, with the exception of the final
region where the player must defeat the previous king. They will be replayable,
possibly featuring areas only accessible after the player has visited other
regions. Players will unlock each element when they enter a region the first
time, but only be allowed to leave after defeating the boss for that region
that first time. There will likely also be various NPCs the player can interact
with, allowing for an NPC area where the player starts.

## Availability

The game will (should approval from the necessary parties be obtained) be
available on Steam, as well as all major console platforms at the time of
release. Mobile devices (phones/tablets) might be supported at some point. The
target price is around 20 Canadian dollars, though a free version for PC may
exist, as player enjoyment is a higher priority than money (within reason). The
game will be available in English, with French, Russian, and simplified Chinese
translations being possibilities.

## Gameplay

The gameplay will involve real-time combat against various themed enemies.
Ideally the enemies will be unique depending on the region rather than having
shared archetypes between them, in the hopes of avoiding repetetiveness. The
player will be able to select between the different elements and use three
styles of attack, basic, special, and ranged. Combo moves between different
elements will be investigated. The current ECS-based design lends itself well
to this. Various movement features will also exist, such as jumping and
dashing. Bosses will have various moves that depend on what phase of the fight
they're in.

## Objectives

The ultimate goal is for the player to assume the title of False King by
defeating the final boss in the hidden final region. The objective in each
region will be to learn how to effectively use the power gained and then defeat
its boss. Once the player has become the False King, they will unlock some sort
of new game plus mode which introduces altered mechanics.

## Level design

The level design will be similar to that of Hyper Light Drifter. There will be
open areas with the occasional structure in them, indoor somewhat cramped
dungeon areas with traps, and areas with puzzles, and some large
structures/places that combine the various elements. There will also
potentially be areas that have enemies, which close when the player enters them
until the enemies are defeated. Boss rooms will be similar, additionally
featuring traps.

## Graphics

The game will use low-detail 3D models and textures, and the rendered frames will
be downscaled to look pixelated, making the game seem more 2D.

## Audio

The game will feature different music for each region, as well as variants for
different contexts such as outdoors, puzzles, dungeons, and combat. There will
be a few versions of the sounds for different actions, and most actions such as
walking, attacking, jumping, dashing, etc

## Player input

The game currently supports keyboard and mouse or controller. Keybindings are
currently implemented in a way that will be trivial to adapt to being
configurable by the user. Touch controls may be implemented.

## User interface

There will be a HUD visible at all times (except maybe some kind of screenshot
mode) that shows equipped items and powers, player health, and other relevant
information. There will be separate menus for the player's inventory, settings,
and other things.

## Data Storage

- Images are stored in a simple format in [`util/texture/texture.h`](util/texture/texture.h)
- TOML will be used for configuration files and metadata, such as font
  definitions
- Levels will likely consist of geometry and entities, no format has been designed yet
- Non-changing game assets such as models, textures, shaders, levels, audio,
  etc will be stored in some form of archive to allow more convenient and
  efficient storage
- Save data will be somehow stored, likely being created from a processed
  snapshot of the ECS world.
- The executable won't be particularly large
- Static linking is used to reduce the number of files necessary to work with,
  making distribution somewhat simpler and possibly allow for link time
  optimization
- Due to the fact that the game uses low-detail assets, it will very
  likely be orders of magnitude smaller than a typical AAA game even with no storage
  optimization

## Development

- The game is primarily developed using Visual Studio, alongside similar tools
  on platforms like macOS and Linux (Xcode and Neovim), using [xmake](https://github.com/xmake-io/xmake)
  as a build system (despite being developed in China, I have found absolutely
  no reason not to trust it, and since it's open source I could patch out any
  spyware if it were there)
- Various open source libraries with non-strict licenses are used
- Main game code is stored on GitHub, but assets and precompiled dependencies
  are stored on a personal Gitea server to get around file size limits. Any
  closed platform (console) related material (binaries, abstraction code) is to
  be kept in a private repository on GitHub.
- Due to very simple formats, assets can largely be developed with any image
  editor and text editor, as well as some form of audio editor, and 3D modelling
  software
- Instead of having scripting, everything will simply be implemented in C.
  Some form of extension system may be introduced.

## Target platforms

xmake makes it quite straightforward to build for other platforms, including the
ability to easily separate closed build system logic for consoles.

- Windows 10/11 are supported using the Microsoft Game Development Kit
- Linux and other Unix-like systems are/can be supported. Alternate binaries supporting
  systems not using the most common `libc` for a given platform may eventually
  be provided (although all core and external code thus far is open source and
  could be compiled by people who want a build that works better on their system)
- Xbox Series X|S support is planned, it can be supported with the same code as
  Windows, since the GDKX is only a superset of the GDK and Xbox is just a subset
  of Windows
- Nintendo Switch support is planned, and its successor should the rumoured
  Switch 2 be available for development
- PlayStation 5 support is planned

## Rough timeline

| Goal                     | Completion estimate |
|--------------------------|---------------------|
| Engine development       | Early 2024          |
| Core game mechanics      | Mid 2024            |
| Content development      | Late 2024           |
| Polishing                | Early 2025          |
| Testing                  | Mid 2025            |
| Release                  | Late 2025           |
| Additional localizations | When possible       |
| Additional platforms     | When possible       |
