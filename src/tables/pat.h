/*****************************************************************************
 * pat.h: PAT structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: pat.h,v 1.3 2002/03/25 21:00:50 bozo Exp $
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

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_pat_program_t
 *****************************************************************************/
/*!This structure is used to store a decoded PAT program.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 *****************************************************************************/
typedef struct dvbpsi_pat_program_s
{
  uint16_t                      i_number;               /*!< program_number */
  uint16_t                      i_pid;                  /*!< PID of NIT/PMT */

  struct dvbpsi_pat_program_s * p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_pat_program_t;


/*****************************************************************************
 * dvbpsi_pat_t
 *****************************************************************************/
/*!This structure is used to store a decoded PAT.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 *****************************************************************************/
typedef struct dvbpsi_pat_s
{
  uint16_t                  i_ts_id;            /*!< transport_stream_id */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */

  dvbpsi_pat_program_t *    p_first_program;    /*!< program list */

} dvbpsi_pat_t;


/*****************************************************************************
 * dvbpsi_pat_callback
 *****************************************************************************/
/*!Callback type definition.
 *****************************************************************************/
typedef void (* dvbpsi_pat_callback)(void* p_cb_data, dvbpsi_pat_t* p_new_pat);


/*****************************************************************************
 * dvbpsi_AttachPAT
 *****************************************************************************/
/*!Initialize a PAT decoder and return a handle on it.
 * \param pf_callback function to call back on new PAT
 * \param p_cb_data private data given in argument to the callback
 * \return a pointer to the decoder for future calls.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPAT(dvbpsi_pat_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachPAT
 *****************************************************************************/
/*!Close a PAT decoder. The handle isn't valid any more.
 * \param h_dvbpsi handle to the decoder
 * \return nothing.
 *****************************************************************************/
void dvbpsi_DetachPAT(dvbpsi_handle h_dvbpsi);


/*****************************************************************************
 * dvbpsi_InitPAT/dvbpsi_NewPAT
 *****************************************************************************/
/*!Initialize a user-allocated dvbpsi_pat_t structure.
 * \param p_pat PAT structure
 * \param i_ts_id transport stream ID
 * \param i_version PAT version
 * \param b_current_next current next indicator
 * \return nothing.
 *****************************************************************************/
void dvbpsi_InitPAT(dvbpsi_pat_t* p_pat, uint16_t i_ts_id, uint8_t i_version,
                    int b_current_next);

/*!Allocate and initialize a new dvbpsi_pat_t structure.
 * \param p_pat PAT structure
 * \param i_ts_id transport stream ID
 * \param i_version PAT version
 * \param b_current_next current next indicator
 * \return nothing.
 *****************************************************************************/
#define dvbpsi_NewPAT(p_pat, i_ts_id, i_version, b_current_next)        \
  p_pat = (dvbpsi_pat_t*)malloc(sizeof(dvbpsi_pat_t));                  \
  if(p_pat != NULL)                                                     \
    dvbpsi_InitPAT(p_pat, i_ts_id, i_version, b_current_next);


/*****************************************************************************
 * dvbpsi_EmptyPAT/dvbpsi_DeletePAT
 *****************************************************************************/
/*!Clean a dvbpsi_pat_t structure.
 * \param p_pat PAT structure
 * \return nothing.
 *****************************************************************************/
void dvbpsi_EmptyPAT(dvbpsi_pat_t* p_pat);

/*!Clean and free a dvbpsi_pat_t structure.
 * \param p_pat PAT structure
 * \return nothing.
 *****************************************************************************/
#define dvbpsi_DeletePAT(p_pat)                                         \
  dvbpsi_EmptyPAT(p_pat);                                               \
  free(p_pat);


/*****************************************************************************
 * dvbpsi_PATAddProgram
 *****************************************************************************/
/*!Add a program at the end of the PAT.
 * \param p_pat PAT structure
 * \param i_number program number
 * \param i_pid PID of the NIT/PMT
 * \return a pointer to the added program.
 *****************************************************************************/
dvbpsi_pat_program_t* dvbpsi_PATAddProgram(dvbpsi_pat_t* p_pat,
                                           uint16_t i_number, uint16_t i_pid);

/*****************************************************************************
 * dvbpsi_GenPATSections
 *****************************************************************************/
/*!Generate PAT sections based on the dvbpsi_pat_t structure.
 * \param p_pat PAT structure
 * \param i_max_pps limitation of the number of program in each section
 * (max: 253).
 * \return a pointer to the list of generated PSI sections.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenPATSections(dvbpsi_pat_t* p_pat,
                                            int i_max_pps);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of pat.h"
#endif

