/*
Copyright (C) 2006  Adam Charrett

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

ett.h

Decode PSIP Extended Text Table.

*/
#ifndef _ATSC_ETT_H
#define _ATSC_ETT_H 

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_ett_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_ett_s
 * \brief ETT structure.
 *
 * This structure is used to store a decoded ETT.
 */
/*!
 * \typedef struct dvbpsi_atsc_ett_s dvbpsi_atsc_ett_t
 * \brief dvbpsi_atsc_ett_t type definition.
 */
typedef struct dvbpsi_atsc_ett_s
{
    uint8_t                 i_version;          /*!< version_number */
    int                     b_current_next;     /*!< current_next_indicator */
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */
    uint16_t                i_ett_table_id;     /*!< ETT Table ID extension, normally 0x0000 */
    uint32_t                i_etm_id;           /*!< ETM Identifier, made up of source id and event id (or 0 for channel ETT) */
    uint16_t                i_etm_length;
    uint8_t*                p_etm;
} dvbpsi_atsc_ett_t;


/*****************************************************************************
 * dvbpsi_atsc_ett_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_ett_callback)(void* p_cb_data,
                                         dvbpsi_atsc_ett_t* p_new_ett)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_ett_callback)(void* p_cb_data, dvbpsi_atsc_ett_t* p_new_ett);


/*****************************************************************************
 * dvbpsi_atsc_AttachETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_AttachETT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
            dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a ETT decoder.
 * \param i_extension Table ID extension, normally 0x0000.
 * \param pf_callback function to call back on new ETT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
int dvbpsi_atsc_AttachETT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data);


/*****************************************************************************
 * dvbpsi_atsc_DetachETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachETT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 *
 * \brief Destroy a ETT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table ID extension, normally 0x0000.
 * \return nothing.
 */
void dvbpsi_atsc_DetachETT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_atsc_InitETT/dvbpsi_atsc_NewETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t* p_ett, uint8_t i_version, 
        int b_current_next, uint8_t i_protocol)
 * \brief Initialize a user-allocated dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \param i_version version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param i_ett_table_id Table ID (Normally 0x0000)
 * \param i_etm_id ETM Identifier.
 * \return nothing.
 */
void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett, uint8_t i_version, int b_current_next, uint8_t i_protocol, uint16_t i_ett_table_id, uint32_t i_etm_id);

/*!
 * \def dvbpsi_NewETT(p_ett, i_network_id, i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_atsc_ett_t structure. Use ObjectRefDec to delete it.
 * \param p_ett pointer to the ETT structure
 * \param i_protocol PSIP Protocol version.
 * \param i_ett_table_id Table ID (Normally 0x0000)
 * \param i_etm_id ETM Identifier.
 * \return nothing.
 */
#define dvbpsi_atsc_NewETT(p_ett, i_version, b_current_next, i_protocol, i_ett_table_id, i_etm_id)  \
do {                                                                     \
  p_ett = (dvbpsi_atsc_ett_t*)malloc(sizeof(dvbpsi_atsc_ett_t));         \
  if(p_ett != NULL)                                                      \
    dvbpsi_atsc_InitETT(p_ett, i_version, b_current_next, i_protocol, i_ett_table_id, i_etm_id);   \
} while(0);


/*****************************************************************************
 * dvbpsi_atsc_EmptyETT/dvbpsi_atsc_DeleteETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t* p_ett)
 * \brief Clean a dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t *p_ett);

/*!
 * \def dvbpsi_atsc_DeleteETT(p_ett)
 * \brief Clean and free a dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \return nothing.
 */
#define dvbpsi_atsc_DeleteETT(p_ett)					\
  do {									\
    dvbpsi_atsc_EmptyETT(p_ett);					\
    free(p_ett);							\
  } while(0);

#ifdef __cplusplus
};
#endif

#endif
