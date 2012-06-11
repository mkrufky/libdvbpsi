/*
Copyright (C) 2006-2012  Adam Charrett

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

stt.c

Decode PSIP System Time Table.

*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "../descriptor.h"
#include "../demux.h"

#include "atsc_stt.h"

typedef struct dvbpsi_atsc_stt_decoder_s
{
     DVBPSI_DECODER_COMMON

     dvbpsi_atsc_stt_callback      pf_stt_callback;
     void *                        p_cb_data;

} dvbpsi_atsc_stt_decoder_t;

dvbpsi_descriptor_t *dvbpsi_atsc_STTAddDescriptor(
                                               dvbpsi_atsc_stt_t *p_stt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static void dvbpsi_atsc_GatherSTTSections(dvbpsi_t* p_dvbpsi,
                      dvbpsi_decoder_t *p_decoder, dvbpsi_psi_section_t* p_section);

static void dvbpsi_atsc_DecodeSTTSections(dvbpsi_atsc_stt_t* p_stt,
                                   dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachSTT
 *****************************************************************************
 * Initialize a STT subtable decoder.
 *****************************************************************************/
bool dvbpsi_atsc_AttachSTT(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_stt_callback pf_stt_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_private;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, 0))
    {
        dvbpsi_error(p_dvbpsi, "ATSC STT decoder",
                               "Already a decoder for (table_id == 0x%02x)",
                                i_table_id);
        return false;
    }

    dvbpsi_atsc_stt_decoder_t*  p_stt_decoder;
    p_stt_decoder = (dvbpsi_atsc_stt_decoder_t*)malloc(sizeof(dvbpsi_atsc_stt_decoder_t));
    if (p_stt_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachSTT,
                                         dvbpsi_atsc_GatherSTTSections, DVBPSI_DECODER(p_stt_decoder));
    if (p_subdec == NULL)
    {
        free(p_stt_decoder);
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* STT decoder information */
    p_stt_decoder->pf_stt_callback = pf_stt_callback;
    p_stt_decoder->p_cb_data = p_cb_data;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachSTT
 *****************************************************************************
 * Close a STT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachSTT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_private;

    i_extension = 0;
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC STT Decoder",
                             "No such STT decoder (table_id == 0x%02x,"
                             "extension == 0x00)",
                            i_table_id);
        return;
    }

    dvbpsi_atsc_stt_decoder_t* p_stt_decoder;
    p_stt_decoder = (dvbpsi_atsc_stt_decoder_t*)p_subdec->p_decoder;
    if(!p_stt_decoder)
        return;

    free(p_subdec->p_decoder);
    p_subdec->p_decoder = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitSTT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_stt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitSTT(dvbpsi_atsc_stt_t *p_stt, uint8_t i_version)
{
    assert(p_stt);

    p_stt->i_version = i_version;
    p_stt->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_NewSTT
 *****************************************************************************
 * Allocate and initialize a dvbpsi_atsc_stt_t structure.
 *****************************************************************************/
dvbpsi_atsc_stt_t *dvbpsi_atsc_NewSTT(uint8_t i_version, bool b_current_next)
{
    dvbpsi_atsc_stt_t *p_stt;
    p_stt = (dvbpsi_atsc_stt_t*)malloc(sizeof(dvbpsi_atsc_stt_t));
    if (p_stt != NULL)
        dvbpsi_atsc_InitSTT(p_stt, i_version);
    return p_stt;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptySTT
 *****************************************************************************
 * Clean a dvbpsi_atsc_stt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptySTT(dvbpsi_atsc_stt_t* p_stt)
{
  dvbpsi_DeleteDescriptors(p_stt->p_first_descriptor);
  p_stt->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_DeleteSTT
 *****************************************************************************
 * Empty and Delere a dvbpsi_atsc_stt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_DeleteSTT(dvbpsi_atsc_stt_t *p_stt)
{
    if (p_stt)
        dvbpsi_atsc_EmptySTT(p_stt);
    free(p_stt);
    p_stt = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_STTAddDescriptor
 *****************************************************************************
 * Add a descriptor to the STT table.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_STTAddDescriptor( dvbpsi_atsc_stt_t *p_stt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
            = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if(p_descriptor)
    {
        if(p_stt->p_first_descriptor == NULL)
        {
            p_stt->p_first_descriptor = p_descriptor;
        }
        else
        {
            dvbpsi_descriptor_t * p_last_descriptor = p_stt->p_first_descriptor;
            while(p_last_descriptor->p_next != NULL)
                p_last_descriptor = p_last_descriptor->p_next;
            p_last_descriptor->p_next = p_descriptor;
        }
    }

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherSTTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
static void dvbpsi_atsc_GatherSTTSections(dvbpsi_t *p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t * p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0xCD, "ATSC STT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_atsc_stt_decoder_t *p_stt_decoder = (dvbpsi_atsc_stt_decoder_t*)p_decoder;
    if (!p_stt_decoder)
    {
        dvbpsi_error(p_dvbpsi, "ATSC STT decoder", "No decoder specified");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* FIXME: looks different then from other tables decoders */
    dvbpsi_atsc_stt_t *p_stt;
    p_stt = dvbpsi_atsc_NewSTT(p_section->i_version, p_section->b_current_next);
    if (p_stt)
    {
        /* Decode the sections */
        dvbpsi_atsc_DecodeSTTSections(p_stt, p_section);
        /* Delete the sections */
        dvbpsi_DeletePSISections(p_section);
        /* signal the new STT */
        p_stt_decoder->pf_stt_callback(p_stt_decoder->p_cb_data, p_stt);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeSTTSection
 *****************************************************************************
 * STT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeSTTSections(dvbpsi_atsc_stt_t* p_stt,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t *p_byte, *p_end;
    uint16_t i_length = 0;

    p_byte = p_section->p_payload_start + 1;
    p_stt->i_system_time = (((uint32_t)p_byte[0]) << 24) |
                           (((uint32_t)p_byte[1]) << 16) |
                           (((uint32_t)p_byte[2]) <<  8) |
                           ((uint32_t)p_byte[3]);
    p_stt->i_gps_utc_offset = p_byte[4];
    p_stt->i_daylight_savings = (((uint16_t)p_byte[5]) << 16) |
                                 ((uint16_t)p_byte[6]);
    p_byte += 7;
    /* Table descriptors */
    i_length = (p_section->i_length - 17);
    p_end = p_byte + i_length;

    while(p_byte + 2 <= p_end)
    {
        uint8_t i_tag = p_byte[0];
        uint8_t i_len = p_byte[1];
        if (i_len + 2 <= p_end - p_byte)
            dvbpsi_atsc_STTAddDescriptor(p_stt, i_tag, i_len, p_byte + 2);
        p_byte += 2 + i_len;
    }
}
