project('mednafen-highscore', ['cpp', 'c'],
  version : '1.32.1',
  meson_version: '>= 1.3.0',
  default_options: ['c_std=gnu99']
)

mednafen_version_numeric=0x00103201

version = meson.project_version()
commit = run_command('git', 'describe', '--always', '--dirty', check: false).stdout().strip()

add_project_arguments([
  '-Wno-stringop-truncation',
  '-Wno-unused-but-set-variable',
  '-Wno-unused-variable',
  '-fvisibility=hidden',
], language: 'c')

add_project_arguments([
  '-Wno-address',
  '-Wno-array-bounds',
  '-Wno-format-overflow',
  '-Wno-implicit-fallthrough',
  '-Wno-missing-field-initializers',
  '-Wno-stringop-truncation',
  '-Wno-unused-but-set-variable',
  '-Wno-unused-parameter',
  '-Wno-unused-result',
  '-Wno-unused-value',
  '-Wno-unused-variable',
  '-fvisibility=hidden',
], language: 'cpp')

cc = meson.get_compiler('c')
fs = import('fs')

core_dir = get_option('core_install_dir')
if core_dir == ''
  libdir = get_option('prefix') / get_option('libdir')
  core_dir = libdir / 'highscore' / 'cores' / 'mednafen'
endif

mednafen_deps = [
  dependency('highscore-1'),
  dependency('gio-2.0'),
  dependency('iconv'),
  dependency('libchdr'),
  dependency('libzstd'),
  dependency('vorbisfile'),
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
  '../src/cdrom/CDAccess_CHD.cpp',
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
  '../src/cheat_formats/psx.cpp',
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
  '../src/hash/sha1.cpp',
  '../src/hash/sha256.cpp',
  '../src/hw_cpu/m68k/gen.cpp',
  '../src/hw_cpu/m68k/m68k.cpp',
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
  '../src/sound/SwiftResampler.cpp',
  '../src/sound/WAVRecord.cpp',
  '../src/sound/okiadpcm.cpp',
  '../src/string/escape.cpp',
  '../src/string/string.cpp',
  '../src/time/Time_POSIX.cpp',
  '../src/video/Deinterlacer.cpp',
  '../src/video/Deinterlacer_Blend.cpp',
  '../src/video/Deinterlacer_Simple.cpp',
  '../src/video/convert.cpp',
  '../src/video/primitives.cpp',
  '../src/video/resize.cpp',
  '../src/video/surface.cpp',
  '../src/video/tblur.cpp',
  '../src/ExtMemStream.cpp',
  '../src/FileStream.cpp',
  '../src/IPSPatcher.cpp',
  '../src/MemoryStream.cpp',
  '../src/NativeVFS.cpp',
  '../src/PSFLoader.cpp',
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
  '../src/tests.cpp',
  '../src/testsexp.cpp',

  '../src/lynx/c65c02.cpp',
  '../src/lynx/cart.cpp',
  '../src/lynx/memmap.cpp',
  '../src/lynx/mikie.cpp',
  '../src/lynx/ram.cpp',
  '../src/lynx/rom.cpp',
  '../src/lynx/susie.cpp',
  '../src/lynx/system.cpp',

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

  '../src/psx/input/dualanalog.cpp',
  '../src/psx/input/dualshock.cpp',
  '../src/psx/input/gamepad.cpp',
  '../src/psx/input/guncon.cpp',
  '../src/psx/input/justifier.cpp',
  '../src/psx/input/memcard.cpp',
  '../src/psx/input/mouse.cpp',
  '../src/psx/input/multitap.cpp',
  '../src/psx/input/negcon.cpp',
  '../src/psx/cdc.cpp',
  '../src/psx/cpu.cpp',
  '../src/psx/dis.cpp',
  '../src/psx/dma.cpp',
  '../src/psx/frontio.cpp',
  '../src/psx/gpu.cpp',
  '../src/psx/gpu_line.cpp',
  '../src/psx/gpu_polygon.cpp',
  '../src/psx/gpu_sprite.cpp',
  '../src/psx/gte.cpp',
  '../src/psx/irq.cpp',
  '../src/psx/mdec.cpp',
  '../src/psx/psx.cpp',
  '../src/psx/sio.cpp',
  '../src/psx/spu.cpp',
  '../src/psx/timer.cpp',

  '../src/ss/cart/ar4mp.cpp',
  '../src/ss/cart/backup.cpp',
  '../src/ss/cart/bootrom.cpp',
  '../src/ss/cart/cs1ram.cpp',
  '../src/ss/cart/extram.cpp',
  '../src/ss/cart/rom.cpp',
  '../src/ss/cart/stv.cpp',
  '../src/ss/input/3dpad.cpp',
  '../src/ss/input/gamepad.cpp',
  '../src/ss/input/gun.cpp',
  '../src/ss/input/jpkeyboard.cpp',
  '../src/ss/input/keyboard.cpp',
  '../src/ss/input/mission.cpp',
  '../src/ss/input/mouse.cpp',
  '../src/ss/input/multitap.cpp',
  '../src/ss/input/wheel.cpp',
  '../src/ss/ak93c45.cpp',
  '../src/ss/cart.cpp',
  '../src/ss/cdb.cpp',
  '../src/ss/db.cpp',
  '../src/ss/scu_dsp_gen.cpp',
  '../src/ss/scu_dsp_jmp.cpp',
  '../src/ss/scu_dsp_misc.cpp',
  '../src/ss/scu_dsp_mvi.cpp',
  '../src/ss/smpc.cpp',
  '../src/ss/sound.cpp',
  '../src/ss/ss.cpp',
  '../src/ss/stvio.cpp',
  '../src/ss/vdp1.cpp',
  '../src/ss/vdp1_line.cpp',
  '../src/ss/vdp1_poly.cpp',
  '../src/ss/vdp1_sprite.cpp',
  '../src/ss/vdp2.cpp',
  '../src/ss/vdp2_render.cpp',

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
]

mednafen_c_cpp_args = [
  '-fno-aggressive-loop-optimizations',
  '-fno-fast-math',
  '-fno-strict-aliasing',
  '-fomit-frame-pointer',
  '-fsigned-char',
  '-fwrapv',
  '-DMDFN_DISABLE_PICPIE_ERRWARN=1',
  '-DICONV_CONST=',
  '-DPACKAGE="mednafen"',
  '-DHAVE_CHD=1',
  '-DHAVE_EXTERNAL_LIBZSTD=1',
  '-DHAVE_INLINEASM_AVX=1',
  '-DHAVE_POSIX_PRINTF=1',
  '-DHAVE_POSIX_SOCKETS=1',
  '-DHAVE_PTHREAD_RWLOCK=1',
  '-DHAVE_VISIBILITY=1',
  '-DHAVE_WORKING_O_NOATIME=1',
  '-DHAVE_WORKING_O_NOFOLLOW=1',
  '-DMDFN_PSS_STYLE=1',
  '-DPTHREAD_AFFINITY_NP=cpu_set_t',
  '-DMEDNAFEN_VERSION="@0@"'.format(version),
  '-DMEDNAFEN_VERSION_NUMERIC=@0@'.format(mednafen_version_numeric),
  '-DWANT_68K_EMU=1',
  '-DWANT_LYNX_EMU=1',
  '-DWANT_NGP_EMU=1',
  '-DWANT_PCE_FAST_EMU=1',
  '-DWANT_PSX_EMU=1',
  '-DWANT_SS_EMU=1',
  '-DWANT_VB_EMU=1',
  '-DWANT_WSWAN_EMU=1',
  '-DSIZEOF_CHAR=@0@'.format(cc.sizeof('char')),
  '-DSIZEOF_SHORT=@0@'.format(cc.sizeof('short')),
  '-DSIZEOF_INT=@0@'.format(cc.sizeof('int')),
  '-DSIZEOF_LONG=@0@'.format(cc.sizeof('long')),
  '-DSIZEOF_LONG_LONG=@0@'.format(cc.sizeof('long long')),
  '-DSIZEOF_OFF_T=@0@'.format(cc.sizeof('off_t')),
  '-DSIZEOF_PTRDIFF_T=@0@'.format(cc.sizeof('ptrdiff_t')),
  '-DSIZEOF_SIZE_T=@0@'.format(cc.sizeof('size_t')),
  '-DSIZEOF_VOID_P=@0@'.format(cc.sizeof('void*')),
  '-D__HIGHSCORE__',
]

mednafen_function_checks = {
  'HAVE_ACCEPT': 'accept',
  'HAVE_ALLOCA': 'alloca',
  'HAVE_ARGZ_COUNT': 'argz_count',
  'HAVE_ARGZ_NEXT': 'argz_next',
  'HAVE_ARGZ_STRINGIFY': 'argz_stringify',
  'HAVE_ASPRINTF': 'asprintf',
  'HAVE_BIND': 'bind',
  'HAVE_BUILTIN_EXPECT': '__builtin_expect',
  'HAVE_CLOCK_GETTIME': 'clock_gettime',
  'HAVE_CLOSE': 'close',
  'HAVE_CONNECT': 'connect',
  'HAVE_DCGETTEXT': 'dcgettext',
  'HAVE_DECL_FEOF_UNLOCKED': 'feof_unlocked',
  'HAVE_DECL_FGETS_UNLOCKED': 'fgets_unlocked',
  'HAVE_DECL_GETC_UNLOCKED': 'getc_unlocked',
  'HAVE_DECL__SNPRINTF': '_snprintf',
  'HAVE_DECL__SNWPRINTF': '_snwprintf',
  'HAVE_DUP2': 'dup2',
  'HAVE_FNCTL': 'fnctl',
  'HAVE_FORK': 'fork',
  'HAVE_FREEADDRINFO': 'freeaddrinfo',
  'HAVE_FSTAT64': 'fstat64',
  'HAVE_FTRUNCATE64': 'ftruncate64',
  'HAVE_FWPRINTF': 'fwprintf',
  'HAVE_GAI_STRERROR': 'gai_strerror',
  'HAVE_GETADDRINFO': 'getaddrinfo',
  'HAVE_GETCWD': 'getcwd',
  'HAVE_GETEGID': 'getegid',
  'HAVE_GETENV': 'getenv',
  'HAVE_GETEUID': 'geteuid',
  'HAVE_GETGID': 'getgid',
  'HAVE_GETHOSTBYADDR': 'gethostbyaddr',
  'HAVE_GETHOSTBYNAME': 'gethostbyname',
  'HAVE_GETPWUID': 'getpwuid',
  'HAVE_GETSOCKOPT': 'getsockopt',
  'HAVE_GETTEXT': 'gettext',
  'HAVE_GETTIMEOFDAY': 'gettimeofday',
  'HAVE_GETUID': 'getuid',
  'HAVE_GMTIME_R': 'gmtime_r',
  'HAVE_ICONV': 'iconv',
  'HAVE_LISTEN': 'listen',
  'HAVE_LOCALTIME_R': 'localtime_r',
  'HAVE_LSEEK64': 'lseek64',
  'HAVE_MADVISE': 'madvise',
  'HAVE_MBRTOWC': 'mbrtowc',
  'HAVE_MEMCMP': 'memcmp',
  'HAVE_MEMCPY': 'memcpy',
  'HAVE_MEMMOVE': 'memmove',
  'HAVE_MEMSET': 'memset',
  'HAVE_MKDIR': 'mkdir',
  'HAVE_MMAP': 'mmap',
  'HAVE_MUNMAP': 'munmap',
  'HAVE_NANOSLEEP': 'nanosleep',
  'HAVE_NEARBYINT': 'nearbyint',
  'HAVE_NEARBYINTF': 'nearbyintf',
  'HAVE_NEWLOCALE': 'newlocale',
  'HAVE_PIPE': 'pipe',
  'HAVE_POLL': 'poll',
  'HAVE_PTHREAD_CONDATTR_SETCLOCK': 'pthread_condattr_setclock',
  'HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE_NP': 'pthread_cond_timedwait_relative_np',
  'HAVE_PTHREAD_CREATE': 'pthread_create',
  'HAVE_PTHREAD_GETAFFINITY_NP': 'pthread_getaffinity_np',
  'HAVE_PTHREAD_SETAFFINITY_NP': 'pthread_setaffinity_np',
  'HAVE_PTHREAD_SETNAME_NP': 'pthread_setname_np',
  'HAVE_PTHREAD_SET_NAME_NP': 'pthread_set_name_np',
  'HAVE_PUTENV': 'putenv',
  'HAVE_RECV': 'recv',
  'HAVE_RINT': 'rint',
  'HAVE_RINTF': 'rintf',
  'HAVE_ROUND': 'round',
  'HAVE_SELECT': 'select',
  'HAVE_SEM_CLOCKWAIT': 'sem_clockwait',
  'HAVE_SEM_CLOCKWAIT_NP': 'sem_clockwait_np',
  'HAVE_SEM_INIT': 'sem_init',
  'HAVE_SEM_TIMEDWAIT': 'sem_timedwait',
  'HAVE_SEM_TIMEDWAIT_MONOTONIC': 'sem_timedwait_monotonic',
  'HAVE_SEND': 'send',
  'HAVE_SETENV': 'setenv',
  'HAVE_SETLOCALE': 'setlocale',
  'HAVE_SETSOCKOPT': 'setsockopt',
  'HAVE_SIGACTION': 'sigaction',
  'HAVE_SIGLONGJMP': 'siglongjmp',
  'HAVE_SIGNAL': 'signal',
  'HAVE_SNPRINTF': 'snprintf',
  'HAVE_SOCKET': 'socket',
  'HAVE_STRCPY': 'strcpy',
  'HAVE_STRCASECMP': 'strcasecmp',
  'HAVE_STRDUP': 'strdup',
  'HAVE_STRERROR': 'strerror',
  'HAVE_STRERROR_R': 'strerror_r',
  'HAVE_STRNLEN': 'strnlen',
  'HAVE_STRTOUL': 'strtoul',
  'HAVE_TSEARCH': 'tsearch',
  'HAVE_USELOCALE': 'uselocale',
  'HAVE_USLEEP': 'usleep',
  'HAVE_WCHAR_T': 'wchar_t',
  'HAVE_WCRTOMB': 'wcrtomb',
  'HAVE_WCSLEN': 'wcslen',
  'HAVE_WCSNLEN': 'wcsnlen',
  'HAVE__LONGJMP': '_longjmp',
  'HAVE__MKDIR': '_mkdir',
  'HAVE___FSETLOCKING': '__fsetlocking',
  'HAVE___MINGW_GET_CRT_INFO': '__mingw_get_crt_info',
}

mednafen_header_checks = {
  'HAVE_ALLOCA_H': 'alloca.h',
  'HAVE_ALVITEC_H': 'alvitec.h',
  'HAVE_ARGZ_H': 'argz.h',
  'HAVE_FNCTL_H': 'fnctl.h',
  'HAVE_FENV_H': 'fenv.h',
  'HAVE_INTTYPES_H': 'inttypes.h',
  'HAVE_LIMITS_H': 'limits.h',
  'HAVE_MEMORY_H': 'memory.h',
  'HAVE_PTHREAD_H': 'pthread.h',
  'HAVE_SCHED_H': 'sched.h',
  'HAVE_STDDEF_H': 'stddef.h',
  'HAVE_STDINT_H': 'stdint.h',
  'HAVE_STDLIB_H': 'stdlib.h',
  'HAVE_STRINGS_H': 'strings.h',
  'HAVE_STRING_H': 'string.h',
  'HAVE_SYS_PARAM_H': 'sys/param.h',
  'HAVE_SYS_STAT_H': 'sys/stat.h',
  'HAVE_SYS_TYPES_H': 'sys/types.h',
  'HAVE_PTHREAD_NP_H': 'pthread_np.h',
  'HAVE_UNISTD_H': 'unistd.h',
}

mednafen_define_checks = {
  'HAVE_PTHREAD_MUTEX_RECIRSIVE': 'PTHREAD_MUTEX_RECURSIVE',
}

mednafen_type_checks = {
  'HAVE_INTMAX_T': 'intmax_t',
  'HAVE_LONG_LONG_INT': 'long long int',
  'HAVE_UINTMAX_T': 'uintmax_t',
  'HAVE_UNSIGNED_LONG_LONG_INT': 'unsigned long long int',
  'HAVE_WINT_T': 'wint_t',
}

foreach macro, func : mednafen_function_checks
  if cc.has_function(func)
    mednafen_c_cpp_args += '-D@0@=1'.format(macro)
  endif
endforeach

foreach macro, header : mednafen_header_checks
  if cc.has_header(header)
    mednafen_c_cpp_args += '-D@0@=1'.format(macro)
  endif
endforeach

foreach macro, define : mednafen_define_checks
  if cc.has_define(define)
    mednafen_c_cpp_args += '-D@0@=1'.format(macro)
  endif
endforeach

foreach macro, type : mednafen_type_checks
  if cc.has_type(type)
    mednafen_c_cpp_args += '-D@0@=1'.format(macro)
  endif
endforeach

arch = build_machine.cpu_family()

if arch == 'x86'
  mednafen_c_cpp_args += [ '-fno-pic', '-fno-pie', '-DARCH_X86=1', '-DARCH_X86_32=1', '-DLSB_FIRST=1' ]
  mednafen_source += '../src/cputest/x86_cpu.c'
elif arch == 'x86_64'
  mednafen_c_cpp_args += [ '-DARCH_X86=1', '-DARCH_X86_64=1', '-DLSB_FIRST=1' ]
  mednafen_source += '../src/cputest/x86_cpu.c'
elif arch == 'arm'
  mednafen_c_cpp_args += [ '-DLSB_FIRST=1' ]
elif arch == 'aarch64'
  mednafen_c_cpp_args += [ '-DLSB_FIRST=1' ]
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
  install_dir : core_dir,
  install : true,
)

descriptor = configuration_data()
descriptor.set('version', version)
descriptor.set('commit', commit)
configure_file(
  input: 'mednafen.highscore.in',
  output: 'mednafen.highscore',
  configuration: descriptor,
  install_dir: core_dir,
  install: true
)
