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

  guint current_disc;
  guint media_cb_id;
};

static void mednafen_neo_geo_pocket_core_init (HsNeoGeoPocketCoreInterface *iface);
static void mednafen_pc_engine_core_init (HsPcEngineCoreInterface *iface);
static void mednafen_pc_engine_cd_core_init (HsPcEngineCdCoreInterface *iface);
static void mednafen_virtual_boy_core_init (HsVirtualBoyCoreInterface *iface);
static void mednafen_wonderswan_core_init (HsWonderSwanCoreInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (MednafenCore, mednafen_core, HS_TYPE_CORE,
                               G_IMPLEMENT_INTERFACE (HS_TYPE_NEO_GEO_POCKET_CORE, mednafen_neo_geo_pocket_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_PC_ENGINE_CORE, mednafen_pc_engine_core_init)
                               G_IMPLEMENT_INTERFACE (HS_TYPE_PC_ENGINE_CD_CORE, mednafen_pc_engine_cd_core_init)
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
  // TODO: For Saturn, add %x to the name
  Mednafen::MDFNI_SetSetting ("filesys.fname_sav", save_path);

  if (platform == HS_PLATFORM_PC_ENGINE_CD) {
    if (!self->pce_cd_bios_path) {
      g_set_error (error, HS_CORE_ERROR, HS_CORE_ERROR_MISSING_BIOS, "Missing System Card 3.0 BIOS");
      return FALSE;
    }

    Mednafen::MDFNI_SetSetting ("pce_fast.cdbios", self->pce_cd_bios_path);
  }

  if (platform == HS_PLATFORM_PC_ENGINE_CD) {
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

  self->rom_path = g_strdup (rom_path);

  if (platform == HS_PLATFORM_PC_ENGINE_CD)
    Mednafen::MDFNI_SetMedia (0, 2, 0, 0);

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

#define MODE_SWITCH_MASK (1 << 12)

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
        *self->input_buffer[player] |= MODE_SWITCH_MASK;
      else
        *self->input_buffer[player] &= ~MODE_SWITCH_MASK;
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
}

static void
mednafen_core_reset (HsCore *core)
{
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

  // TODO: For Saturn, add %x to the name
  Mednafen::MDFNI_SetSetting ("filesys.fname_sav", save_path);

  g_autofree char *system_name = g_strdup (self->game->shortname);
  Mednafen::MDFNI_CloseGame ();
  self->game = Mednafen::MDFNI_LoadGame (system_name, &::Mednafen::NVFS, self->rom_path);

  return TRUE;
}

static gboolean
mednafen_core_sync_save (HsCore  *core,
                         GError **error)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  // Unfortunately, Mednafen only saves games on exit, so we have to do trickery

  // Make a temporary savestate
  g_autofree char *cache_path = hs_core_get_cache_path (core);
  g_autofree char *tmp_path = g_build_filename (cache_path, "tmp-save-XXXXXX", NULL);
  tmp_path = g_mkdtemp (tmp_path);

  g_autoptr (GFile) tmp_dir = g_file_new_for_path (tmp_path);
  g_autoptr (GFile) tmp_save = g_file_get_child (tmp_dir, "savestate");
  const char *tmp_save_path = g_file_peek_path (tmp_save);

  Mednafen::MDFNI_SaveState (tmp_save_path, "", NULL, NULL, NULL);

  // Close and reopen the game
  g_autofree char *system_name = g_strdup (self->game->shortname);
  Mednafen::MDFNI_CloseGame ();
  self->game = Mednafen::MDFNI_LoadGame (system_name, &::Mednafen::NVFS, self->rom_path);

  // Load the temporary savestate and delete the file
  Mednafen::MDFNI_LoadState (tmp_save_path, "");

  if (!g_file_delete (tmp_save, NULL, error))
    return FALSE;
  if (!g_file_delete (tmp_dir, NULL, error))
    return FALSE;

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
  // self->game->fps / 65536.0 / 256.0
  switch (hs_platform_get_base_platform (hs_core_get_platform (core))) {
  case HS_PLATFORM_NEO_GEO_POCKET:
    return 60.253016;
  case HS_PLATFORM_PC_ENGINE:
    return 59.826105;
  case HS_PLATFORM_VIRTUAL_BOY:
    return 50.273488;
  case HS_PLATFORM_WONDERSWAN:
    return 75.471698;
  default:
    g_assert_not_reached ();
  }
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

  if (platform != HS_PLATFORM_PC_ENGINE_CD)
    return;

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
