/*****************************************************************************
 * descriptor.h: common descriptor structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: descriptor.h,v 1.2 2002/03/15 12:16:01 bozo Exp $
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

#ifndef _DVBPSI_DESCRIPTOR_H_
#define _DVBPSI_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_descriptor_t
 *****************************************************************************
 * This structure is used to store a descriptor.
 * (ISO/IEC 13818-1 section 2.6).
 *****************************************************************************/
typedef struct dvbpsi_descriptor_s
{
  uint8_t                       i_tag;          /* descriptor_tag */
  uint8_t                       i_length;       /* descriptor_length */

  uint8_t *                     p_data;         /* content */

  struct dvbpsi_descriptor_s *  p_next;

} dvbpsi_descriptor_t;


/*****************************************************************************
 * dvbpsi_NewDescriptor
 *****************************************************************************
 * Creation of a new dvbpsi_descriptor_t structure.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_NewDescriptor(uint8_t i_tag, uint8_t i_length,
                                          uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_DeleteDescriptor
 *****************************************************************************
 * Destruction of a dvbpsi_descriptor_t structure.
 *****************************************************************************/
void dvbpsi_DeleteDescriptor(dvbpsi_descriptor_t* p_descriptor);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of descriptor.h"
#endif

