/*****************************************************************************
 * pat.h: PAT structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: pat.h,v 1.1 2002/01/22 20:30:16 bozo Exp $
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

#ifndef _DVBPSI_PAT_H_
#define _DVBPSI_PAT_H_


/*****************************************************************************
 * dvbpsi_pat_program_t
 *****************************************************************************
 * This structure is used to store a decoded PAT program.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 *****************************************************************************/
typedef struct dvbpsi_pat_program_s
{
  uint16_t                      i_number;               /* program_number */
  uint16_t                      i_pid;                  /* PID of NIT/PMT */

  struct dvbpsi_pat_program_s * p_next;

} dvbpsi_pat_program_t;


/*****************************************************************************
 * dvbpsi_pat_t
 *****************************************************************************
 * This structure is used to store a decoded PAT.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 *****************************************************************************/
typedef struct dvbpsi_pat_s
{
  uint16_t                  i_ts_id;            /* transport_stream_id */
  uint8_t                   i_version;          /* version_number */
  int                       b_current_next;     /* current_next_indicator */

  dvbpsi_pat_program_t *    p_first_program;    /* program list */

} dvbpsi_pat_t;


/*****************************************************************************
 * dvbpsi_pat_callback
 *****************************************************************************
 * Callback type definition.
 *****************************************************************************/
typedef void (* dvbpsi_pat_callback)(void* p_cb_data, dvbpsi_pat_t* p_new_pat);


/*****************************************************************************
 * dvbpsi_AttachPAT
 *****************************************************************************
 * Initialize a PAT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPAT(dvbpsi_pat_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachPAT
 *****************************************************************************
 * Close a PAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_DetachPAT(dvbpsi_handle h_dvbpsi);


/*****************************************************************************
 * dvbpsi_InitPAT/dvbpsi_NewPAT
 *****************************************************************************
 * Initialize a pre-allocated/new dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_InitPAT(dvbpsi_pat_t* p_pat, uint16_t i_ts_id, uint8_t i_version,
                    int b_current_next);

#define dvbpsi_NewPAT(p_pat, i_ts_id, i_version, b_current_next)        \
  p_pat = (dvbpsi_pat_t*)malloc(sizeof(dvbpsi_pat_t));                  \
  if(p_pat != NULL)                                                     \
    dvbpsi_InitPAT(p_pat, i_ts_id, i_version, b_current_next);


/*****************************************************************************
 * dvbpsi_EmptyPAT/dvbpsi_DeletePAT
 *****************************************************************************
 * Clean/free a dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_EmptyPAT(dvbpsi_pat_t* p_pat);

#define dvbpsi_DeletePAT(p_pat)                                         \
  dvbpsi_EmptyPAT(p_pat);                                               \
  free(p_pat);


/*****************************************************************************
 * dvbpsi_PATAddProgram
 *****************************************************************************
 * Add a program at the end of the PAT.
 *****************************************************************************/
dvbpsi_pat_program_t* dvbpsi_PATAddProgram(dvbpsi_pat_t* p_pat,
                                           uint16_t i_number, uint16_t i_pid);


/*****************************************************************************
 * dvbpsi_GenPATSections
 *****************************************************************************
 * Generate PAT sections based on the dvbpsi_pat_t structure. The third
 * argument is used to limit the number of program in each section (max: 253).
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenPATSections(dvbpsi_pat_t* p_pat,
                                            int i_max_pps);


#else
#error "Multiple inclusions of pat.h"
#endif

