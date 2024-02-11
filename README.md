# Audacious GME — Updated

This is an updated version of the [Audacious “Game Console Music Decoder” plugin](https://github.com/audacious-media-player/audacious-plugins/tree/c6257f9/src/console).

The Game_Music_Emu backend has been updated to version 0.6.x from the [libgme](https://github.com/libgme/game-music-emu) fork. Changes from that upstream should be kept as minimal as possible, and the Audacious plugin parts should instead adapt to it. (Or at least, that's the plan here)

**This is a work-in-progress!**

## Changes so far

- Update from Blargg's GME 0.5.2 to libgme indev 0.6.4

## Licenses

- libgme: LGPL-2.1
- Nuked-OPN2: LGPL-2.1
- emu2413: MIT
- Audacious-Plugins: BSD-2-Clause

## What I've tested it with

Audacious 4.3.1 on Fedora Linux 39 x86_64

- [ ] AY
- [x] GBS
- [ ] GYM
- [ ] HES
- [ ] KSS
- [ ] NSF
  - [ ] NSFe
- [ ] SAP
- [ ] SPC
- [ ] VGM
  - [ ] YM2413 (emu2413)
  - [x] YM2612
    - [x] Nuked
    - [ ] MAME
    - [ ] Gens
  - [x] SN76489
  - [ ] T6W28
  - [ ] ???

## TODO

**Support:**

- Compressed files (e.g. vgz, rsn)
- Sidecar tag file support (even if it's m3u, the tagging landscape is desparate)
  - Make it reflect properly both on the metadata page and in the playlists

**Configuration:**

- Pick between different emulator backends (Nuked/Gens/MAME)
- A channel mixer, tempo adjuster, etc. etc., if possible