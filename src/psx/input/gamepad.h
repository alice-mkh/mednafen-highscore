/******************************************************************************/
/* Mednafen Sony PS1 Emulation Module                                         */
/******************************************************************************/
/* gamepad.h:
**  Copyright (C) 2011-2023 Mednafen Team
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

#ifndef __MDFN_PSX_INPUT_GAMEPAD_H
#define __MDFN_PSX_INPUT_GAMEPAD_H

namespace MDFN_IEN_PSX
{

InputDevice *Device_Gamepad_Create(void) MDFN_COLD;
MDFN_HIDE extern IDIISG Device_Gamepad_IDII;
MDFN_HIDE extern IDIISG Device_Dancepad_IDII;
MDFN_HIDE extern IDIISG Device_PopnMusic_IDII;

}
#endif
