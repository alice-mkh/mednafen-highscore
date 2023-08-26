/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mednafen/mednafen.h>

namespace Mednafen
{

enum
{
 MOVIE_STOPPED = 0,
 MOVIE_PLAYING = 1,
 MOVIE_RECORDING = 2
};

bool MDFNMOV_IsPlaying(void) noexcept
{
 return false;
}

bool MDFNMOV_IsRecording(void) noexcept
{
 return false;
}

void MDFNI_SaveMovie(char *fname, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const int32 *LineWidths)
{
}

void MDFNMOV_Stop(void) noexcept
{
}

void MDFNI_LoadMovie(char *fname)
{
}

void MDFNMOV_ProcessInput(uint8 *PortData[], uint32 PortLen[], int NumPorts) noexcept
{
}

void MDFNMOV_AddCommand(uint8 cmd, uint32 data_len, uint8* data) noexcept
{
}

void MDFNMOV_RecordState(void) noexcept
{
}

void MDFNMOV_StateAction(StateMem* sm, const unsigned load)
{
}

void MDFNMOV_CheckMovies(void)
{
}

void MDFNI_SelectMovie(int w)
{
}

}
