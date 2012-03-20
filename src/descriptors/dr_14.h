/*
Copyright (C) 2010  Adam Charrett

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

dr_14.h

Decode Association Tag Descriptor.

*/
#ifndef _DR_14_H
#define _DR_14_H

/*****************************************************************************
 * dvbpsi_association_tag_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_association_tag_dr_s
 * \brief Data Broadcast id Descriptor
 *
 * This structure is used to store a decoded Association Tag descriptor.
 */
/*!
 * \typedef struct dvbpsi_association_tag_dr_s dvbpsi_association_tag_dr_t
 * \brief dvbpsi_association_tag_dr_t type definition.
 */
typedef struct dvbpsi_association_tag_dr_s
{
    uint16_t i_tag;
    uint16_t i_use;
    uint8_t  i_selector_len;
    uint8_t *p_selector;
    uint8_t  i_private_data_len;
    uint8_t *p_private_data;        /*< release allocated memory */
} dvbpsi_association_tag_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeAssociationTagDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_association_tag_dr_t *dvbpsi_DecodeAssociationTagDr(
 *        dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Association Tag descriptor (tag 0x14)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_association_tag_dr_t structure.
 */
dvbpsi_association_tag_dr_t *dvbpsi_DecodeAssociationTagDr(dvbpsi_descriptor_t *p_descriptor);

#endif





