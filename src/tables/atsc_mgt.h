/*
Copyright (C) 2006  Adam Charrett
Copyright (C) 2011  Michael Krufky

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

mgt.h

Decode PSIP Master Guide Table.

*/
#ifndef _ATSC_MGT_H
#define _ATSC_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_mgt_table_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_mgt_table_s
 * \brief MGT table structure.
 *
 * This structure is used to store a decoded MGT table information.
 */
/*!
 * \typedef struct dvbpsi_atsc_mgt_table_s dvbpsi_atsc_mgt_table_t
 * \brief dvbpsi_atsc_mgt_table_t type definition.
 */
typedef struct dvbpsi_atsc_mgt_table_s
{
    uint16_t                    i_table_type;       /*!< type of table */
    uint16_t                    i_table_type_pid;   /*!< PID of table */
    uint8_t                     i_table_type_version; /*!< version of table */
    uint32_t                    i_number_bytes;     /*!< bytes used for table */

    dvbpsi_descriptor_t        *p_first_descriptor; /*!< First descriptor. */

    struct dvbpsi_atsc_mgt_table_s *p_next;         /*!< next element of the list */
} dvbpsi_atsc_mgt_table_t;

/*****************************************************************************
 * dvbpsi_atsc_mgt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_mgt_s
 * \brief MGT structure.
 *
 * This structure is used to store a decoded MGT.
 */
/*!
 * \typedef struct dvbpsi_atsc_mgt_s dvbpsi_atsc_mgt_t
 * \brief dvbpsi_atsc_mgt_t type definition.
 */
typedef struct dvbpsi_atsc_mgt_s
{
    uint8_t                 i_version;          /*!< version_number */
    int                     b_current_next;     /*!< current_next_indicator */
    uint16_t                i_table_id_ext;     /*!< 0x0000 */
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */

    dvbpsi_atsc_mgt_table_t   *p_first_table;   /*!< First table information structure. */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_mgt_t;

/*****************************************************************************
 * dvbpsi_mgt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_mgt_callback)(void* p_cb_data,
                                         dvbpsi_atsc_mgt_t* p_new_mgt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_mgt_callback)(void* p_cb_data, dvbpsi_atsc_mgt_t* p_new_mgt);

/*****************************************************************************
 * dvbpsi_atsc_AttachMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_AttachMGT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
            dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a MGT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \param pf_callback function to call back on new MGT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
int dvbpsi_atsc_AttachMGT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_DetachMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachMGT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 *
 * \brief Destroy a MGT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \return nothing.
 */
void dvbpsi_atsc_DetachMGT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_atsc_InitMGT/dvbpsi_atsc_NewMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt, uint8_t i_version,
        int b_current_next, uint8_t i_protocol)
 * \brief Initialize a user-allocated dvbpsi_atsc_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \param i_version MGT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \param i_table_id_extension 0x0000.
 * \return nothing.
 */
void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt,uint8_t i_version, int b_current_next,
                       uint8_t i_protocol, uint16_t i_table_id_extension);

/*!
 * \def dvbpsi_atsc_NewMGT(p_mgt, i_network_id, i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \param i_network_id network id
 * \param i_version MGT version
 * \param b_current_next current next indicator
 * \param i_protocol PSIP Protocol version.
 * \return nothing.
 */
#define dvbpsi_atsc_NewMGT(p_mgt, i_version, b_current_next, i_protocol, \
            i_table_id_extension)                                        \
do {                                                                     \
  p_mgt = (dvbpsi_atsc_mgt_t*)malloc(sizeof(dvbpsi_atsc_mgt_t));         \
  if(p_mgt != NULL)                                                      \
    dvbpsi_atsc_InitMGT(p_mgt, i_version, b_current_next, i_protocol,    \
        i_table_id_extension);                                           \
} while(0);

/*****************************************************************************
 * dvbpsi_atsc_EmptyMGT/dvbpsi_atsc_DeleteMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t* p_mgt)
 * \brief Clean a dvbpsi_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t *p_mgt);

/*!
 * \def dvbpsi_atsc_DeleteMGT(p_mgt)
 * \brief Clean and free a dvbpsi_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \return nothing.
 */
#define dvbpsi_atsc_DeleteMGT(p_mgt)                                     \
do {                                                                     \
  dvbpsi_atsc_EmptyMGT(p_mgt);                                           \
  free(p_mgt);                                                           \
} while(0);

#ifdef __cplusplus
};
#endif

#endif
