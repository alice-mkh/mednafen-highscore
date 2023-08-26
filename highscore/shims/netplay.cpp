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

#include <zlib.h>
#include <trio/trio.h>

namespace Mednafen
{

int MDFNnetplay=0;

bool NetplaySendCommand(uint8 cmd, uint32 len, const void* data)
{
  return false;
}

void NetplaySendState(void)
{
}

void Netplay_Update(const uint32 PortDevIdx[], uint8* const PortData[], const uint32 PortLen[])
{
}

void Netplay_PostProcess(const uint32 PortDevIdx[], uint8* const PortData[], const uint32 PortLen[])
{
}

void MDFNI_NetplayDisconnect(void)
{
}

void MDFNI_NetplayLine(const char *text, bool &inputable, bool &viewable)
{
}

}
