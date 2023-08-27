#include <mednafen/mednafen.h>
#include <mednafen/state-driver.h>

#include "mednafen-highscore.h"

#define SOUND_BUFFER_SIZE 0x10000
#define SAMPLE_RATE 44100

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
};

static void mednafen_virtual_boy_core_init (HsVirtualBoyCoreInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (MednafenCore, mednafen_core, HS_TYPE_CORE,
                               G_IMPLEMENT_INTERFACE (HS_TYPE_VIRTUAL_BOY_CORE, mednafen_virtual_boy_core_init))

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

static gboolean
mednafen_core_start (HsCore      *core,
                     const char  *rom_path,
                     const char  *save_path,
                     GError     **error)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

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
  Mednafen::MDFNI_SetSetting ("filesys.fname_sav", save_path);

  self->game = Mednafen::MDFNI_LoadGame ("vb", &::Mednafen::NVFS, rom_path);

  self->context = hs_core_create_software_context (core, self->game->fb_width, self->game->fb_height, HS_PIXEL_FORMAT_XRGB8888_REV);

  self->surface = new Mednafen::MDFN_Surface (hs_software_context_get_framebuffer (self->context),
                                              self->game->fb_width, self->game->fb_height, self->game->fb_width,
                                              Mednafen::MDFN_PixelFormat::ARGB32_8888);

  self->game->SetInput (0, "gamepad", (uint8_t *) self->input_buffer[0]);
  self->game->SetInput (1, "misc", (uint8_t *) self->input_buffer[1]); // TODO use this

  self->rom_path = g_strdup (rom_path);

  return TRUE;
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
}

static gboolean
mednafen_core_save_data (HsCore  *core,
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
  g_autofree char *tmp_save_path = g_file_get_path (tmp_save);

  Mednafen::MDFNI_SaveState (tmp_save_path, "", NULL, NULL, NULL);

  // Close and reopen the game
  Mednafen::MDFNI_CloseGame ();
  self->game = Mednafen::MDFNI_LoadGame ("vb", &::Mednafen::NVFS, self->rom_path);

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
  switch (hs_core_get_platform (core)) {
  case HS_PLATFORM_VIRTUAL_BOY:
    return 50.27;
  default:
    g_assert_not_reached ();
    // TODO
  }
}

static double
mednafen_core_get_aspect_ratio (HsCore *core)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  if (self->game == NULL)
    return 1;

  return self->game->nominal_width / (double) self->game->nominal_height;
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

static void
mednafen_core_finalize (GObject *object)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  for (guint i = 0; i < 13; i++)
    g_free (self->input_buffer[i]);

  g_free (self->sound_buffer);

  core = NULL;

  G_OBJECT_CLASS (mednafen_core_parent_class)->finalize (object);
}

static void
mednafen_core_class_init (MednafenCoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  HsCoreClass *core_class = HS_CORE_CLASS (klass);

  object_class->finalize = mednafen_core_finalize;

  core_class->start = mednafen_core_start;
  core_class->run_frame = mednafen_core_run_frame;
  core_class->reset = mednafen_core_reset;
  core_class->stop = mednafen_core_stop;

  core_class->save_data = mednafen_core_save_data;

  core_class->load_state = mednafen_core_load_state;
  core_class->save_state = mednafen_core_save_state;

  core_class->get_frame_rate = mednafen_core_get_frame_rate;
  core_class->get_aspect_ratio = mednafen_core_get_aspect_ratio;

  core_class->get_sample_rate = mednafen_core_get_sample_rate;
  core_class->get_channels = mednafen_core_get_channels;
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

const int vb_button_mapping[] = {
  9,  8,  7,  6,  // L_UP, L_DOWN, L_LEFT, L_RIGHT
  4,  13, 12, 5,  // R_UP, R_DOWN, R_LEFT, R_RIGHT
  0,  1,  11, 10, // A,    B,      SELECT, START
  2,  3,          // L,    R
};

static void
mednafen_virtual_boy_core_button_pressed (HsVirtualBoyCore *core, HsVirtualBoyButton button)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

  *self->input_buffer[0] |= 1 << vb_button_mapping[button];
}

static void
mednafen_virtual_boy_core_button_released (HsVirtualBoyCore *core, HsVirtualBoyButton button)
{
  MednafenCore *self = MEDNAFEN_CORE (core);

 *self->input_buffer[0] &= ~(1 << vb_button_mapping[button]);
}


static void
mednafen_virtual_boy_core_init (HsVirtualBoyCoreInterface *iface)
{
  iface->button_pressed = mednafen_virtual_boy_core_button_pressed;
  iface->button_released = mednafen_virtual_boy_core_button_released;
}

GType
hs_get_core_type (void)
{
  return MEDNAFEN_TYPE_CORE;
}
