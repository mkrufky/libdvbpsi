/*****************************************************************************
 * pmt.h: PMT structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: pmt.h,v 1.4 2002/03/25 21:00:50 bozo Exp $
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

#ifndef _DVBPSI_PMT_H_
#define _DVBPSI_PMT_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_pmt_es_t
 *****************************************************************************/
/*!This structure is used to store a decoded ES description.
 * (ISO/IEC 13818-1 section 2.4.4.8).
 *****************************************************************************/
typedef struct dvbpsi_pmt_es_s
{
  uint8_t                       i_type;                 /*!< stream_type */
  uint16_t                      i_pid;                  /*!< elementary_PID */

  dvbpsi_descriptor_t *         p_first_descriptor;     /*!< descriptor list */

  struct dvbpsi_pmt_es_s *      p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_pmt_es_t;


/*****************************************************************************
 * dvbpsi_pmt_t
 *****************************************************************************/
/*!This structure is used to store a decoded PMT.
 * (ISO/IEC 13818-1 section 2.4.4.8).
 *****************************************************************************/
typedef struct dvbpsi_pmt_s
{
  uint16_t                  i_program_number;   /*!< program_number */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */

  uint16_t                  i_pcr_pid;          /*!< PCR_PID */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

  dvbpsi_pmt_es_t *         p_first_es;         /*!< ES list */

} dvbpsi_pmt_t;


/*****************************************************************************
 * dvbpsi_pmt_callback
 *****************************************************************************/
/*!Callback type definition.
 *****************************************************************************/
typedef void (* dvbpsi_pmt_callback)(void* p_cb_data, dvbpsi_pmt_t* p_new_pmt);


/*****************************************************************************
 * dvbpsi_AttachPMT
 *****************************************************************************/
/*!Initialize a PMT decoder and return a handle on it.
 * \param i_program_number program number
 * \param pf_callback function to call back on new PMT
 * \param p_cb_data private data given in argument to the callback
 * \return a pointer to the decoder for future calls.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPMT(uint16_t i_program_number,
                               dvbpsi_pmt_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachPMT
 *****************************************************************************/
/*!Close a PMT decoder. The handle isn't valid any more.
 * \param h_dvbpsi handle to the decoder
 * \return nothing.
 *****************************************************************************/
void dvbpsi_DetachPMT(dvbpsi_handle h_dvbpsi);


/*****************************************************************************
 * dvbpsi_InitPMT/dvbpsi_NewPMT
 *****************************************************************************/
/*!Initialize a user-allocated dvbpsi_pmt_t structure.
 * \param p_pmt PMT structure
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 *****************************************************************************/
void dvbpsi_InitPMT(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, int b_current_next, uint16_t i_pcr_pid);

/*!Allocate and initialize a new dvbpsi_pmt_t structure.
 * \param p_pmt PMT structure
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 *****************************************************************************/
#define dvbpsi_NewPMT(p_pmt, i_program_number,                          \
                      i_version, b_current_next, i_pcr_pid)             \
  p_pmt = (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));                  \
  if(p_pmt != NULL)                                                     \
    dvbpsi_InitPMT(p_pmt, i_program_number, i_version, b_current_next,  \
                   i_pcr_pid);


/*****************************************************************************
 * dvbpsi_EmptyPMT/dvbpsi_DeletePMT
 *****************************************************************************/
/*!Clean a dvbpsi_pmt_t structure.
 * \param p_pmt PMT structure
 * \return nothing.
 *****************************************************************************/
void dvbpsi_EmptyPMT(dvbpsi_pmt_t* p_pmt);

/*!Clean and free a dvbpsi_pmt_t structure.
 * \param p_pmt PMT structure
 * \return nothing.
 *****************************************************************************/
#define dvbpsi_DeletePMT(p_pmt)                                         \
  dvbpsi_EmptyPMT(p_pmt);                                               \
  free(p_pmt);


/*****************************************************************************
 * dvbpsi_PMTAddDescriptor
 *****************************************************************************/
/*!Add a descriptor in the PMT.
 * \param p_pmt PMT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTAddDescriptor(dvbpsi_pmt_t* p_pmt,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_PMTAddES
 *****************************************************************************/
/*!Add an ES in the PMT.
 * \param p_pmt PMT structure
 * \param i_type type of ES
 * \param i_pid PID of the ES
 * \return a pointer to the added ES.
 *****************************************************************************/
dvbpsi_pmt_es_t* dvbpsi_PMTAddES(dvbpsi_pmt_t* p_pmt,
                                 uint8_t i_type, uint16_t i_pid);


/*****************************************************************************
 * dvbpsi_PMTESAddDescriptor
 *****************************************************************************/
/*!Add a descriptor in the PMT ES.
 * \param p_es ES structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTESAddDescriptor(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_GenPMTSections
 *****************************************************************************/
/*!Generate PMT sections based on the dvbpsi_pmt_t structure.
 * \param p_pmt PMT structure
 * \return a pointer to the list of generated PSI sections.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenPMTSections(dvbpsi_pmt_t* p_pmt);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of pmt.h"
#endif

