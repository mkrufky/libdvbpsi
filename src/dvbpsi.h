/*****************************************************************************
 * dvbpsi.h: main header
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: dvbpsi.h,v 1.2 2002/01/22 20:30:16 bozo Exp $
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


/*****************************************************************************
 * The following definitions are just here to allow external decoders but
 * shouldn't be used for any other purpose.
 *****************************************************************************/

typedef struct dvbpsi_psi_section_s dvbpsi_psi_section_t;

/*****************************************************************************
 * dvbpsi_callback
 *****************************************************************************
 * Callback type definition.
 *****************************************************************************/
typedef void (* dvbpsi_callback)(dvbpsi_handle p_decoder,
                                 dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_decoder_t
 *****************************************************************************
 * PSI decoder.
 *****************************************************************************/
typedef struct dvbpsi_decoder_s
{
  dvbpsi_callback               pf_callback;

  void *                        p_private_decoder;

  int                           i_section_max_size;

  uint8_t                       i_continuity_counter;
  int                           b_discontinuity;

  dvbpsi_psi_section_t *        p_current_section;
  int                           i_need;
  int                           b_complete_header;

} dvbpsi_decoder_t;


#else
#error "Multiple inclusions of dvbpsi.h"
#endif

