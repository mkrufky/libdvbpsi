/*
Copyright (C) 2006-2012 Adam Charrett

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

atsc_ett.c

Decode PSIP Extended Text Table.

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

#include "atsc_ett.h"

/*****************************************************************************
 * dvbpsi_atsc_ett_etm_id_s
 *****************************************************************************
 * ETM Version information.
 *****************************************************************************/
typedef struct dvbpsi_atsc_ett_etm_version_s
{
    uint32_t i_etm_id;
    uint8_t  i_version;
    struct dvbpsi_atsc_ett_etm_version_s *p_next;
}dvbpsi_atsc_ett_etm_version_t;

/*****************************************************************************
 * dvbpsi_atsc_ett_decoder_s
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
typedef struct dvbpsi_atsc_ett_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_atsc_ett_callback      pf_ett_callback;
    void *                        p_cb_data;
    dvbpsi_atsc_ett_etm_version_t * p_etm_versions;
} dvbpsi_atsc_ett_decoder_t;

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
static void dvbpsi_atsc_GatherETTSections(dvbpsi_t * p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSection
 *****************************************************************************
 * TDT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeETTSection(dvbpsi_atsc_ett_t* p_ett,
                                         dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachETT
 *****************************************************************************
 * Initialize a ETT decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_atsc_AttachETT(dvbpsi_t * p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_private;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_atsc_ett_decoder_t* p_ett_decoder;
    p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)malloc(sizeof(dvbpsi_atsc_ett_decoder_t));
    if (p_ett_decoder == NULL)
        return false;

    /* PSI decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachETT,
                                         dvbpsi_atsc_GatherETTSections, DVBPSI_DECODER(p_ett_decoder));
    if (p_subdec == NULL)
    {
        free(p_ett_decoder);
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* ETT decoder information */
    p_ett_decoder->pf_ett_callback = pf_callback;
    p_ett_decoder->p_cb_data = p_cb_data;
    p_ett_decoder->p_etm_versions = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachETT
 *****************************************************************************
 * Close a ETT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_atsc_DetachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_subdec_t* p_subdec;
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_private;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT Decoder",
                     "No such ETT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
        return;
    }

    dvbpsi_atsc_ett_decoder_t* p_ett_decoder;
    p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)p_subdec->p_decoder;
    if (!p_ett_decoder)
        return;

    dvbpsi_atsc_ett_etm_version_t *p_etm_version, *p_next;
    for (p_etm_version = p_ett_decoder->p_etm_versions; p_etm_version; p_etm_version = p_next)
    {
        p_next = p_etm_version->p_next;
        free(p_etm_version);
    }

    free(p_subdec->p_decoder);
    p_subdec->p_decoder = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitETT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_ett_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett,
                         uint8_t i_version,
                         uint8_t i_protocol,
                         uint16_t i_ett_table_id,
                         uint32_t i_etm_id,
                         bool b_current_next)
{
    assert(p_ett);

    p_ett->i_version = i_version;
    p_ett->b_current_next = b_current_next;
    p_ett->i_protocol = i_protocol;
    p_ett->i_ett_table_id = i_ett_table_id;
    p_ett->i_etm_id = i_etm_id;
}

dvbpsi_atsc_ett_t *dvbpsi_atsc_NewETT(uint8_t i_version, uint8_t i_protocol,
             uint16_t i_ett_table_id, uint32_t i_etm_id, bool b_current_next)
{
    dvbpsi_atsc_ett_t *p_ett;
    p_ett = (dvbpsi_atsc_ett_t*)malloc(sizeof(dvbpsi_atsc_ett_t));
    if (p_ett != NULL)
        dvbpsi_atsc_InitETT(p_ett, i_version, b_current_next, i_protocol,
                            i_ett_table_id, i_etm_id);
    return p_ett;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptyETT
 *****************************************************************************
 * Clean a dvbpsi_atsc_ett_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t *p_ett)
{
    if (p_ett->p_etm)
    {
        free(p_ett->p_etm);
        p_ett->p_etm = NULL;
        p_ett->i_etm_length = 0;
    }
}

void dvbpsi_atsc_DeleteETT(dvbpsi_atsc_ett_t *p_ett)
{
    if (p_ett)
        dvbpsi_atsc_EmptyETT(p_ett);
    free(p_ett);
    p_ett = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
static void dvbpsi_atsc_GatherETTSections(dvbpsi_t* p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    // FIXME: Gather*Sections needs updating
    // dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_private;
    dvbpsi_atsc_ett_decoder_t* p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)p_decoder;
    if (!p_ett_decoder)
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder", "No decoder specified");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    if (!p_section->b_syntax_indicator)
    {
        /* Invalid section_syntax_indicator */
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "invalid section (section_syntax_indicator == 0)");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    if (p_section->i_table_id != 0xCC)
    {
        /* Invalid section_syntax_indicator */
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "invalid table id (0x%x)", p_section->i_table_id);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    dvbpsi_debug(p_dvbpsi,"ATSC ETT decoder",
                 "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                 "section %3d up to %3d, " "current %1d",
                 p_section->i_version, p_section->i_table_id,
                 p_section->i_extension,
                 p_section->i_number, p_section->i_last_number,
                 p_section->b_current_next);
#if 0
    /* We have a valid ETT section */
    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        b_reinit = true;
        p_demux->b_discontinuity = true;
    }
#endif

    dvbpsi_atsc_ett_t* p_ett;
    dvbpsi_atsc_ett_etm_version_t *p_etm_version;
    bool b_found = false;

    uint32_t i_etm_id = ((uint32_t)p_section->p_payload_start[1] << 24) |
            ((uint32_t)p_section->p_payload_start[2] << 16) |
            ((uint32_t)p_section->p_payload_start[3] << 8)  |
            ((uint32_t)p_section->p_payload_start[4] << 0);

    for (p_etm_version = p_ett_decoder->p_etm_versions; p_etm_version;
         p_etm_version = p_etm_version->p_next)
    {
        if (p_etm_version->i_etm_id == i_etm_id)
        {
            b_found = true;
            break;
        }
    }

    if (!b_found || (p_etm_version->i_version != p_section->i_version))
    {
        p_ett = dvbpsi_atsc_NewETT(p_section->i_version,
                           p_section->p_payload_start[0],
                           p_section->i_extension,
                           i_etm_id,
                           p_section->b_current_next);
        if (p_ett)
        {
            dvbpsi_atsc_DecodeETTSection(p_ett, p_section);
            p_ett_decoder->pf_ett_callback(p_ett_decoder->p_cb_data, p_ett);
        }
        else
        {
            dvbpsi_error(p_dvbpsi, "ATSC ETT decoder", "Could not signal new ATSC ETT.");
            dvbpsi_DeletePSISections(p_section);
            return;
        }
    }

    if (!b_found)
    {
        p_etm_version = malloc(sizeof(dvbpsi_atsc_ett_etm_version_t));
        if (p_etm_version)
        {
            p_etm_version->i_etm_id = i_etm_id;
            p_etm_version->p_next = p_ett_decoder->p_etm_versions;
            p_ett_decoder->p_etm_versions = p_etm_version;
        }
    }
    p_etm_version->i_version = p_section->i_version;

    dvbpsi_DeletePSISections(p_section);
}

/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSection
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeETTSection(dvbpsi_atsc_ett_t* p_ett,
                             dvbpsi_psi_section_t* p_section)
{
    uint16_t i_etm_length = p_section->i_length - 14;
    uint8_t *p_etm = malloc(i_etm_length);
    if (p_etm)
    {
        memcpy(p_etm, p_section->p_payload_start + 5, i_etm_length);
        p_ett->p_etm = p_etm;
        p_ett->i_etm_length = i_etm_length;
    }
}
