/*****************************************************************************
 * psi.h: common PSI structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: psi.h,v 1.3 2002/03/15 12:16:01 bozo Exp $
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

#ifndef _DVBPSI_PSI_H_
#define _DVBPSI_PSI_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_psi_section_t
 *****************************************************************************
 * This structure is used to store a PSI section. The common information are
 * decoded (ISO/IEC 13818-1 section 2.4.4.10).
 *****************************************************************************/
struct dvbpsi_psi_section_s
{
  /* non-specific section data */
  uint8_t       i_table_id;             /* table_id */
  int           b_syntax_indicator;     /* section_syntax_indicator */
  int           b_private_indicator;    /* private_indicator */
  uint16_t      i_length;               /* section_length */

  /* used if b_syntax_indicator is true */
  uint16_t      i_extension;            /* table_id_extension */
                                        /* transport_stream_id for a
                                           PAT section */
  uint8_t       i_version;              /* version_number */
  int           b_current_next;         /* current_next_indicator */
  uint8_t       i_number;               /* section_number */
  uint8_t       i_last_number;          /* last_section_number */

  /* non-specific section data */
  /* the content is table-specific */
  uint8_t *     p_data;                 /* complete section */
  uint8_t *     p_payload_start;        /* payload start */
  uint8_t *     p_payload_end;          /* payload end */

  /* used if b_syntax_indicator is true */
  uint32_t      i_crc;                  /* CRC_32 */

  /* list handling */
  struct dvbpsi_psi_section_s *         p_next;

};


/*****************************************************************************
 * dvbpsi_NewPSISection
 *****************************************************************************
 * Creation of a new dvbpsi_psi_section_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t * dvbpsi_NewPSISection(int i_max_size);


/*****************************************************************************
 * dvbpsi_DeletePSISections
 *****************************************************************************
 * Destruction of a dvbpsi_psi_section_t structure.
 *****************************************************************************/
void dvbpsi_DeletePSISections(dvbpsi_psi_section_t * p_section);


/*****************************************************************************
 * dvbpsi_ValidPSISection
 *****************************************************************************
 * Check the CRC_32 if the section has b_syntax_indicator set.
 *****************************************************************************/
int dvbpsi_ValidPSISection(dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_BuildPSISection
 *****************************************************************************
 * Build the section based on the information in the structure.
 *****************************************************************************/
void dvbpsi_BuildPSISection(dvbpsi_psi_section_t* p_section);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of psi.h"
#endif

