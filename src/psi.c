/*****************************************************************************
 * psi.c: common PSI functions
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"

/*****************************************************************************
 * dvbpsi_NewPSISection
 *****************************************************************************
 * Creation of a new dvbpsi_psi_section_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_NewPSISection(int i_max_size)
{
    /* Allocate the dvbpsi_psi_section_t structure */
    dvbpsi_psi_section_t * p_section
                  = (dvbpsi_psi_section_t*)calloc(1, sizeof(dvbpsi_psi_section_t));
    if (p_section == NULL)
        return NULL;

    /* Allocate the p_data memory area */
    p_section->p_data = (uint8_t*)calloc(1, i_max_size * sizeof(uint8_t));
    if (p_section->p_data == NULL)
    {
        free(p_section);
        return NULL;
    }

    p_section->p_payload_end = p_section->p_data;
    p_section->p_next = NULL;

    return p_section;
}

/*****************************************************************************
 * dvbpsi_DeletePSISections
 *****************************************************************************
 * Destruction of a dvbpsi_psi_section_t structure.
 *****************************************************************************/
void dvbpsi_DeletePSISections(dvbpsi_psi_section_t *p_section)
{
    while(p_section != NULL)
    {
        dvbpsi_psi_section_t* p_next = p_section->p_next;

        if (p_section->p_data != NULL)
            free(p_section->p_data);

        free(p_section);
        p_section = p_next;
    }
}

/*****************************************************************************
 * dvbpsi_ValidPSISection
 *****************************************************************************
 * Check the CRC_32 if the section has b_syntax_indicator set.
 *****************************************************************************/
bool dvbpsi_ValidPSISection(dvbpsi_psi_section_t* p_section)
{
    if (p_section->b_syntax_indicator)
    {
        /* Check the CRC_32 if b_syntax_indicator is false */
        uint32_t i_crc = 0xffffffff;
        uint8_t* p_byte = p_section->p_data;

        while(p_byte < p_section->p_payload_end + 4)
        {
            i_crc = (i_crc << 8) ^ dvbpsi_crc32_table[(i_crc >> 24) ^ (*p_byte)];
            p_byte++;
        }

        if (i_crc == 0)
            return true;
        else
            return false;
    }
    else
    {
        /* No check to do if b_syntax_indicator is false */
        return false;
    }
}

/*****************************************************************************
 * dvbpsi_BuildPSISection
 *****************************************************************************
 * Build the section based on the information in the structure.
 *****************************************************************************/
void dvbpsi_BuildPSISection(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t* p_section)
{
  uint8_t* p_byte = p_section->p_data;

  /* table_id */
  p_section->p_data[0] = p_section->i_table_id;
  /* setion_syntax_indicator | private_indicator |
     first 4 MSB of section_length */
  p_section->p_data[1] =   (p_section->b_syntax_indicator ? 0x80 : 0x00)
                         | (p_section->b_private_indicator ? 0x40 : 0x00)
                         | 0x30 /* reserved bits set to 1 */
                         | ((p_section->i_length >> 8) & 0x0f);
  /* 8 LSB of section_length */
  p_section->p_data[2] = p_section->i_length & 0xff;

  /* Optional part of a PSI section */
  if (p_section->b_syntax_indicator)
  {
    /* 8 MSB of table_id_extension */
    p_section->p_data[3] = (p_section->i_extension >> 8) & 0xff;
    /* 8 LSB of table_id_extension */
    p_section->p_data[4] = p_section->i_extension & 0xff;
    /* 5 bits of version_number | current_next_indicator */
    p_section->p_data[5] =   0xc0 /* reserved bits set to 1 */
                           | ((p_section->i_version & 0x1f) << 1)
                           | (p_section->b_current_next ? 0x01 : 0x00);
    /* section_number */
    p_section->p_data[6] = p_section->i_number;
    /* last_section_number */
    p_section->p_data[7] = p_section->i_last_number;

    /* CRC_32 */
    p_section->i_crc = 0xffffffff;

    while (p_byte < p_section->p_payload_end)
    {
      p_section->i_crc =   (p_section->i_crc << 8)
                         ^ dvbpsi_crc32_table[(p_section->i_crc >> 24) ^ (*p_byte)];
      p_byte++;
    }

    p_section->p_payload_end[0] = (p_section->i_crc >> 24) & 0xff;
    p_section->p_payload_end[1] = (p_section->i_crc >> 16) & 0xff;
    p_section->p_payload_end[2] = (p_section->i_crc >> 8) & 0xff;
    p_section->p_payload_end[3] = p_section->i_crc & 0xff;

    if (!dvbpsi_ValidPSISection(p_section))
    {
      dvbpsi_error(p_dvbpsi, "misc PSI", "********************************************");
      dvbpsi_error(p_dvbpsi, "misc PSI", "* Generated PSI section has a bad CRC_32.  *");
      dvbpsi_error(p_dvbpsi, "misc PSI", "* THIS IS A BUG, PLEASE REPORT TO THE LIST *");
      dvbpsi_error(p_dvbpsi, "misc PSI", "*  ---  libdvbpsi-devel@videolan.org  ---  *");
      dvbpsi_error(p_dvbpsi, "misc PSI", "********************************************");
    }
  }
}
