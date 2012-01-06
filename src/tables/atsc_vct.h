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

vct.h

Decode PSIP Virtual Channel Table.

*/
#ifndef _ATSC_VCT_H
#define _ATSC_VCT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_vct_channel_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_vct_channel_s
 * \brief VCT channel structure.
 *
 * This structure is used to store a decoded VCT channel information.
 */
/*!
 * \typedef struct dvbpsi_atsc_vct_channel_s dvbpsi_atsc_vct_channel_t
 * \brief dvbpsi_atsc_vct_channel_t type definition.
 */
typedef struct dvbpsi_atsc_vct_channel_s
{
    uint8_t                     i_short_name[14];/*!< Channel name (7*UTF16-BE)*/
    uint16_t                    i_major_number;  /*!< Channel major number */
    uint16_t                    i_minor_number;  /*!< Channel minor number */

    uint8_t                     i_modulation;    /*!< Modulation mode. */
    uint32_t                    i_carrier_freq;  /*!< Carrier center frequency. */
    uint16_t                    i_channel_tsid;  /*!< Channel Transport stream id. */
    uint16_t                    i_program_number;/*!< Channel MPEG program number. */
    uint8_t                     i_etm_location;  /*!< Extended Text Message location. */
    int                         b_access_controlled; /*!< Whether the channel is scrambled. */
    int                         b_path_select;   /*!< Path selection, only used by CVCT. */
    int                         b_out_of_band;   /*!< Whether the channel is carried on the out-of-band
                                                      physical transmission channel, only used by CVCT. */
    int                         b_hidden;        /*!< Not accessible directly by the user. */
    int                         b_hide_guide;    /*!< Whether the channel should not be displayed in the guide. */

    uint8_t                     i_service_type;  /*!< Channel type. */
    uint16_t                    i_source_id;     /*!< Programming source associated with the channel.*/

    dvbpsi_descriptor_t        *p_first_descriptor; /*!< First descriptor. */

    struct dvbpsi_atsc_vct_channel_s *p_next;         /*!< next element of the list */
} dvbpsi_atsc_vct_channel_t;


/*****************************************************************************
 * dvbpsi_atsc_vct_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_vct_s
 * \brief VCT structure.
 *
 * This structure is used to store a decoded VCT.
 */
/*!
 * \typedef struct dvbpsi_atsc_vct_s dvbpsi_atsc_vct_t
 * \brief dvbpsi_atsc_vct_t type definition.
 */
typedef struct dvbpsi_atsc_vct_s
{
    uint8_t                 i_version;          /*!< version_number */
    int                     b_current_next;     /*!< current_next_indicator */
    uint16_t                i_ts_id;            /*!< transport stream id */
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */
    int                     b_cable_vct;        /*!< 1 if this is a cable VCT, 0 if it is a Terrestrial VCT. */

    dvbpsi_atsc_vct_channel_t   *p_first_channel;    /*!< First channel information structure. */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_vct_t;


/*****************************************************************************
 * dvbpsi_vct_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_vct_callback)(void* p_cb_data,
                                         dvbpsi_atsc_vct_t* p_new_vct)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_vct_callback)(void* p_cb_data, dvbpsi_atsc_vct_t* p_new_vct);


/*****************************************************************************
 * dvbpsi_atsc_AttachVCT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_AttachVCT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
            dvbpsi_atsc_vct_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a VCT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xC8 or 0xC9.
 * \param i_extension Table ID extension, here TS ID.
 * \param pf_callback function to call back on new VCT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
int dvbpsi_atsc_AttachVCT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_atsc_vct_callback pf_callback, void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachVCT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachVCT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 *
 * \brief Destroy a VCT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xC8 or 0xC9.
 * \param i_extension Table ID extension, here TS ID.
 * \return nothing.
 */
void dvbpsi_atsc_DetachVCT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_atsc_InitVCT/dvbpsi_atsc_NewVCT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitVCT(dvbpsi_atsc_vct_t* p_vct, uint8_t i_version,
        int b_current_next, uint8_t i_protocol)
 * \brief Initialize a user-allocated dvbpsi_atsc_vct_t structure.
 * \param p_vct pointer to the VCT structure
 * \param i_version VCT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param i_ts_id Transport Stream id.
 * \param b_cable_vct Whether this is CVCT or a TVCT.
 * \return nothing.
 */
void dvbpsi_atsc_InitVCT(dvbpsi_atsc_vct_t* p_vct,uint8_t i_version, int b_current_next,
                       uint8_t i_protocol, uint16_t i_ts_id, int b_cable_vct);

/*!
 * \def dvbpsi_atsc_NewVCT(p_vct, i_network_id, i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_vct_t structure.
 * \param p_vct pointer to the VCT structure
 * \param i_network_id network id
 * \param i_version VCT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param b_cable_vct Whether this is CVCT or a TVCT.
 * \return nothing.
 */
#define dvbpsi_atsc_NewVCT(p_vct, i_version, b_current_next, i_protocol, \
            i_ts_id, b_cable_vct)                                        \
do {                                                                     \
  p_vct = (dvbpsi_atsc_vct_t*)malloc(sizeof(dvbpsi_atsc_vct_t));         \
  if(p_vct != NULL)                                                      \
    dvbpsi_atsc_InitVCT(p_vct, i_version, b_current_next, i_protocol,    \
        i_ts_id, b_cable_vct);                                           \
} while(0);


/*****************************************************************************
 * dvbpsi_atsc_EmptyVCT/dvbpsi_atsc_DeleteVCT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyVCT(dvbpsi_atsc_vct_t* p_vct)
 * \brief Clean a dvbpsi_vct_t structure.
 * \param p_vct pointer to the VCT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyVCT(dvbpsi_atsc_vct_t *p_vct);

/*!
 * \def dvbpsi_atsc_DeleteVCT(p_vct)
 * \brief Clean and free a dvbpsi_vct_t structure.
 * \param p_vct pointer to the VCT structure
 * \return nothing.
 */
#define dvbpsi_atsc_DeleteVCT(p_vct)                                     \
do {                                                                     \
  dvbpsi_atsc_EmptyVCT(p_vct);                                           \
  free(p_vct);                                                           \
} while(0);

#ifdef __cplusplus
};
#endif

#endif

