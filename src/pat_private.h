/*****************************************************************************
 * pat_private.h: private PAT structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: pat_private.h,v 1.1 2002/01/07 18:30:35 bozo Exp $
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

#ifndef _DVBPSI_PAT_PRIVATE_H_
#define _DVBPSI_PAT_PRIVATE_H_


/*****************************************************************************
 * dvbpsi_pat_decoder_t
 *****************************************************************************
 * PAT decoder.
 *****************************************************************************/
typedef struct dvbpsi_pat_decoder_s
{
  dvbpsi_pat_callback   pf_callback;
  void *                p_cb_data;

  dvbpsi_pat_t          current_pat;
  dvbpsi_pat_t          next_pat;

  int                   b_building_next;
  uint8_t               i_previous_section_number;
  uint8_t               i_last_section_number;

  int                   b_section_lost;

} dvbpsi_pat_decoder_t;


/*****************************************************************************
 * dvbpsi_DecodePATSection
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_DecodePATSection(dvbpsi_decoder_t* p_decoder,
                             dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of pat_private.h"
#endif

