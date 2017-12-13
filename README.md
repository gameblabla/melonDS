melonDS for the GCW0

This is a port of melonDS, a Nintendo DS emulator, for the GCW0.

Just like DesMume, it is very slow, plus unlike DesMume it requires BIOS & Firmware files in /usr/local/home/.config/melonds :

bios7.bin, bios9.bin and firmware.bin.

It required GTK+3.0/SDL 2.0 but this was ported to SDL 1.2, without the GTK bloatness.

This port is a proof-of-concept and just for the lulz.

Don't use it for gaming, you'll seriously cry. (even though compatibility is decent)

Hopefully melonDS will work on the hardware renderer and HOPEFULLY, its OpenGL ES 2.0.

Press Start + Select to exit the emulator.

=========


# melonDS

DS emulator, sorta

The goal is to do things right and fast, akin to blargSNES (but hopefully better). But also to, you know, have a fun challenge :)

The source code is provided under the GPLv3 license.

## How to use

melonDS requires BIOS/firmware copies from a DS. Files required:
 * bios7.bin, 16KB: ARM7 BIOS
 * bios9.bin, 4KB: ARM9 BIOS
 * firmware.bin, 128/256/512KB: firmware
 
Firmware boot requires a firmware dump from an original DS or DS Lite.
DS firmwares dumped from a DSi or 3DS aren't bootable and only contain configuration data, thus they are only suitable when booting games directly.

### Possible firmware sizes

 * 128KB: DSi/3DS DS-mode firmware (reduced size due to lacking bootcode)
 * 256KB: regular DS firmware
 * 512KB: iQue DS firmware

DS BIOS dumps from a 3DS can be used with no compatibility issues. DSi BIOS dumps should be usable too, provided they were dumped properly.

As for the rest, the interface should be pretty straightforward. If you have a question, don't hesitate to ask, though!

## How to build

### Linux:

```sh
mkdir -p build
cd build
cmake ..
make
```

### Windows:

 * use CodeBlocks
 * or receive golden cookies if you get Cmake to work

Build system is not set in stone.

## TODO LIST

 * UI that doesn't suck
 * better sound emulation
 * emulating some fancy addons
 * other non-core shit (debugger, graphics viewers, cheat crapo, etc)
 
### TODO LIST FOR LATER

 * hardware renderer for 3D (w/ upscaling and fancy shit)
 * better wifi
 * maybe emulate flashcarts or other fancy hardware
 * big-endian compatibility (Wii, etc)
 * LCD refresh time (used by some games for blending effects)
 * any feature you can eventually ask for that isn't outright stupid
 
## Credits

 * Martin for GBAtek, a good piece of documentation
 * Cydrak for the extra 3D GPU research
 * All of you comrades who have been testing melonDS, reporting issues, suggesting shit, etc
