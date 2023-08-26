/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* qtrecord.cpp:
**  Copyright (C) 2010-2020 Mednafen Team
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

#include <mednafen/mednafen.h>
#include <mednafen/Time.h>
#include <mednafen/qtrecord.h>

namespace Mednafen
{

QTRecord::QTRecord(const std::string& path, const VideoSpec &spec) : qtfile(path, FileStream::MODE_WRITE_SAFE), resampler(NULL)
{
}

void QTRecord::WriteFrame(const MDFN_Surface *surface, const MDFN_Rect &DisplayRect, const int32 *LineWidths,
			  const int16 *SoundBuf, const int32 SoundBufSize, const int64 MasterCycles)
{
}

void QTRecord::Finish(void)
{
}

QTRecord::~QTRecord(void)
{
}

}
