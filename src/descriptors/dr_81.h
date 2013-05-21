/*
Copyright (C) 2013  Michael Krufky

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

dr_81.h

Decode AC-3 Audio Descriptor.

*/

/*!
 * \file dr_81.h
 * \author Michael Krufky
 * \brief Decode AC-3 Audio Descriptor.
 */

#ifndef _DR_81_H
#define _DR_81_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_ac3_audio_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_ac3_audio_dr_s
 * \brief AC-3 Audio Descriptor
 *
 * This structure is used to store a decoded AC-3 Audio descriptor.
 */
/*!
 * \typedef struct dvbpsi_ac3_audio_dr_s dvbpsi_ac3_audio_dr_t
 * \brief dvbpsi_ac3_audio_dr_t type definition.
 */
typedef struct dvbpsi_ac3_audio_dr_s
{
    uint8_t  i_sample_rate_code; // 3bits
    uint8_t  i_bsid;             // 5bits
    uint8_t  i_bit_rate_code;    // 6bits
    uint8_t  i_surround_mode;    // 2bits
    uint8_t  i_bsmod;            // 3bits
    uint8_t  i_num_channels;     // 4bits
    int      b_full_svc;
    uint8_t  i_lang_code;        // 8bits
    uint8_t  i_lang_code2;       // 8bits
    uint8_t  i_mainid;           // 3bits
    uint8_t  i_priority;         // 2bits
    uint8_t  i_asvcflags;        // 8bits
    uint8_t  i_textlen;          // 7bits
    int      b_text_code;
    unsigned char text[128];
    int      b_language_flag;
    int      b_language_flag_2;
    uint8_t  language[3];
    uint8_t  language_2[3];
}dvbpsi_ac3_audio_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeAc3AudioDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_ac3_audio_dr_t dvbpsi_DecodeAc3AudioDr(dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a AC-3 Audio descriptor (tag 0x81)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_ac3_audio_dr_t structure.
 */
dvbpsi_ac3_audio_dr_t *dvbpsi_DecodeAc3AudioDr(dvbpsi_descriptor_t *p_descriptor);

#ifdef __cplusplus
};
#endif

#endif

