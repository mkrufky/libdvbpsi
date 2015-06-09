/*
Copyright (C) 2015 Daniel Kamil Kozar

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
*/

/*!
 * \file <dr_24.h>
 * \author Daniel Kamil Kozar <dkk089@gmail.com>
 * \brief Application interface for the content labelling descriptor decoder and
 * generator.
 *
 * Application interface for the content labelling descriptor decoder and
 * generator. This descriptor's definition can be found in ISO/IEC 13818-1:2015
 * section 2.6.56.
 */

#ifndef _DVBPSI_DR_24_H_
#define _DVBPSI_DR_24_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \struct dvbpsi_content_labelling_dr_s
 * \brief Content labelling descriptor structure.
 *
 * This structure is used to store a decoded content labelling descriptor.
 * (ISO/IEC 13818-1 section 2.6.56).
 */

/*!
 * \typedef struct dvbpsi_content_labelling_dr_s dvbpsi_content_labelling_dr_t
 * \brief dvbpsi_content_labelling_dr_t type definition.
 */
typedef struct dvbpsi_content_labelling_dr_s
{
    uint16_t i_metadata_application_format; /*!< metadata_application_format */

    /*! metadata_application_format_identifier
     * \warning Valid only if \c i_metadata_application_format is \c 0xFFFF . */
    uint32_t i_metadata_application_format_identifier;

    /*! content_reference_id_record_flag */
    bool b_content_reference_id_record_flag;
    uint8_t i_content_time_base_indicator; /*!< content_time_base_indicator */

    /*! content_reference_id_record_length
     * \warning Valid only if \c b_content_reference_id_record_flag is true. */
    uint8_t i_content_reference_id_record_length;

    /*! content_reference_id_byte
     * An array of \c i_content_reference_id_record_length bytes. Memory is
     * allocated by the decoder when decoding, and by the caller when generating.
     * \warning Valid only if \c b_content_reference_id_record_flag is true. */
    uint8_t *p_content_reference_id;

    /*! content_time_base_value
     * \warning Valid only if \c i_content_time_base_indicator is 1 or 2. */
    uint64_t i_content_time_base_value;

    /*! metadata_time_base_value
     * \warning Valid only if \c i_content_time_base_indicator is 1 or 2. */
    uint64_t i_metadata_time_base_value;

    /*! contentId
     * \warning Valid only if \c i_content_time_base_indicator is 2. */
    uint8_t i_contentId;

    /*! time_base_association_data_length
     * \warning Valid only if \c i_content_time_base_indicator is 3, 4, 5, 6,
     * or 7. */
    uint8_t i_time_base_association_data_length;

    /*! time_base_association_data
     * An array of \c i_time_base_association_data_length bytes. Memory is
     * allocated by the decoder when decoding, and by the caller when generating.
     * \note This field is marked as reserved, but access to it is nevertheless
     * provided if applications want to use it.
     * \warning Valid only if \c i_content_time_base_indicator is 3, 4, 5, 6,
     * or 7. */
    uint8_t *p_time_base_association_data;

    /*! private_data_len */
    uint8_t i_private_data_len;

    /*! private_data
     * \note An array of \c i_private_data_len bytes at the end of the
     * descriptor. Memory is allocated by the decoder when decoding, and by the
     * caller when generating. */
    uint8_t *p_private_data;
} dvbpsi_content_labelling_dr_t;

/*!
 * \brief Content labelling descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return A pointer to a new content labelling descriptor structure which
 * contains the decoded data.
 */
dvbpsi_content_labelling_dr_t* dvbpsi_DecodeContentLabellingDr(
                                      dvbpsi_descriptor_t * p_descriptor);

/*!
 * \brief Content labelling descriptor generator.
 * \param p_decoded pointer to a decoded content labelling descriptor structure.
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenContentLabellingDr(
                                      dvbpsi_content_labelling_dr_t * p_decoded);

#ifdef __cplusplus
}
#endif

#else
#error "Multiple inclusions of dr_24.h"
#endif
