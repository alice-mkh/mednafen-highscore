#include <mednafen/mednafen.h>
#include <mednafen/state-driver.h>

#include "mednafen-highscore.h"

#define SOUND_BUFFER_SIZE 0x10000
#define SAMPLE_RATE 44100

#define PCE_OVERSCAN_TOP 4
#define PCE_OVERSCAN_BOTTOM 4

static MednafenCore *core;

struct _MednafenCore
{
  HsCore parent_instance;

  Mednafen::MDFNGI *game;
  Mednafen::MDFN_Surface *surface;

  HsSoftwareContext *context;

  uint32_t *input_buffer[13];
  int16_t *sound_buffer;

  char *rom_path;
  GFile *m3u_file;
  char *pce_cd_bios_path;
  char *psx_bios_path[HS_PLAYSTATION_BIOS_N_BIOS];
  char *ss_bios_path[HS_SEGA_SATURN_BIOS_N_BIOS];

  guint current_disc;
  guint media_cb_id;

  int ss_reset_counter;
};

static void mednafen_neo_geo_pocket_core_init (HsNeoGeoPocketCoreInterface *iface);
static void mednafen_pc_engine_core_init (HsPcEngineCoreInterface *iface);
static void mednafen_pc_engine_cd_core_init (HsPcEngineCdCoreInterface *iface);
static void mednafen_playstation_core_init (HsPlayStationCoreInterface *iface);
static void mednafen_sega_saturn_core_init (HsSegaSaturnCoreInterface *iface);
static void mednafen_virtual_boy_core_init (HsVirtualBoyCoreInterface *iface);
static void mednafen_wonderswan_core_init (HsWonderSwanCoreInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (MednafenCore, mednafen_core, HS_TYPE_CORE,
                               G_IMPLEMENT_INTERFACE (HS_TYPE_NEO_GEO_POCKET_CORE, mednafen_neo_geo_pocket_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_PC_ENGINE_CORE, mednafen_pc_engine_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_PC_ENGINE_CD_CORE, mednafen_pc_engine_cd_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_PLAYSTATION_CORE, mednafen_playstation_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_SEGA_SATURN_CORE, mednafen_sega_saturn_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_VIRTUAL_BOY_CORE, mednafen_virtual_boy_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_WONDERSWAN_CORE, mednafen_wonderswan_core_init))

void
Mednafen::MDFND_OutputInfo (const char *s) noexcept
{
  g_autofree char *message = g_strdup (s);

  // Trim the unwanted newline
  int len = strlen (message);
  if (message[len - 1] == '\n')
    message[len - 1] = '\0';

  hs_core_log (HS_CORE (core), HS_LOG_INFO, message);
}

void
Mednafen::MDFND_OutputNotice (MDFN_NoticeType t, const char* s) noexcept
{
  HsLogLevel level;

  switch (t) {
  case MDFN_NOTICE_STATUS:
    level = HS_LOG_DEBUG;
    break;
  case MDFN_NOTICE_WARNING:
    level = HS_LOG_WARNING;
    break;
  case MDFN_NOTICE_ERROR:
    level = HS_LOG_CRITICAL;
    break;
  default:
    g_assert_not_reached ();
  }

  hs_core_log (HS_CORE (core), level, s);
}

void
Mednafen::MDFND_MediaSetNotification(uint32 drive_idx, uint32 state_idx, uint32 media_idx, uint32 orientation_idx)
{
  if (state_idx == 0 || media_idx == core->current_disc)
    return;

  core->current_disc = media_idx;
  hs_core_notify_current_media (HS_CORE (core));
}

static GFile *
make_m3u (MednafenCore  *core,
          const char   **rom_paths,
          int            n_rom_paths,
          GError       **error)
{
  g_autoptr (GFileIOStream) iostream = NULL;
  GOutputStream *ostream;
  GDataOutputStream *stream;
  GFile *ret;
  int i;

  ret = g_file_new_tmp ("mednafen_highscore_XXXXXX.m3u", &iostream, error);
  if (!ret)
    return NULL;

  ostream = g_io_stream_get_output_stream (G_IO_STREAM (iostream));
  stream = g_data_output_stream_new (ostream);

  for (i = 0; i < n_rom_paths; i++) {
    g_data_output_stream_put_string (stream, rom_paths[i], NULL, error);
    g_data_output_stream_put_byte (stream, '\n', NULL, error);
  }

  g_io_stream_close (G_IO_STREAM (iostream), NULL, NULL);

  return ret;
}

static void
setup_controllers (MednafenCore *self)
{
  HsPlatform platform = hs_core_get_platform (HS_CORE (self));
  HsPlatform base_platform = hs_platform_get_base_platform (platform);

  switch (base_platform) {
  case HS_PLATFORM_NEO_GEO_POCKET:
    self->game->SetInput (0, "gamepad", (uint8_t *) self->input_buffer[0]);
    break;
  case HS_PLATFORM_PC_ENGINE:
    self->game->SetInput (0, "gamepad", (uint8_t *) self->input_buffer[0]);
    self->game->SetInput (1, "gamepad", (uint8_t *) self->input_buffer[1]);
    self->game->SetInput (2, "gamepad", (uint8_t *) self->input_buffer[2]);
    self->game->SetInput (3, "gamepad", (uint8_t *) self->input_buffer[3]);
    self->game->SetInput (4, "gamepad", (uint8_t *) self->input_buffer[4]);
    break;
  case HS_PLATFORM_PLAYSTATION:
    self->game->SetInput (0, "dualshock", (uint8_t *) self->input_buffer[0]);
    self->game->SetInput (1, "dualshock", (uint8_t *) self->input_buffer[1]);
    self->game->SetInput (2, "dualshock", (uint8_t *) self->input_buffer[2]);
    self->game->SetInput (3, "dualshock", (uint8_t *) self->input_buffer[3]);
    break;
  case HS_PLATFORM_SEGA_SATURN:
    for (int i = 0; i < 12; i++)
      self->game->SetInput (i, "gamepad", (uint8_t *) self->input_buffer[i]);
    self->game->SetInput (12, "builtin", (uint8_t *) self->input_buffer[12]); // reset button status
    break;
  case HS_PLATFORM_VIRTUAL_BOY:
    self->game->SetInput (0, "gamepad", (uint8_t *) self->input_buffer[0]);
    self->game->SetInput (1, "misc", (uint8_t *) self->input_buffer[1]); // TODO use this
    break;
  case HS_PLATFORM_WONDERSWAN:
    self->game->SetInput (0, "gamepad", (uint8_t *) self->input_buffer[0]);
    break;
  default:
    g_assert_not_reached ();
  }
}

static gboolean
try_migrate_libretro_save (MednafenCore  *self,
                           const char    *save_path,
                           GError       **error)
{
  HsPlatform platform = hs_core_get_platform (HS_CORE (self));
  HsPlatform base_platform = hs_platform_get_base_platform (platform);

  if (base_platform == HS_PLATFORM_SEGA_SATURN) {
    g_autoptr (GFile) save_dir = g_file_new_for_path (save_path);
    g_autoptr (GFileEnumerator) enumerator = NULL;
    g_autoptr (GFile) bkr_file = NULL;
    g_autoptr (GFile) bkr_dest = NULL;
    g_autoptr (GFile) bcr_file = NULL;
    g_autoptr (GFile) bcr_dest = NULL;
    g_autoptr (GFile) arp_file = NULL;
    g_autoptr (GFile) arp_dest = NULL;
    g_autoptr (GFile) seep_file = NULL;
    g_autoptr (GFile) seep_dest = NULL;
    g_autoptr (GFile) smpc_file = NULL;
    g_autoptr (GFile) smpc_dest = NULL;
    GFileInfo *info;

    if (!g_file_query_exists (save_dir, NULL))
      return TRUE;

    enumerator =
      g_file_enumerate_children (save_dir, G_FILE_ATTRIBUTE_STANDARD_NAME,
                                 G_FILE_QUERY_INFO_NONE, NULL, error);
    if (!enumerator)
      return FALSE;

    while ((info = g_file_enumerator_next_file (enumerator, NULL, error))) {
      const char *filename = g_file_info_get_name (info);

      if (g_str_has_suffix (filename, ".bkr") && g_strcmp0 (filename, "save.bkr") && !bkr_file) {
        bkr_file = g_file_get_child (save_dir, filename);
        bkr_dest = g_file_get_child (save_dir, "save.bkr");
      } else if (g_str_has_suffix (filename, ".bcr") && g_strcmp0 (filename, "save.bcr") && !bcr_file) {
        bcr_file = g_file_get_child (save_dir, filename);
        bcr_dest = g_file_get_child (save_dir, "save.bcr");
      } else if (g_str_has_suffix (filename, ".arp") && g_strcmp0 (filename, "save.arp") && !arp_file) {
        arp_file = g_file_get_child (save_dir, filename);
        arp_dest = g_file_get_child (save_dir, "save.arp");
      } else if (g_str_has_suffix (filename, ".seep") && g_strcmp0 (filename, "save.seep") && !seep_file) {
        seep_file = g_file_get_child (save_dir, filename);
        seep_dest = g_file_get_child (save_dir, "save.seep");
      } else if (g_str_has_suffix (filename, ".smpc") && g_strcmp0 (filename, "save.smpc") && !smpc_file) {
        smpc_file = g_file_get_child (save_dir, filename);
        smpc_dest = g_file_get_child (save_dir, "save.smpc");
      }

      g_object_unref (info);
    }

    if (bkr_file && !g_file_move (bkr_file, bkr_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
      return FALSE;
    if (bcr_file && !g_file_move (bcr_file, bcr_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
      return FALSE;
    if (arp_file && !g_file_move (arp_file, arp_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
      return FALSE;
    if (seep_file && !g_file_move (seep_file, seep_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
      return FALSE;
    if (smpc_file && !g_file_move (smpc_file, smpc_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
      return FALSE;

    if (bkr_file || bcr_file || arp_file || seep_file || smpc_file)
      hs_core_log (HS_CORE (self), HS_LOG_MESSAGE, "Libretro save files migrated successfully");

    return TRUE;
  }

  return TRUE;
}

static gboolean
set_save_path (MednafenCore  *self,
               const char    *save_path,
               GError       **error)
{
  HsPlatform platform = hs_core_get_platform (HS_CORE (self));
  HsPlatform base_platform = hs_platform_get_base_platform (platform);

  if (base_platform == HS_PLATFORM_SEGA_SATURN) {
    g_autofree char *path_with_ext = g_build_filename (save_path, "save.%x", NULL);
    g_autoptr (GFile) save_dir = g_file_new_for_path (save_path);

    if (!g_file_query_exists (save_dir, NULL) &&
        !g_file_make_directory_with_parents (save_dir, NULL, error)) {
      return FALSE;
    }

    Mednafen::MDFNI_SetSetting ("filesys.fname_sav", path_with_ext);
    return TRUE;
  }

  Mednafen::MDFNI_SetSetting ("filesys.fname_sav", save_path);
  return TRUE;
}

static gboolean
mednafen_core_load_rom (HsCore      *core,
                        const char **rom_paths,
                        int          n_rom_paths,
                        const char  *save_path,
                        GError     **error)
{
  MednafenCore *self = MEDNAFEN_CORE (core);
  HsPlatform platform = hs_core_get_platform (core);
  HsPlatform base_platform = hs_platform_get_base_platform (platform);
  const char *rom_path;

  if (!try_migrate_libretro_save (self, save_path, error))
    return FALSE;

  if (!Mednafen::MDFNI_Init ()) {
    g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to initialize Mednafen");
    return FALSE;
  }

  g_autofree char *cache_dir = hs_core_get_cache_path (core);
  if (!Mednafen::MDFNI_InitFinalize (cache_dir)) {
    g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to finish initializing Mednafen");
    return FALSE;
  }

  Mednafen::MDFNI_SetSetting ("filesys.path_sav", "");

  if (!set_save_path (self, save_path, error))
    return FALSE;

  if (platform == HS_PLATFORM_PC_ENGINE_CD) {
    if (!self->pce_cd_bios_path) {
      g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_MISSING_BIOS, "Missing System Card 3.0 BIOS");
      return FALSE;
    }

    Mednafen::MDFNI_SetSetting ("pce_fast.cdbios", self->pce_cd_bios_path);
  }

  if (platform == HS_PLATFORM_PLAYSTATION) {
    if (self->psx_bios_path[HS_PLAYSTATION_BIOS_JP])
      Mednafen::MDFNI_SetSetting ("psx.bios_jp", self->psx_bios_path[HS_PLAYSTATION_BIOS_JP]);
    if (self->psx_bios_path[HS_PLAYSTATION_BIOS_US])
      Mednafen::MDFNI_SetSetting ("psx.bios_na", self->psx_bios_path[HS_PLAYSTATION_BIOS_US]);
    if (self->psx_bios_path[HS_PLAYSTATION_BIOS_EU])
      Mednafen::MDFNI_SetSetting ("psx.bios_eu", self->psx_bios_path[HS_PLAYSTATION_BIOS_EU]);

    Mednafen::MDFNI_SetSetting ("psx.h_overscan", "0");
  }

  if (platform == HS_PLATFORM_SEGA_SATURN) {
    if (self->ss_bios_path[HS_SEGA_SATURN_BIOS_JP])
      Mednafen::MDFNI_SetSetting ("ss.bios_jp", self->ss_bios_path[HS_SEGA_SATURN_BIOS_JP]);
    if (self->ss_bios_path[HS_SEGA_SATURN_BIOS_US_EU])
      Mednafen::MDFNI_SetSetting ("ss.bios_na_eu", self->ss_bios_path[HS_SEGA_SATURN_BIOS_US_EU]);

    Mednafen::MDFNI_SetSetting ("ss.h_overscan", "0");
  }

  if (platform == HS_PLATFORM_PC_ENGINE_CD ||
      platform == HS_PLATFORM_PLAYSTATION ||
      platform == HS_PLATFORM_SEGA_SATURN) {
    if (n_rom_paths > 1) {
      // Make m3u work
      Mednafen::MDFNI_SetSetting ("filesys.untrusted_fip_check", "0");

      self->m3u_file = make_m3u (self, rom_paths, n_rom_paths, error);
      if (!self->m3u_file)
        return FALSE;

      rom_path = g_file_peek_path (self->m3u_file);
    } else {
      rom_path = rom_paths[0];
    }
  } else {
    g_assert (n_rom_paths == 1);
    rom_path = rom_paths[0];
  }

  const char *platform_name;

  switch (base_platform) {
  case HS_PLATFORM_NEO_GEO_POCKET:
    platform_name = "ngp";
    break;
  case HS_PLATFORM_PC_ENGINE:
    platform_name = "pce_fast";
    break;
  case HS_PLATFORM_PLAYSTATION:
    platform_name = "psx";
    break;
  case HS_PLATFORM_SEGA_SATURN:
    platform_name = "ss";
    break;
  case HS_PLATFORM_VIRTUAL_BOY:
    platform_name = "vb";
    break;
  case HS_PLATFORM_WONDERSWAN:
    platform_name = "wswan";
    break;
  default:
    g_assert_not_reached ();
  }

  self->game = Mednafen::MDFNI_LoadGame (platform_name, &::Mednafen::NVFS, rom_path);
  if (!self->game) {
    g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to load game");
    return FALSE;
  }

  self->context = hs_core_create_software_context (core, self->game->fb_width, self->game->fb_height, HS_PIXEL_FORMAT_XRGB8888_REV);

  self->surface = new Mednafen::MDFN_Surface (hs_software_context_get_framebuffer (self->context),
                                              self->game->fb_width, self->game->fb_height, self->game->fb_width,
                                              Mednafen::MDFN_PixelFormat::ARGB32_8888);

  setup_controllers (self);

  self->rom_path = g_strdup (rom_path);

  if (platform == HS_PLATFORM_PC_ENGINE_CD ||
      platform == HS_PLATFORM_PLAYSTATION ||
      platform == HS_PLATFORM_SEGA_SATURN) {
    Mednafen::MDFNI_SetMedia (0, 2, 0, 0);
  }

  return TRUE;
}

const int NGP_BUTTON_MAPPING[] = {
  0, 1, 2, 3,  // UP, DOWN, LEFT, RIGHT
  4, 5, 6,     // A, B, OPTION
};

const int PCE_BUTTON_MAPPING[] = {
  4, 6, 7,  5,  // UP, DOWN, LEFT, RIGHT
  0, 1,         // I, II
  8, 9, 10, 11, // III, IV, V, VI
  2, 3          // SELECT, RUN
};

#define PCE_MODE_SWITCH_MASK (1 << 12)

const int PSX_BUTTON_MAPPING[] = {
  4,  6,  7,  5,  // UP, DOWN, LEFT, RIGHT
  12, 15, 13, 14, // TRIANGLE, SQUARE, CIRCLE, CROSS
  10, 8,  1,      // L1, L2, L3
  11, 9,  2,      // R1, R2, R3
  0,  3,          // SELECT, START
};

#define PSX_MODE_SWITCH_MASK (1 << 16)

const int PSX_STICK_MAPPING[] = {
  7, 9, // L(x, y)
  3, 5, // R(x, y)
};

const int SS_BUTTON_MAPPING[] = {
  4,  5, 6, 7, // UP, DOWN, LEFT, RIGHT
  10, 8, 9,    // A, B, C
  2,  1, 0,    // X, Y, Z
  15, 3, 11,   // L, R, START
};

const int VB_BUTTON_MAPPING[] = {
  9,  8,  7,  6,  // L_UP, L_DOWN, L_LEFT, L_RIGHT
  4,  13, 12, 5,  // R_UP, R_DOWN, R_LEFT, R_RIGHT
  0,  1,  11, 10, // A,    B,      SELECT, START
  2,  3,          // L,    R
};

const int WS_BUTTON_MAPPING[] = {
  0, 1,  2, 3, // X1, X2, X3, X4
  4, 5,  6, 7, // Y1, Y2, Y3, Y4
  9, 10, 8,    // A, B, START
};

static void
mednafen_core_poll_input (HsCore *core, HsInputState *input_state)
{
  MednafenCore *self = MEDNAFEN_CORE (core);
  HsPlatform platform = hs_core_get_platform (core);
  HsPlatform base_platform = hs_platform_get_base_platform (platform);

  if (base_platform == HS_PLATFORM_NEO_GEO_POCKET) {
    uint32 buttons = input_state->neo_geo_pocket.buttons;

    for (int btn = 0; btn < HS_NEO_GEO_POCKET_N_BUTTONS; btn++) {
      if (buttons & 1 << btn)
        *self->input_buffer[0] |= 1 << NGP_BUTTON_MAPPING[btn];
      else
        *self->input_buffer[0] &= ~(1 << NGP_BUTTON_MAPPING[btn]);
    }

    return;
  }

  if (base_platform == HS_PLATFORM_PC_ENGINE) {
    for (int player = 0; player < HS_PC_ENGINE_MAX_PLAYERS; player++) {
      uint32 buttons = input_state->pc_engine.pad_buttons[player];

      for (int btn = 0; btn < HS_PC_ENGINE_N_BUTTONS; btn++) {
        if (buttons & 1 << btn)
          *self->input_buffer[player] |= 1 << PCE_BUTTON_MAPPING[btn];
        else
          *self->input_buffer[player] &= ~(1 << PCE_BUTTON_MAPPING[btn]);
      }

      if (input_state->pc_engine.pad_mode[player] == HS_PC_ENGINE_SIX_BUTTONS)
        *self->input_buffer[player] |= PCE_MODE_SWITCH_MASK;
      else
        *self->input_buffer[player] &= ~PCE_MODE_SWITCH_MASK;
    }

    return;
  }

  if (base_platform == HS_PLATFORM_PLAYSTATION) {
    for (int player = 0; player < HS_PLAYSTATION_MAX_PLAYERS; player++) {
      uint32 buttons = input_state->psx.pad_buttons[player];
      uint8_t *buf = (uint8_t *) self->input_buffer[player];

      for (int btn = 0; btn < HS_PLAYSTATION_N_BUTTONS; btn++) {
        if (buttons & 1 << btn)
          *self->input_buffer[player] |= 1 << PSX_BUTTON_MAPPING[btn];
        else
          *self->input_buffer[player] &= ~(1 << PSX_BUTTON_MAPPING[btn]);
      }

/* TODO
      *self->input_buffer[player] |= PSX_MODE_SWITCH_MASK;
*/

      for (int stick = 0; stick < HS_PLAYSTATION_N_STICKS; stick++) {
        double x = input_state->psx.pad_sticks_x[HS_PLAYSTATION_N_STICKS * player + stick];
        double y = input_state->psx.pad_sticks_y[HS_PLAYSTATION_N_STICKS * player + stick];

        double multiplier = 1.33;
        // 30712 / cos(2*pi/8) / 32767 = 1.33
        if (x < 0)
          x = -MIN (floor (0.5 + ABS (x) * 32767 * multiplier), 32767);
        else
          x = MIN (floor (0.5 + ABS (x) * 32767 * multiplier), 32767);

        if (y < 0)
          y = -MIN (floor (0.5 + ABS (y) * 32767 * multiplier), 32767);
        else
          y = MIN (floor (0.5 + ABS (y) * 32767 * multiplier), 32767);

        Mednafen::MDFN_en16lsb (&buf[PSX_STICK_MAPPING[stick * 2]],     x + 32767);
        Mednafen::MDFN_en16lsb (&buf[PSX_STICK_MAPPING[stick * 2 + 1]], y + 32767);
      }
    }

    return;
  }

  if (base_platform == HS_PLATFORM_SEGA_SATURN) {
    for (int player = 0; player < HS_SEGA_SATURN_MAX_PLAYERS; player++) {
      uint32 buttons = input_state->saturn.pad_buttons[player];

      for (int btn = 0; btn < HS_SEGA_SATURN_N_BUTTONS; btn++) {
        if (buttons & 1 << btn)
          *self->input_buffer[player] |= 1 << SS_BUTTON_MAPPING[btn];
        else
          *self->input_buffer[player] &= ~(1 << SS_BUTTON_MAPPING[btn]);
      }
    }

    return;
  }

  if (base_platform == HS_PLATFORM_VIRTUAL_BOY) {
    uint32 buttons = input_state->virtual_boy.buttons;

    for (int btn = 0; btn < HS_VIRTUAL_BOY_N_BUTTONS; btn++) {
      if (buttons & 1 << btn)
        *self->input_buffer[0] |= 1 << VB_BUTTON_MAPPING[btn];
      else
        *self->input_buffer[0] &= ~(1 << VB_BUTTON_MAPPING[btn]);
    }

    return;
  }

  if (base_platform == HS_PLATFORM_WONDERSWAN) {
    uint32 buttons = input_state->wonderswan.buttons;

    for (int btn = 0; btn < HS_WONDERSWAN_N_BUTTONS; btn++) {
      if (buttons & 1 << btn)
        *self->input_buffer[0] |= 1 << WS_BUTTON_MAPPING[btn];
      else
        *self->input_buffer[0] &= ~(1 << WS_BUTTON_MAPPING[btn]);
    }

    return;
  }

  g_assert_not_reached ();
}

static void
mednafen_core_run_frame (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);
  int32 rects[self->game->fb_height];

  memset (rects, 0, self->game->fb_height * sizeof (int32_t));
  rects[0] = ~0;

  Mednafen::EmulateSpecStruct spec;
  spec.surface = self->surface;
  spec.SoundRate = SAMPLE_RATE;
  spec.SoundBuf = self->sound_buffer;
  spec.LineWidths = rects;
  spec.SoundBufMaxSize = SOUND_BUFFER_SIZE;
  spec.SoundVolume = 1.0;
  spec.soundmultiplier = 1.0;

  Mednafen::MDFNI_Emulate (&spec);

  int width = 0;
  if (self->game->multires)
    width = rects[spec.DisplayRect.y];
  else
    width = spec.DisplayRect.w ?: rects[spec.DisplayRect.y];

  HsRectangle rect = { spec.DisplayRect.x, spec.DisplayRect.y, width, spec.DisplayRect.h };
  hs_software_context_set_area (self->context, &rect);

  hs_core_play_samples (core, self->sound_buffer, spec.SoundBufSize * self->game->soundchan);

  if (hs_core_get_platform (core) == HS_PLATFORM_SEGA_SATURN && self->ss_reset_counter > 0) {
    self->ss_reset_counter--;

    if (self->ss_reset_counter == 0)
      *self->input_buffer[12] = 0;
  }
}

static void
mednafen_core_reset (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  // Saturn has a reset button instead of implementing the usual reset function
  // This button has to be held for a few frames before releasing
  if (hs_core_get_platform (core) == HS_PLATFORM_SEGA_SATURN) {
    *self->input_buffer[12] = 1;
    self->ss_reset_counter = 3;
    return;
  }

  Mednafen::MDFNI_Reset ();
}

static void
mednafen_core_stop (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  Mednafen::MDFNI_CloseGame ();
  Mednafen::MDFNI_Kill ();

  g_clear_pointer (&self->rom_path, g_free);

  if (self->m3u_file) {
    g_autoptr (GError) error = NULL;

    if (!g_file_delete (self->m3u_file, NULL, &error)) {
      g_autofree char *message = g_strdup_printf ("Failed to delete the m3u file: %s", error->message);
      hs_core_log (HS_CORE (core), HS_LOG_WARNING, message);
    }

    g_clear_object (&self->m3u_file);
  }
}

static gboolean
mednafen_core_reload_save (HsCore      *core,
                           const char  *save_path,
                           GError    **error)
{
  MednafenCore *self = MEDNAFEN_CORE (core);
  HsPlatform platform = hs_core_get_platform (core);

  if (!set_save_path (self, save_path, error))
    return FALSE;

  g_autofree char *system_name = g_strdup (self->game->shortname);
  Mednafen::MDFNI_CloseGame ();
  self->game = Mednafen::MDFNI_LoadGame (system_name, &::Mednafen::NVFS, self->rom_path);
  if (!self->game) {
    g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to load game");
    return FALSE;
  }

  setup_controllers (self);

  if (platform == HS_PLATFORM_PC_ENGINE_CD ||
      platform == HS_PLATFORM_PLAYSTATION ||
      platform == HS_PLATFORM_SEGA_SATURN) {
    Mednafen::MDFNI_SetMedia (0, 2, self->current_disc, 0);
  }

  return TRUE;
}

static gboolean
mednafen_core_sync_save (HsCore  *core,
                         GError **error)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  if (self->game->SyncSave)
    self->game->SyncSave();

  return TRUE;
}

static void
mednafen_core_load_state (HsCore          *core,
                          const char      *path,
                          HsStateCallback  callback)
{
  if (!Mednafen::MDFNI_LoadState (path, "")) {
    GError *error = NULL;
    g_set_error (&error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to load state");
    callback (core, &error);
    return;
  }

  callback (core, NULL);
}

static void
mednafen_core_save_state (HsCore          *core,
                          const char      *path,
                          HsStateCallback  callback)
{
  if (!Mednafen::MDFNI_SaveState (path, "", NULL, NULL, NULL)) {
    GError *error = NULL;
    g_set_error (&error, HS_CORE_ERROR, HS_CORE_ERROR_INTERNAL, "Failed to save state");
    callback (core, &error);
    return;
  }

  callback (core, NULL);
}

static double
mednafen_core_get_frame_rate (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  return self->game->fps / 65536.0 / 256.0;
}

static double
mednafen_core_get_aspect_ratio (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  if (self->game == NULL)
    return 1;

  switch (hs_platform_get_base_platform (hs_core_get_platform (core))) {
  case HS_PLATFORM_PC_ENGINE:
    return 256.0 / (240.0 - PCE_OVERSCAN_TOP - PCE_OVERSCAN_BOTTOM) * 8.0 / 7.0;
  default:
    return self->game->nominal_width / (double) self->game->nominal_height;
  }
}

static double
mednafen_core_get_sample_rate (HsCore *core)
{
  return SAMPLE_RATE;
}

static int
mednafen_core_get_channels (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  return self->game->soundchan;
}

static guint
mednafen_core_get_current_media (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  return self->current_disc;
}

static void
set_media_cb (gpointer data)
{
  guint media = GPOINTER_TO_UINT (data);

  Mednafen::MDFNI_SetMedia (0, 2, media, 0);

  core->media_cb_id = 0;
}

static void
mednafen_core_set_current_media (HsCore *core, guint media)
{
  MednafenCore *self = MEDNAFEN_CORE (core);
  HsPlatform platform = hs_core_get_platform (core);

  if (platform != HS_PLATFORM_PC_ENGINE_CD &&
      platform != HS_PLATFORM_PLAYSTATION &&
      platform != HS_PLATFORM_SEGA_SATURN) {
    return;
  }

  g_clear_handle_id (&self->media_cb_id, g_source_remove);

  self->current_disc = media;
  hs_core_notify_current_media (core);

  Mednafen::MDFNI_SetMedia (0, 0, 0, 0);

  self->media_cb_id = g_timeout_add_once (1000, (GSourceOnceFunc) set_media_cb, GUINT_TO_POINTER (media));
}

static void
mednafen_core_finalize (GObject *object)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  g_clear_handle_id (&self->media_cb_id, g_source_remove);

  for (guint i = 0; i < 13; i++)
    g_free (self->input_buffer[i]);

  g_free (self->sound_buffer);
  g_free (self->pce_cd_bios_path);

  for (int i = 0; i < HS_PLAYSTATION_BIOS_N_BIOS; i++)
    g_free (self->psx_bios_path[i]);

  for (int i = 0; i < HS_SEGA_SATURN_BIOS_N_BIOS; i++)
    g_free (self->ss_bios_path[i]);

  core = NULL;

  G_OBJECT_CLASS (mednafen_core_parent_class)->finalize (object);
}

static void
mednafen_core_class_init (MednafenCoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  HsCoreClass *core_class = HS_CORE_CLASS (klass);

  object_class->finalize = mednafen_core_finalize;

  core_class->load_rom = mednafen_core_load_rom;
  core_class->poll_input = mednafen_core_poll_input;
  core_class->run_frame = mednafen_core_run_frame;
  core_class->reset = mednafen_core_reset;
  core_class->stop = mednafen_core_stop;

  core_class->reload_save = mednafen_core_reload_save;
  core_class->sync_save = mednafen_core_sync_save;

  core_class->load_state = mednafen_core_load_state;
  core_class->save_state = mednafen_core_save_state;

  core_class->get_frame_rate = mednafen_core_get_frame_rate;
  core_class->get_aspect_ratio = mednafen_core_get_aspect_ratio;

  core_class->get_sample_rate = mednafen_core_get_sample_rate;
  core_class->get_channels = mednafen_core_get_channels;

  core_class->get_current_media = mednafen_core_get_current_media;
  core_class->set_current_media = mednafen_core_set_current_media;
}

static void
mednafen_core_init (MednafenCore *self)
{
  g_assert (!core);

  core = self;

  for (guint i = 0; i < 13; i++)
    self->input_buffer[i] = g_new0 (uint32_t, 9);

  self->sound_buffer = g_new0 (int16_t, SOUND_BUFFER_SIZE);
}

static void
mednafen_neo_geo_pocket_core_init (HsNeoGeoPocketCoreInterface *iface)
{
}

static void
mednafen_pc_engine_core_init (HsPcEngineCoreInterface *iface)
{
}

static void
mednafen_pc_engine_cd_core_set_bios_path (HsPcEngineCdCore *core, const char *path)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  g_set_str (&self->pce_cd_bios_path, path);
}

static void
mednafen_pc_engine_cd_core_init (HsPcEngineCdCoreInterface *iface)
{
  iface->set_bios_path = mednafen_pc_engine_cd_core_set_bios_path;
}

static void
mednafen_playstation_core_set_bios_path (HsPlayStationCore *core, HsPlayStationBios type, const char *path)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  g_set_str (&self->psx_bios_path[type], path);
}

static void
mednafen_playstation_core_init (HsPlayStationCoreInterface *iface)
{
  iface->set_bios_path = mednafen_playstation_core_set_bios_path;
}

static void
mednafen_sega_saturn_core_set_bios_path (HsSegaSaturnCore *core, HsSegaSaturnBios type, const char *path)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  g_set_str (&self->ss_bios_path[type], path);
}

static void
mednafen_sega_saturn_core_init (HsSegaSaturnCoreInterface *iface)
{
  iface->set_bios_path = mednafen_sega_saturn_core_set_bios_path;
}

static void
mednafen_virtual_boy_core_init (HsVirtualBoyCoreInterface *iface)
{
}

static void
mednafen_wonderswan_core_init (HsWonderSwanCoreInterface *iface)
{
}

GType
hs_get_core_type (void)
{
  return MEDNAFEN_TYPE_CORE;
}
