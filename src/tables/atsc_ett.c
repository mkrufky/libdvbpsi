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

atsc_ett.c

Decode PSIP Extended Text Table.

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"
#include "descriptor.h"
#include "demux.h"
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
  dvbpsi_atsc_ett_callback      pf_callback;
  void *                        p_cb_data;
  dvbpsi_atsc_ett_etm_version_t * p_etm_versions;
} dvbpsi_atsc_ett_decoder_t;

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_atsc_GatherETTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t* p_section);
/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSection
 *****************************************************************************
 * TDT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DecodeETTSection(dvbpsi_atsc_ett_t* p_ett,
                             dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachETT
 *****************************************************************************
 * Initialize a ETT decoder and return a handle on it.
 *****************************************************************************/
int dvbpsi_atsc_AttachETT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_atsc_ett_decoder_t* p_ett_decoder;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("ETT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);
    return 1;
  }

  p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
  if(p_subdec == NULL)
  {
    return 1;
  }

  p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)malloc(sizeof(dvbpsi_atsc_ett_decoder_t));
  if(p_ett_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* PSI decoder configuration */
  p_subdec->pf_callback = &dvbpsi_atsc_GatherETTSections;
  p_subdec->p_cb_data = p_ett_decoder;
  p_subdec->i_id = ((uint32_t)i_table_id << 16) | i_extension;
  p_subdec->pf_detach = dvbpsi_atsc_DetachETT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* ETT decoder information */
  p_ett_decoder->pf_callback = pf_callback;
  p_ett_decoder->p_cb_data = p_cb_data;
  p_ett_decoder->p_etm_versions = NULL;

  return 0;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachETT
 *****************************************************************************
 * Close a ETT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_atsc_DetachETT(dvbpsi_demux_t * p_demux, uint8_t i_table_id, uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_atsc_ett_decoder_t* p_ett_decoder;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
  if(p_subdec == NULL)
  {
    DVBPSI_ERROR_ARG("ETT Decoder",
                     "No such ETT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
    return;
  }

  p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)p_subdec->p_cb_data;
  if(!p_ett_decoder)
      return;

  dvbpsi_atsc_ett_etm_version_t *p_etm_version, *p_next;
  for (p_etm_version = p_ett_decoder->p_etm_versions; p_etm_version; p_etm_version = p_next)
  {
    p_next = p_etm_version->p_next;
    free(p_etm_version);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitETT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_ett_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett,
                            uint8_t i_version,
                            int b_current_next,
                            uint8_t i_protocol,
                            uint16_t i_ett_table_id,
                            uint32_t i_etm_id)
{
    p_ett->i_version = i_version;
    p_ett->b_current_next = b_current_next;
    p_ett->i_protocol = i_protocol;
    p_ett->i_ett_table_id = i_ett_table_id;
    p_ett->i_etm_id = i_etm_id;
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

#ifndef TRUE
#define TRUE (1 == 1)
#endif /* TRUE */
#ifndef FALSE
#define FALSE (0 == 1)
#endif /* FALSE */

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_atsc_GatherETTSections(dvbpsi_decoder_t* p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t* p_section)
{
  dvbpsi_atsc_ett_decoder_t* p_ett_decoder
                        = (dvbpsi_atsc_ett_decoder_t*)p_private_decoder;
  if(!p_ett_decoder)
      return;

  if(p_section->i_table_id == 0xCC)
  {
    dvbpsi_atsc_ett_t* p_ett;
    dvbpsi_atsc_ett_etm_version_t *p_etm_version;
    int b_found = FALSE;
    uint32_t i_etm_id = ((uint32_t)p_section->p_payload_start[1] << 24) |
                        ((uint32_t)p_section->p_payload_start[2] << 16) |
                        ((uint32_t)p_section->p_payload_start[3] << 8)  |
                        ((uint32_t)p_section->p_payload_start[4] << 0);

    for (p_etm_version = p_ett_decoder->p_etm_versions; p_etm_version; p_etm_version=p_etm_version->p_next)
    {
        if (p_etm_version->i_etm_id == i_etm_id)
        {
            b_found = TRUE;
            break;
        }
    }
    if (!b_found || (p_etm_version->i_version != p_section->i_version))
    {
        dvbpsi_atsc_NewETT(p_ett, p_section->i_version,
                         p_section->b_current_next,
                         p_section->p_payload_start[0],
                         p_section->i_extension,
                         i_etm_id
                         );
        dvbpsi_atsc_DecodeETTSection(p_ett, p_section);
        p_ett_decoder->pf_callback(p_ett_decoder->p_cb_data, p_ett);
    }
    if (!b_found)
    {
        p_etm_version = malloc(sizeof(dvbpsi_atsc_ett_etm_version_t));
        p_etm_version->i_etm_id = i_etm_id;
        p_etm_version->p_next = p_ett_decoder->p_etm_versions;
        p_ett_decoder->p_etm_versions = p_etm_version;
    }
    p_etm_version->i_version = p_section->i_version;
  }

  dvbpsi_DeletePSISections(p_section);
}

/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSection
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DecodeETTSection(dvbpsi_atsc_ett_t* p_ett,
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
