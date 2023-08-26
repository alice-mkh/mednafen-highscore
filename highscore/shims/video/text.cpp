/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* text.cpp:
**  Copyright (C) 2005-2017 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
** European-centric fixed-width bitmap font text rendering, with some CJK support.
*/

#include <mednafen/video/video-common.h>
#include <mednafen/string/string.h>

namespace Mednafen
{

uint32 GetFontHeight(const unsigned fontid)
{
 return 0;
}

void MDFN_InitFontData(void)
{
}

uint32 GetGlyphWidth(char32_t cp, uint32 fontid)
{
 return 0;
}

uint32 GetTextPixLength(const char32_t* text, const size_t text_len, const uint32 fontid)
{
 return 0;
}

uint32 GetTextPixLength(const char* text, uint32 fontid)
{
 return 0;
}

uint32 GetTextPixLength(const char32_t* text, uint32 fontid)
{
 return 0;
}

uint32 DrawText(MDFN_Surface* surf, int32 x, int32 y, const char* text, uint32 color, uint32 fontid, uint32 hcenterw)
{
 return 0;
}

uint32 DrawText(MDFN_Surface* surf, const MDFN_Rect& cr, int32 x, int32 y, const char* text, uint32 color, uint32 fontid, uint32 hcenterw)
{
 return 0;
}

uint32 DrawTextShadow(MDFN_Surface* surf, int32 x, int32 y, const char* text, uint32 color, uint32 shadcolor, uint32 fontid, uint32 hcenterw)
{
 return 0;
}

uint32 DrawTextShadow(MDFN_Surface* surf, const MDFN_Rect& cr, int32 x, int32 y, const char* text, uint32 color, uint32 shadcolor, uint32 fontid, uint32 hcenterw)
{
 return 0;
}

uint32 DrawText(MDFN_Surface* surf, int32 x, int32 y, const char32_t* text, uint32 color, uint32 fontid, uint32 hcenterw)
{
  return 0;
}

uint32 DrawText(MDFN_Surface* surf, const MDFN_Rect& cr, int32 x, int32 y, const char32_t* text, uint32 color, uint32 fontid, uint32 hcenterw)
{
  return 0;
}

uint32 DrawTextShadow(MDFN_Surface* surf, int32 x, int32 y, const char32_t* text, uint32 color, uint32 shadcolor, uint32 fontid, uint32 hcenterw)
{
  return 0;
}

uint32 DrawTextShadow(MDFN_Surface* surf, const MDFN_Rect& cr, int32 x, int32 y, const char32_t* text, uint32 color, uint32 shadcolor, uint32 fontid, uint32 hcenterw)
{
  return 0;
}

}
