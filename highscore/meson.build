project('mednafen-highscore', ['cpp', 'c'],
  version : '0.1',
)

mednafen_version='1.31.0-UNSTABLE'
mednafen_version_numeric=0x00103100

add_project_arguments([
  '-Wno-unused-but-set-variable',
  '-Wno-unused-variable',
  '-std=gnu99',
  '-fvisibility=hidden',
], language: 'c')

add_project_arguments([
  '-Wno-format-overflow',
  '-Wno-implicit-fallthrough',
  '-Wno-missing-field-initializers',
  '-Wno-stringop-truncation',
  '-Wno-unused-but-set-variable',
  '-Wno-unused-parameter',
  '-Wno-unused-value',
  '-Wno-unused-variable',
  '-fvisibility=hidden',
], language: 'cpp')

cc = meson.get_compiler('c')
fs = import('fs')

cores_dir = get_option('prefix') / get_option('libdir') / 'highscore' / 'cores'

mednafen_deps = [
  dependency('highscore-1'),
  dependency('gio-2.0'),
  dependency('iconv'),
  dependency('libzstd'),
  dependency('zlib'),
  cc.find_library('m', required : false),
]

minilzo_source = [
  '../src/minilzo/minilzo.c',
]

minilzo = static_library('minilzo',
  minilzo_source,
  include_directories: include_directories('include')
)

trio_source = [
  '../src/trio/trio.c',
  '../src/trio/trionan.c',
  '../src/trio/triostr.c',
]

trio = static_library('trio',
  trio_source,
  include_directories: include_directories('include')
)

mednafen_source = [
  '../src/cdplay/cdplay.cpp',
  '../src/cdrom/CDAFReader.cpp',
  '../src/cdrom/CDAFReader_MPC.cpp',
  '../src/cdrom/CDAFReader_PCM.cpp',
  '../src/cdrom/CDAFReader_Vorbis.cpp',
  '../src/cdrom/CDAccess.cpp',
  '../src/cdrom/CDAccess_CCD.cpp',
  '../src/cdrom/CDAccess_Image.cpp',
  '../src/cdrom/CDInterface.cpp',
  '../src/cdrom/CDInterface_MT.cpp',
  '../src/cdrom/CDInterface_ST.cpp',
  '../src/cdrom/CDUtility.cpp',
  '../src/cdrom/crc32.cpp',
  '../src/cdrom/galois.cpp',
  '../src/cdrom/l-ec.cpp',
  '../src/cdrom/lec.cpp',
  '../src/cdrom/recover-raw.cpp',
  '../src/compress/ArchiveReader.cpp',
  '../src/compress/DecompressFilter.cpp',
  '../src/compress/GZFileStream.cpp',
  '../src/compress/ZIPReader.cpp',
  '../src/compress/ZLInflateFilter.cpp',
  '../src/compress/ZstdDecompressFilter.cpp',
  '../src/cputest/cputest.c',
  '../src/demo/demo.cpp',
  '../src/hash/crc.cpp',
  '../src/hash/md5.cpp',
  '../src/hw_cpu/v810/v810_cpu.cpp',
  '../src/hw_cpu/v810/v810_fp_ops.cpp',
  '../src/hw_cpu/z80-fuse/z80.cpp',
  '../src/hw_cpu/z80-fuse/z80_ops.cpp',
  '../src/hw_misc/arcade_card/arcade_card.cpp',
  '../src/mpcdec/crc32.c',
  '../src/mpcdec/huffman.c',
  '../src/mpcdec/mpc_bits_reader.c',
  '../src/mpcdec/mpc_decoder.c',
  '../src/mpcdec/mpc_demux.c',
  '../src/mpcdec/mpc_reader.c',
  '../src/mpcdec/requant.c',
  '../src/mpcdec/streaminfo.c',
  '../src/mpcdec/synth_filter.c',
  '../src/mthreading/MThreading_POSIX.cpp',
  '../src/quicklz/quicklz.c',
  '../src/resampler/resample.c',
  '../src/sound/Blip_Buffer.cpp',
  '../src/sound/DSPUtility.cpp',
  '../src/sound/Fir_Resampler.cpp',
  '../src/sound/OwlResampler.cpp',
  '../src/sound/Stereo_Buffer.cpp',
  '../src/sound/WAVRecord.cpp',
  '../src/sound/okiadpcm.cpp',
  '../src/string/escape.cpp',
  '../src/string/string.cpp',
  '../src/time/Time_POSIX.cpp',
  '../src/tremor/bitwise.c',
  '../src/tremor/block.c',
  '../src/tremor/codebook.c',
  '../src/tremor/floor0.c',
  '../src/tremor/floor1.c',
  '../src/tremor/framing.c',
  '../src/tremor/info.c',
  '../src/tremor/mapping0.c',
  '../src/tremor/mdct.c',
  '../src/tremor/registry.c',
  '../src/tremor/res012.c',
  '../src/tremor/sharedbook.c',
  '../src/tremor/synthesis.c',
  '../src/tremor/vorbisfile.c',
  '../src/tremor/window.c',
  '../src/video/Deinterlacer.cpp',
  '../src/video/Deinterlacer_Blend.cpp',
  '../src/video/Deinterlacer_Simple.cpp',
  '../src/video/convert.cpp',
  '../src/video/primitives.cpp',
  '../src/video/resize.cpp',
  '../src/video/surface.cpp',
  '../src/video/tblur.cpp',
  '../src/FileStream.cpp',
  '../src/IPSPatcher.cpp',
  '../src/MemoryStream.cpp',
  '../src/NativeVFS.cpp',
  '../src/Stream.cpp',
  '../src/VirtualFS.cpp',
  '../src/endian.cpp',
  '../src/error.cpp',
  '../src/file.cpp',
  '../src/general.cpp',
  '../src/git.cpp',
  '../src/mednafen.cpp',
  '../src/mempatcher.cpp',
  '../src/memory.cpp',
  '../src/player.cpp',
  '../src/settings.cpp',
  '../src/state.cpp',
  '../src/state_rewind.cpp',

  '../src/ngp/TLCS-900h/TLCS900h_disassemble.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_disassemble_dst.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_disassemble_extra.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_disassemble_reg.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_disassemble_src.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_interpret.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_interpret_dst.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_interpret_reg.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_interpret_single.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_interpret_src.cpp',
  '../src/ngp/TLCS-900h/TLCS900h_registers.cpp',
  '../src/ngp/bios.cpp',
  '../src/ngp/biosHLE.cpp',
  '../src/ngp/dma.cpp',
  '../src/ngp/flash.cpp',
  '../src/ngp/gfx.cpp',
  '../src/ngp/gfx_scanline_colour.cpp',
  '../src/ngp/gfx_scanline_mono.cpp',
  '../src/ngp/interrupt.cpp',
  '../src/ngp/mem.cpp',
  '../src/ngp/neopop.cpp',
  '../src/ngp/rom.cpp',
  '../src/ngp/rtc.cpp',
  '../src/ngp/sound.cpp',
  '../src/ngp/T6W28_Apu.cpp',
  '../src/ngp/Z80_interface.cpp',

  '../src/pce_fast/hes.cpp',
  '../src/pce_fast/huc6280.cpp',
  '../src/pce_fast/huc.cpp',
  '../src/pce_fast/input.cpp',
  '../src/pce_fast/pcecd.cpp',
  '../src/pce_fast/pcecd_drive.cpp',
  '../src/pce_fast/pce.cpp',
  '../src/pce_fast/psg.cpp',
  '../src/pce_fast/vdc.cpp',

  '../src/vb/input.cpp',
  '../src/vb/timer.cpp',
  '../src/vb/vb.cpp',
  '../src/vb/vip.cpp',
  '../src/vb/vsu.cpp',

  '../src/wswan/comm.cpp',
  '../src/wswan/eeprom.cpp',
  '../src/wswan/gfx.cpp',
  '../src/wswan/interrupt.cpp',
  '../src/wswan/main.cpp',
  '../src/wswan/memory.cpp',
  '../src/wswan/rtc.cpp',
  '../src/wswan/sound.cpp',
  '../src/wswan/tcache.cpp',
  '../src/wswan/v30mz.cpp',
  '../src/wswan/dis/dis_decode.cpp',
  '../src/wswan/dis/dis_groups.cpp',
  '../src/wswan/dis/resolve.cpp',
  '../src/wswan/dis/syntax.cpp',

  'mednafen-highscore.cpp',

  'shims/video/text.cpp',
  'shims/driver.cpp',
  'shims/movie.cpp',
  'shims/netplay.cpp',
  'shims/qtrecord.cpp',
  'shims/tests.cpp',
]

mednafen_c_cpp_args = [
  '-fno-aggressive-loop-optimizations',
  '-fno-fast-math',
  '-fno-strict-aliasing',
  '-fomit-frame-pointer',
  '-fsigned-char',
  '-fwrapv',
  '-DLSB_FIRST=1',
  '-DMDFN_DISABLE_PICPIE_ERRWARN=1',
  '-DICONV_CONST=',
  '-DPACKAGE="mednafen"',
  '-DPSS_STYLE=1',
  '-DHAVE_INLINEASM_AVX=1',
  '-DHAVE_MKDIR=1',
  '-DMEDNAFEN_VERSION="@0@"'.format(mednafen_version),
  '-DMEDNAFEN_VERSION_NUMERIC=@0@'.format(mednafen_version_numeric),
  '-DWANT_NGP_EMU=1',
  '-DWANT_PCE_FAST_EMU=1',
  '-DWANT_VB_EMU=1',
  '-DWANT_WSWAN_EMU=1',
]

arch = build_machine.cpu_family()

if arch == 'x86'
  mednafen_c_cpp_args += [ '-fno-pic', '-fno-pie', '-DARCH_X86=1', '-DARCH_X86_32=1' ]
  mednafen_source += '../src/cputest/x86_cpu.c'
elif arch == 'x86_64'
  mednafen_c_cpp_args += [ '-DARCH_X86=1', '-DARCH_X86_64=1' ]
  mednafen_source += '../src/cputest/x86_cpu.c'
elif arch == 'arm'
elif arch == 'aarch64'
else
  error('Unsupported CPU family')
endif


mednafen_include_dirs = [
  include_directories('include'),
  include_directories('../include'),
]

shared_library('mednafen-highscore',
  mednafen_source,
  dependencies : mednafen_deps,
  include_directories : mednafen_include_dirs,
  link_with : [ trio, minilzo ],
  c_args : mednafen_c_cpp_args,
  cpp_args : mednafen_c_cpp_args,
  name_prefix: '',
  install_dir : cores_dir,
  install : true,
)

install_data('mednafen.highscore',
  install_dir : cores_dir,
)
