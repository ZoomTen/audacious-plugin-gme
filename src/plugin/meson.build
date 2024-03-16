gme_sources = [
# Basic stuff
  'Blip_Buffer.cc',
  'Classic_Emu.cc',
  'Data_Reader.cc',
  'Dual_Resampler.cc',
  'Effects_Buffer.cc',
  'Fir_Resampler.cc',
  'gme.cc',
  'Gme_File.cc',
  'M3u_Playlist.cc',
  'Multi_Buffer.cc',
  'Music_Emu.cc',

# AY
  'Ay_Apu.cc',
  'Ay_Cpu.cc',
  'Ay_Emu.cc',

# YM2612, TODO: pick your backend!
  'Ym2612_Nuked.cc',
  #'Ym2612_MAME.cc',
  #'Ym2612_GENS.cc',

# SMS / GG
  'Sms_Apu.cc',

# Game Boy
  'Gb_Apu.cc',
  'Gb_Cpu.cc',
  'Gb_Oscs.cc',
  'Gbs_Emu.cc',

# GYM support
  'Gym_Emu.cc',

# HES
  'Hes_Apu.cc',
  'Hes_Cpu.cc',
  'Hes_Emu.cc',

# KSS = AY + TI-something
  'Kss_Cpu.cc',
  'Kss_Emu.cc',
  'Kss_Scc_Apu.cc',

# NES
  'Nes_Apu.cc',
  'Nes_Cpu.cc',
  'Nes_Fme7_Apu.cc',
  'Nes_Namco_Apu.cc',
  'Nes_Oscs.cc',
  'Nes_Vrc6_Apu.cc',
  'Nes_Fds_Apu.cc',
  'Nes_Vrc7_Apu.cc',
  'ext/emu2413.c',
  'ext/panning.c',
  'Nsf_Emu.cc',

# NSFE support
  'Nsfe_Emu.cc',

# SAP
  'Sap_Apu.cc',
  'Sap_Cpu.cc',
  'Sap_Emu.cc',

# SNES
  'Snes_Spc.cc',
  'Spc_Cpu.cc',
  'Spc_Dsp.cc',
  'Spc_Emu.cc',
  'Spc_Filter.cc',

# VGM support
  'Vgm_Emu.cc',
  'Vgm_Emu_Impl.cc',
  'Ym2413_Emu.cc'
]


plugin_sources = [
  'audacious/Vfs_File.cc',
  'audacious/Audacious_Driver.cc',
  'audacious/configure.cc',
  'audacious/plugin.cc'
]

tag_parser_sources = [
  'tags/libgmtag/src/libgmtag.cpp',
  'tags/libgmtag/src/private/getters.cpp',
  'tags/libgmtag/src/private/setters.cpp',
  'tags/libgmtag/src/private/utils.cpp'
]

tag_parser_includes = include_directories(
  'tags/libgmtag/include'
)

cpp_args = cxx.get_supported_arguments([
  '-Wno-shift-negative-value'
])


shared_module('gme',
  gme_sources,
  plugin_sources,
  tag_parser_sources,
  dependencies: [audacious_dep, zlib_dep],
  cpp_args: cpp_args,
  name_prefix: '',
  install: true,
  install_dir: input_plugin_dir,
  include_directories: tag_parser_includes
)
