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

stt.h

Decode PSIP System Time Table.

*/
#ifndef _ATSC_STT_H
#define _ATSC_STT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_stt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_stt_s
 * \brief STT structure.
 *
 * This structure is used to store a decoded STT.
 */
/*!
 * \typedef struct dvbpsi_atsc_stt_s dvbpsi_atsc_stt_t
 * \brief dvbpsi_atsc_stt_t type definition.
 */
typedef struct dvbpsi_atsc_stt_s
{
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */

    uint32_t                i_system_time;      /*!< GPS seconds since 1 January 1980 00:00:00 UTC. */
    uint8_t                 i_gps_utc_offset;   /*!< Seconds offset between GPS and UTC time. */
    uint16_t                i_daylight_savings; /*!< Daylight savings control bytes. */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_stt_t;


/*****************************************************************************
 * dvbpsi_atsc_stt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_stt_callback)(void* p_cb_data,
                                         dvbpsi_atsc_stt_t* p_new_stt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_stt_callback)(void* p_cb_data, dvbpsi_atsc_stt_t* p_new_stt);


/*****************************************************************************
 * dvbpsi_atsc_AttachSTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_AttachSTT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
            dvbpsi_atsc_stt_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a STT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCD.
 * \param pf_callback function to call back on new STT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
int dvbpsi_atsc_AttachSTT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          dvbpsi_atsc_stt_callback pf_callback, void* p_cb_data);


/*****************************************************************************
 * dvbpsi_atsc_DetachSTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachSTT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 *
 * \brief Destroy a STT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table extension, ignored as this should always be 0.
 *                    (Required to match prototype for demux)
 * \return nothing.
 */
void dvbpsi_atsc_DetachSTT(dvbpsi_demux_t * p_demux, uint8_t i_table_id, uint16_t i_externsion);


/*****************************************************************************
 * dvbpsi_atsc_InitSTT/dvbpsi_atsc_NewSTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitSTT(dvbpsi_atsc_stt_t* p_stt, uint8_t i_version,
        int b_current_next, uint8_t i_protocol)
 * \brief Initialize a user-allocated dvbpsi_atsc_stt_t structure.
 * \param p_stt pointer to the STT structure
 * \param i_protocol PSIP Protocol version.
 * \return nothing.
 */
void dvbpsi_atsc_InitSTT(dvbpsi_atsc_stt_t *p_stt,uint8_t i_protocol);

/*!
 * \def dvbpsi_NewSTT(p_stt, i_network_id, i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_atsc_stt_t structure. Use ObjectRefDec to delete it.
 * \param p_stt pointer to the STT structure
 * \param i_protocol PSIP Protocol version.
 * \return nothing.
 */
#define dvbpsi_atsc_NewSTT(p_stt, i_protocol)                            \
do {                                                                     \
  p_stt = (dvbpsi_atsc_stt_t*)malloc(sizeof(dvbpsi_atsc_stt_t));         \
  if(p_stt != NULL)                                                      \
    dvbpsi_atsc_InitSTT(p_stt, i_protocol);                              \
} while(0);


/*****************************************************************************
 * dvbpsi_atsc_EmptySTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptySTT(dvbpsi_atsc_stt_t* p_stt)
 * \brief Clean a dvbpsi_atsc_stt_t structure.
 * \param p_stt pointer to the STT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptySTT(dvbpsi_atsc_stt_t *p_stt);

/*!
 * \def dvbpsi_atsc_DeleteSTT(p_vct)
 * \brief Clean and free a dvbpsi_stt_t structure.
 * \param p_vct pointer to the STT structure
 * \return nothing.
 */
#define dvbpsi_atsc_DeleteSTT(p_stt)                                     \
do {                                                                     \
  dvbpsi_atsc_EmptySTT(p_stt);                                           \
  free(p_stt);                                                           \
} while(0);

#ifdef __cplusplus
};
#endif

#endif

