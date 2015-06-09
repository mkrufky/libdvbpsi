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

#include "config.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"

#include "dr_24.h"

/* the minimum size of a valid content labelling descriptor. it consists of
 * metadata_application_format, content_{reference_id_record_flag, time_base_
 * indicator}, and a reserved field. they all sum up to 24 bits. */
#define DR_24_MIN_SIZE 3

static int decode_content_reference_id(dvbpsi_content_labelling_dr_t *p_decoded,
    const uint8_t **pp_data, uint8_t *pi_left)
{
    uint8_t i_left = *pi_left;
    const uint8_t *p_data = *pp_data;

    if(i_left < 1)
        return 1;

    p_decoded->i_content_reference_id_record_length = *p_data;
    i_left--; p_data++;

    /* H.222.0 says that content_reference_id_record_length "shall not be coded
     * with the value '0'". reject such data as invalid. */
    if(i_left < p_decoded->i_content_reference_id_record_length ||
      p_decoded->i_content_reference_id_record_length == 0)
        return 1;

    p_decoded->p_content_reference_id = malloc(
        p_decoded->i_content_reference_id_record_length);
    if(!p_decoded->p_content_reference_id)
        return 1;

    memcpy(p_decoded->p_content_reference_id, p_data,
        p_decoded->i_content_reference_id_record_length);
    i_left -= p_decoded->i_content_reference_id_record_length;
    p_data += p_decoded->i_content_reference_id_record_length;

    *pp_data = p_data;
    *pi_left = i_left;

    return 0;
}

static int decode_content_time_base_indicator(dvbpsi_content_labelling_dr_t
    *p_decoded, const uint8_t **pp_data, uint8_t *pi_left)
{
    const uint8_t *p_data = *pp_data;
    uint8_t i_left = *pi_left;

    if(p_decoded->i_content_time_base_indicator == 1 ||
        p_decoded->i_content_time_base_indicator == 2)
    {
        if(i_left < 10)
            return 1;

        p_decoded->i_content_time_base_value =
            (((uint64_t)(p_data[0] & 0x01)) << 32) |
            ((uint64_t)p_data[1] << 24) |
            ((uint64_t)p_data[2] << 16) |
            ((uint64_t)p_data[3] <<  8) |
            (uint64_t)p_data[4];

        p_decoded->i_metadata_time_base_value =
            (((uint64_t)(p_data[5] & 0x01)) << 32) |
            ((uint64_t)p_data[6] << 24) |
            ((uint64_t)p_data[7] << 16) |
            ((uint64_t)p_data[8] <<  8) |
            (uint64_t)p_data[9];

        p_data += 10; i_left -= 10;
    }

    if(p_decoded->i_content_time_base_indicator == 2)
    {
        if(i_left < 1)
            return 1;

        p_decoded->i_contentId = *p_data & 0x7f;
        p_data++; i_left--;
    }

    if(p_decoded->i_content_time_base_indicator >= 3 &&
        p_decoded->i_content_time_base_indicator <= 7)
    {
        if(i_left < 1)
            return 1;

        p_decoded->i_time_base_association_data_length = *p_data;
        p_data++; i_left--;

        if(i_left < p_decoded->i_time_base_association_data_length)
            return 1;

        if(p_decoded->i_time_base_association_data_length)
        {
            p_decoded->p_time_base_association_data = malloc(
                p_decoded->i_time_base_association_data_length);
            if(!p_decoded->p_time_base_association_data)
                return 1;

            memcpy(p_decoded->p_time_base_association_data, p_data,
                p_decoded->i_time_base_association_data_length);
        }

        p_data += p_decoded->i_time_base_association_data_length;
        i_left -= p_decoded->i_time_base_association_data_length;
    }

    *pp_data = p_data;
    *pi_left = i_left;

    return 0;
}

dvbpsi_content_labelling_dr_t* dvbpsi_DecodeContentLabellingDr(
                                      dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_content_labelling_dr_t *p_decoded;
    const uint8_t *p_data = p_descriptor->p_data;
    uint8_t i_left = p_descriptor->i_length;

    /* check the tag. */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x24))
        return NULL;

    /* don't decode twice. */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* simplest descriptors of this type contain 3 bytes of payload. */
    if (p_descriptor->i_length < DR_24_MIN_SIZE)
        return NULL;

    p_decoded = malloc(sizeof(*p_decoded));
    if (!p_decoded)
        return NULL;

    /* set the pointers to NULL so we can safely free() them in err: */
    p_decoded->p_content_reference_id = NULL;
    p_decoded->p_time_base_association_data = NULL;
    p_decoded->p_private_data = NULL;

    p_decoded->i_metadata_application_format =
        ((uint16_t)p_data[0] << 8) | (uint16_t)p_data[1];
    i_left -= 2; p_data += 2;

    if(p_decoded->i_metadata_application_format == 0xFFFF)
    {
        if(i_left < 4)
            goto err;

        p_decoded->i_metadata_application_format_identifier =
            ((uint32_t)p_data[0] << 24) |
            ((uint32_t)p_data[1] << 16) |
            ((uint32_t)p_data[2] <<  8) |
            (uint32_t)p_data[3];
        i_left -= 4; p_data += 4;
    }

    if(i_left < 1)
        goto err;

    p_decoded->b_content_reference_id_record_flag = (*p_data & 0x80);
    p_decoded->i_content_time_base_indicator = ((*p_data & 0x78) >> 3);
    i_left--; p_data++;

    if(p_decoded->b_content_reference_id_record_flag &&
        decode_content_reference_id(p_decoded, &p_data, &i_left) != 0)
        goto err;

    if(p_decoded->i_content_time_base_indicator &&
        decode_content_time_base_indicator(p_decoded, &p_data, &i_left) != 0)
        goto err;

    p_decoded->i_private_data_len = i_left;
    if(p_decoded->i_private_data_len)
    {
        p_decoded->p_private_data = malloc(p_decoded->i_private_data_len);
        if(!p_decoded->p_private_data)
            goto err;

        memcpy(p_decoded->p_private_data, p_data, p_decoded->i_private_data_len);
    }

    p_descriptor->p_decoded = p_decoded;
    return p_decoded;

err:
    free(p_decoded->p_private_data);
    free(p_decoded->p_time_base_association_data);
    free(p_decoded->p_content_reference_id);
    free(p_decoded);
    return NULL;
}

static unsigned int generate_get_descriptor_size(const
    dvbpsi_content_labelling_dr_t * p_decoded)
{
    unsigned int size = DR_24_MIN_SIZE + p_decoded->i_private_data_len;

    if(p_decoded->i_metadata_application_format == 0xFFFF)
        size += 4;

    if(p_decoded->b_content_reference_id_record_flag)
        size += p_decoded->i_content_reference_id_record_length + 1;

    if(p_decoded->i_content_time_base_indicator == 1 ||
        p_decoded->i_content_time_base_indicator == 2)
        size += 10;

    if(p_decoded->i_content_time_base_indicator == 2)
        size += 1;

    if(p_decoded->i_content_time_base_indicator >= 3 &&
        p_decoded->i_content_time_base_indicator <= 7)
        size += p_decoded->i_time_base_association_data_length + 1;

    return size;
}

static bool generate_check_struct_valid(const
    dvbpsi_content_labelling_dr_t * p_decoded)
{
    /* check if the pointers are valid if relevant sizes are nonzero. also,
     * check if i_content_reference_id_record_length isn't zero if the flag is
     * set, as this is explicitly forbidden by H.222.0. */
    if((p_decoded->b_content_reference_id_record_flag &&
        !p_decoded->i_content_reference_id_record_length) ||
        (p_decoded->i_content_reference_id_record_length &&
        !p_decoded->p_content_reference_id))
        return false;

    if(p_decoded->i_time_base_association_data_length &&
        !p_decoded->p_time_base_association_data)
        return false;

    if(p_decoded->i_private_data_len && !p_decoded->p_private_data)
        return false;

    return true;
}

dvbpsi_descriptor_t * dvbpsi_GenContentLabellingDr(
                                      dvbpsi_content_labelling_dr_t * p_decoded)
{
    unsigned int size;
    dvbpsi_descriptor_t * p_descriptor;
    uint8_t *p_data;

    if(!generate_check_struct_valid(p_decoded))
        return NULL;

    size = generate_get_descriptor_size(p_decoded);
    if(size > 253) /* maximum possible descriptor payload size. */
        return NULL;

    p_descriptor = dvbpsi_NewDescriptor(0x24, size, NULL);
    if (!p_descriptor)
        return NULL;

    p_data = p_descriptor->p_data;

    p_data[0] = (p_decoded->i_metadata_application_format & 0xFF00) >> 8;
    p_data[1] = (p_decoded->i_metadata_application_format & 0x00FF);
    p_data += 2;

    if(p_decoded->i_metadata_application_format == 0xFFFF)
    {
        p_data[0] = (p_decoded->i_metadata_application_format_identifier & 0xFF000000) >> 24;
        p_data[1] = (p_decoded->i_metadata_application_format_identifier & 0x00FF0000) >> 16;
        p_data[2] = (p_decoded->i_metadata_application_format_identifier & 0x0000FF00) >> 8;
        p_data[3] = (p_decoded->i_metadata_application_format_identifier & 0x000000FF);
        p_data += 4;
    }

    *p_data = ((p_decoded->b_content_reference_id_record_flag ? 1 : 0) << 7) |
        ((p_decoded->i_content_time_base_indicator & 0xf) << 6) |
        0x07;
    p_data++;

    if(p_decoded->b_content_reference_id_record_flag)
    {
        *p_data = p_decoded->i_content_reference_id_record_length;
        memcpy(p_data + 1, p_decoded->p_content_reference_id,
            p_decoded->i_content_reference_id_record_length);
        p_data += (p_decoded->i_content_reference_id_record_length + 1);
    }

    if(p_decoded->i_content_time_base_indicator == 1 ||
        p_decoded->i_content_time_base_indicator == 2)
    {
        p_data[0] = 0xfe | ((p_decoded->i_content_time_base_value & 0x100000000) >> 32);
        p_data[1] = (p_decoded->i_content_time_base_value & 0xFF000000) >> 24;
        p_data[2] = (p_decoded->i_content_time_base_value & 0x00FF0000) >> 16;
        p_data[3] = (p_decoded->i_content_time_base_value & 0x0000FF00) >> 8;
        p_data[4] = (p_decoded->i_content_time_base_value & 0x000000FF);

        p_data[5] = 0xfe | ((p_decoded->i_metadata_time_base_value & 0x100000000) >> 32);
        p_data[6] = (p_decoded->i_metadata_time_base_value & 0xFF000000) >> 24;
        p_data[7] = (p_decoded->i_metadata_time_base_value & 0x00FF0000) >> 16;
        p_data[8] = (p_decoded->i_metadata_time_base_value & 0x0000FF00) >> 8;
        p_data[9] = (p_decoded->i_metadata_time_base_value & 0x000000FF);

        p_data += 10;
    }

    if(p_decoded->i_content_time_base_indicator == 2)
    {
        *p_data = 0x80 | (p_decoded->i_contentId & 0x7f);
        p_data++;
    }

    if(p_decoded->i_content_time_base_indicator >= 3 &&
        p_decoded->i_content_time_base_indicator <= 7)
    {
        *p_data = p_decoded->i_time_base_association_data_length;
        memcpy(p_data + 1, p_decoded->p_time_base_association_data,
            p_decoded->i_time_base_association_data_length);
        p_data += (p_decoded->i_time_base_association_data_length + 1);
    }

    memcpy(p_data, p_decoded->p_private_data, p_decoded->i_private_data_len);
    return p_descriptor;
}
