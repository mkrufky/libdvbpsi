/*
Copyright (C) 2006  Adam Charrett

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

eit.h

Decode PSIP Event Information Table.

*/
#ifndef _ATSC_EIT_H
#define _ATSC_EIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_eit_event_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_eit_event_s
 * \brief EIT Event structure.
 *
 * This structure is used to store decoded event information.
 */
/*!
 * \typedef struct dvbpsi_atsc_eit_event_s dvbpsi_atsc_eit_event_t
 * \brief dvbpsi_atsc_eit_event_t type definition.
 */
typedef struct dvbpsi_atsc_eit_event_s
{
    uint16_t   i_event_id;      /*!< Event ID */
    uint32_t   i_start_time;    /*!< Start time in GPS seconds */
    uint8_t    i_etm_location;  /*!< Extended Text Message location. */
    uint32_t   i_length_seconds;/*!< Length of program in seconds. */
    uint8_t    i_title_length;  /*!< Length of the title in bytes */
    uint8_t    i_title[256];    /*!< Title in multiple string structure format. */

    dvbpsi_descriptor_t *p_first_descriptor; /*!< First descriptor structure. */

    struct dvbpsi_atsc_eit_event_s   *p_next;/*!< Next event information structure. */

} dvbpsi_atsc_eit_event_t;

/*****************************************************************************
 * dvbpsi_atsc_eit_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_eit_s
 * \brief EIT structure.
 *
 * This structure is used to store a decoded EIT.
 */
/*!
 * \typedef struct dvbpsi_atsc_eit_s dvbpsi_atsc_eit_t
 * \brief dvbpsi_atsc_eit_t type definition.
 */
typedef struct dvbpsi_atsc_eit_s
{
    uint8_t                 i_version;          /*!< version_number */
    int                     b_current_next;     /*!< current_next_indicator */
    uint16_t                i_source_id;        /*!< Source id used to match against channels */
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */

    dvbpsi_atsc_eit_event_t   *p_first_event;   /*!< First event information structure. */

} dvbpsi_atsc_eit_t;


/*****************************************************************************
 * dvbpsi_eit_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_eit_callback)(void* p_cb_data,
                                         dvbpsi_atsc_eit_t* p_new_eit)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_eit_callback)(void* p_cb_data, dvbpsi_atsc_eit_t* p_new_eit);


/*****************************************************************************
 * dvbpsi_atsc_AttachEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_AttachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
            dvbpsi_atsc_eit_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a EIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCB.
 * \param i_extension Table ID extension, here TS ID.
 * \param pf_callback function to call back on new EIT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
int dvbpsi_atsc_AttachEIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_atsc_eit_callback pf_callback, void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 *
 * \brief Destroy a EIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCB.
 * \param i_extension Table ID extension, here TS ID.
 * \return nothing.
 */
void dvbpsi_atsc_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_atsc_InitEIT/dvbpsi_atsc_NewEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitEIT(dvbpsi_atsc_eit_t* p_eit, uint8_t i_version,
        int b_current_next, uint8_t i_protocol, uint16_t i_source_id)
 * \brief Initialize a user-allocated dvbpsi_atsc_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param i_source_id Source id.
 * \return nothing.
 */
void dvbpsi_atsc_InitEIT(dvbpsi_atsc_eit_t* p_eit,uint8_t i_version, int b_current_next,
                       uint8_t i_protocol, uint16_t i_source_id);

/*!
 * \def dvbpsi_atsc_NewEIT(p_eit, i_network_id, i_version, b_current_next,
          i_protocol i_source_id)
 * \brief Allocate and initialize a new dvbpsi_eit_t structure. Use ObjectRefDec to delete it.
 * \param p_eit pointer to the EIT structure
 * \param i_network_id network id
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param b_cable_eit Whether this is CEIT or a TEIT.
 * \return nothing.
 */
#define dvbpsi_atsc_NewEIT(p_eit, i_version, b_current_next, i_protocol, \
            i_source_id)                                                 \
do {                                                                     \
  p_eit = (dvbpsi_atsc_eit_t*)malloc(sizeof(dvbpsi_atsc_eit_t));         \
  if(p_eit != NULL)                                                      \
    dvbpsi_atsc_InitEIT(p_eit, i_version, b_current_next, i_protocol,    \
        i_source_id);                                                    \
} while(0);


/*****************************************************************************
 * dvbpsi_atsc_EmptyEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyEIT(dvbpsi_atsc_eit_t* p_eit)
 * \brief Clean a dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyEIT(dvbpsi_atsc_eit_t *p_eit);

/*!
 * \def dvbpsi_atsc_DeleteEIT(p_eit)
 * \brief Clean and free a dvbpsi_eit_t structure.
 * \param p_vct pointer to the EIT structure
 * \return nothing.
 */
#define dvbpsi_atsc_DeleteEIT(p_eit)                                     \
do {                                                                     \
  dvbpsi_atsc_EmptyEIT(p_eit);                                           \
  free(p_eit);                                                           \
} while(0);

#ifdef __cplusplus
};
#endif

#endif

