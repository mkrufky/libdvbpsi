/*****************************************************************************
 * dvbpsi.c: conversion from TS packets to PSI sections
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id$
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

#include <string.h>
#include <stdio.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"


/*****************************************************************************
 * dvbpsi_PushPacket
 *****************************************************************************
 * Injection of a TS packet into a PSI decoder.
 *****************************************************************************/
void dvbpsi_PushPacket(dvbpsi_handle h_dvbpsi, uint8_t* p_data)
{
  uint8_t i_expected_counter;           /* Expected continuity counter */
  dvbpsi_psi_section_t* p_section;      /* Current section */
  uint8_t* p_payload_pos;               /* Where in the TS packet */
  uint8_t* p_new_pos = NULL;            /* Beginning of the new section,
                                           updated to NULL when the new
                                           section is handled */
  int i_available;                      /* Byte count available in the
                                           packet */

  /* TS start code */
  if(p_data[0] != 0x47)
  {
    DVBPSI_ERROR("PSI decoder", "not a TS packet");
    return;
  }

  /* Continuity check */
  i_expected_counter = (h_dvbpsi->i_continuity_counter + 1) & 0xf;
  h_dvbpsi->i_continuity_counter = p_data[3] & 0xf;

  if(i_expected_counter == ((h_dvbpsi->i_continuity_counter + 1) & 0xf))
  {
    DVBPSI_ERROR_ARG("PSI decoder",
                     "TS duplicate (received %d, expected %d) for PID %d",
                     h_dvbpsi->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);
    return;
  }

  if(i_expected_counter != h_dvbpsi->i_continuity_counter)
  {
    DVBPSI_ERROR_ARG("PSI decoder",
                     "TS discontinuity (received %d, expected %d) for PID %d",
                     h_dvbpsi->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);
    h_dvbpsi->b_discontinuity = 1;
    if(h_dvbpsi->p_current_section)
    {
      dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
      h_dvbpsi->p_current_section = NULL;
    }
  }

  /* Return if no payload in the TS packet */
  if(!(p_data[3] & 0x10))
  {
    return;
  }

  /* Skip the adaptation_field if present */
  if(p_data[3] & 0x20)
    p_payload_pos = p_data + 5 + p_data[4];
  else
    p_payload_pos = p_data + 4;

  /* Unit start -> skip the pointer_field and a new section begins */
  if(p_data[1] & 0x40)
  {
    p_new_pos = p_payload_pos + *p_payload_pos + 1;
    p_payload_pos += 1;
  }

  p_section = h_dvbpsi->p_current_section;

  /* If the psi decoder needs a begginning of section and a new section
     begins in the packet then initialize the dvbpsi_psi_section_t structure */
  if(p_section == NULL)
  {
    if(p_new_pos)
    {
      /* Allocation of the structure */
      h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
      /* Update the position in the packet */
      p_payload_pos = p_new_pos;
      /* New section is being handled */
      p_new_pos = NULL;
      /* Just need the header to know how long is the section */
      h_dvbpsi->i_need = 3;
      h_dvbpsi->b_complete_header = 0;
    }
    else
    {
      /* No new section => return */
      return;
    }
  }

  /* Remaining bytes in the payload */
  i_available = 188 + p_data - p_payload_pos;

  while(i_available > 0)
  {
    if(i_available >= h_dvbpsi->i_need)
    {
      /* There are enough bytes in this packet to complete the
         header/section */
      memcpy(p_section->p_payload_end, p_payload_pos, h_dvbpsi->i_need);
      p_payload_pos += h_dvbpsi->i_need;
      p_section->p_payload_end += h_dvbpsi->i_need;
      i_available -= h_dvbpsi->i_need;

      if(!h_dvbpsi->b_complete_header)
      {
        /* Header is complete */
        h_dvbpsi->b_complete_header = 1;
        /* Compute p_section->i_length and update h_dvbpsi->i_need */
        h_dvbpsi->i_need = p_section->i_length
                         =   ((uint16_t)(p_section->p_data[1] & 0xf)) << 8
                           | p_section->p_data[2];
        /* Check that the section isn't too long */
        if(h_dvbpsi->i_need > h_dvbpsi->i_section_max_size - 3)
        {
          DVBPSI_ERROR("PSI decoder", "PSI section too long");
          dvbpsi_DeletePSISections(p_section);
          h_dvbpsi->p_current_section = NULL;
          /* If there is a new section not being handled then go forward
             in the packet */
          if(p_new_pos)
          {
            h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
            p_payload_pos = p_new_pos;
            p_new_pos = NULL;
            h_dvbpsi->i_need = 3;
            h_dvbpsi->b_complete_header = 0;
            i_available = 188 + p_data - p_payload_pos;
          }
          else
          {
            i_available = 0;
          }
        }
      }
      else
      {
        /* PSI section is complete */
        p_section->b_syntax_indicator = p_section->p_data[1] & 0x80;
        p_section->b_private_indicator = p_section->p_data[1] & 0x40;
        /* Update the end of the payload if CRC_32 is present */
        if(p_section->b_syntax_indicator)
          p_section->p_payload_end -= 4;

        if(dvbpsi_ValidPSISection(p_section))
        {
          /* PSI section is valid */
          p_section->i_table_id = p_section->p_data[0];
          if(p_section->b_syntax_indicator)
          {
            p_section->i_extension =   (p_section->p_data[3] << 8)
                                     | p_section->p_data[4];
            p_section->i_version = (p_section->p_data[5] & 0x3e) >> 1;
            p_section->b_current_next = p_section->p_data[5] & 0x1;
            p_section->i_number = p_section->p_data[6];
            p_section->i_last_number = p_section->p_data[7];
            p_section->p_payload_start = p_section->p_data + 8;
          }
          else
          {
            p_section->p_payload_start = p_section->p_data + 3;
          }

          h_dvbpsi->pf_callback(h_dvbpsi, p_section);
          h_dvbpsi->p_current_section = NULL;
        }
        else
        {
          /* PSI section isn't valid => trash it */
          dvbpsi_DeletePSISections(p_section);
          h_dvbpsi->p_current_section = NULL;
        }

        /* If there is a new section not being handled then go forward
           in the packet */
        if(p_new_pos)
        {
          h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
          p_payload_pos = p_new_pos;
          p_new_pos = NULL;
          h_dvbpsi->i_need = 3;
          h_dvbpsi->b_complete_header = 0;
          i_available = 188 + p_data - p_payload_pos;
        }
        else
        {
          i_available = 0;
        }
      }
    }
    else
    {
      /* There aren't enough bytes in this packet to complete the
         header/section */
      memcpy(p_section->p_payload_end, p_payload_pos, i_available);
      p_section->p_payload_end += i_available;
      h_dvbpsi->i_need -= i_available;
      i_available = 0;
    }
  }
}

