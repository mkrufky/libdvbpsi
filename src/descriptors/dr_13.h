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

dr_13.h

Decode Carousel id Descriptor.

*/
#ifndef _DR_13_H
#define _DR_13_H

/*****************************************************************************
 * dvbpsi_carousel_id_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_carousel_id_dr_s
 * \brief Data Broadcast id Descriptor
 *
 * This structure is used to store a decoded Carsouel ID descriptor.
 */
/*!
 * \typedef struct dvbpsi_carousel_id_dr_s dvbpsi_carousel_id_dr_t
 * \brief dvbpsi_carousel_id_dr_t type definition.
 */
typedef struct dvbpsi_carousel_id_dr_s
{
    uint32_t i_carousel_id;
    uint8_t i_private_data_len;
    uint8_t s_private_data[0];
}dvbpsi_carousel_id_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeCarouselIdDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_carousel_id_dr_t *dvbpsi_DecodeCarouselIdDr(
 *        dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Carousel id descriptor (tag 0x13)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_carousel_id_dr_t structure.
 */
dvbpsi_carousel_id_dr_t *dvbpsi_DecodeCarouselIdDr(dvbpsi_descriptor_t *p_descriptor);

#endif




