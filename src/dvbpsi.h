/*****************************************************************************
 * dvbpsi.h: main header
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: dvbpsi.h,v 1.1 2002/01/07 18:30:35 bozo Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef _DVBPSI_DVBPSI_H_
#define _DVBPSI_DVBPSI_H_


#include <inttypes.h>


/*****************************************************************************
 * dvbpsi_handle
 *****************************************************************************
 * Decoder abstration.
 *****************************************************************************/
typedef struct dvbpsi_decoder_s * dvbpsi_handle;


/*****************************************************************************
 * dvbpsi_PushPacket
 *****************************************************************************
 * Injection of a TS packet into a PSI decoder.
 *****************************************************************************/
void dvbpsi_PushPacket(dvbpsi_handle h_dvbpsi, uint8_t* p_data);


#else
#error "Multiple inclusions of dvbpsi.h"
#endif

