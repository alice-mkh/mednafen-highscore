#pragma once

#include <highscore/libhighscore.h>

G_BEGIN_DECLS

#define MEDNAFEN_TYPE_CORE (mednafen_core_get_type())

G_DECLARE_FINAL_TYPE (MednafenCore, mednafen_core, MEDNAFEN, CORE, HsCore)

GType hs_get_core_type (void);

G_END_DECLS
