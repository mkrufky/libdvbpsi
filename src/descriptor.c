/*****************************************************************************
 * descriptor.c: descriptors functions
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: descriptor.c,v 1.6 2002/10/07 14:15:14 sam Exp $
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


#include "config.h"

#include <stdlib.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "dvbpsi.h"
#include "descriptor.h"


/*****************************************************************************
 * dvbpsi_NewDescriptor
 *****************************************************************************
 * Creation of a new dvbpsi_descriptor_t structure.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_NewDescriptor(uint8_t i_tag, uint8_t i_length,
                                          uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                = (dvbpsi_descriptor_t*)malloc(sizeof(dvbpsi_descriptor_t));

  if(p_descriptor)
  {
    p_descriptor->p_data = (uint8_t*)malloc(i_length * sizeof(uint8_t));

    if(p_descriptor->p_data)
    {
      p_descriptor->i_tag = i_tag;
      p_descriptor->i_length = i_length;
      if(p_data)
        memcpy(p_descriptor->p_data, p_data, i_length);
      p_descriptor->p_decoded = NULL;
      p_descriptor->p_next = NULL;
    }
    else
    {
      free(p_descriptor);
      p_descriptor = NULL;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_DeleteDescriptors
 *****************************************************************************
 * Destruction of a dvbpsi_descriptor_t structure.
 *****************************************************************************/
void dvbpsi_DeleteDescriptors(dvbpsi_descriptor_t* p_descriptor)
{
  while(p_descriptor != NULL)
  { 
    dvbpsi_descriptor_t* p_next = p_descriptor->p_next;

    if(p_descriptor->p_data != NULL)
      free(p_descriptor->p_data);

    if(p_descriptor->p_decoded != NULL)
      free(p_descriptor->p_decoded);

    free(p_descriptor);
    p_descriptor = p_next;
  }
}

