/*
Copyright (C) 2006  Adam Charrett

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

vct.c

Decode PSIP Virtual Channel Table.

*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"
#include "descriptor.h"
#include "demux.h"
#include "atsc_vct.h"

typedef struct dvbpsi_atsc_vct_decoder_s
{
  dvbpsi_atsc_vct_callback      pf_callback;
  void *                        p_cb_data;

  dvbpsi_atsc_vct_t             current_vct;
  dvbpsi_atsc_vct_t *           p_building_vct;

  int                           b_current_valid;

  uint8_t                       i_last_section_number;
  dvbpsi_psi_section_t *        ap_sections [256];

} dvbpsi_atsc_vct_decoder_t;

dvbpsi_descriptor_t *dvbpsi_atsc_VCTAddDescriptor(
                                               dvbpsi_atsc_vct_t *p_vct,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

dvbpsi_atsc_vct_channel_t *dvbpsi_atsc_VCTAddChannel(dvbpsi_atsc_vct_t* p_vct,
                                            uint8_t *p_short_name,
                                            uint16_t i_major_number,
                                            uint16_t i_minor_number,
                                            uint8_t  i_modulation,
                                            uint32_t i_carrier_freq,
                                            uint16_t i_channel_tsid,
                                            uint16_t i_program_number,
                                            uint8_t  i_etm_location,
                                            int      b_access_controlled,
                                            int      b_hidden,
                                            int      b_path_select,
                                            int      b_out_of_band,
                                            int      b_hide_guide,
                                            uint8_t  i_service_type,
                                            uint16_t i_source_id);

dvbpsi_descriptor_t *dvbpsi_atsc_VCTChannelAddDescriptor(
                                               dvbpsi_atsc_vct_channel_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

void dvbpsi_atsc_GatherVCTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section);

void dvbpsi_atsc_DecodeVCTSections(dvbpsi_atsc_vct_t* p_vct,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachVCT
 *****************************************************************************
 * Initialize a VCT subtable decoder.
 *****************************************************************************/
int dvbpsi_atsc_AttachVCT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_vct_callback pf_callback, void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_atsc_vct_decoder_t*  p_vct_decoder;
  unsigned int i;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("VCT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);

    return 1;
  }

  p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
  if(p_subdec == NULL)
  {
    return 1;
  }

  p_vct_decoder = (dvbpsi_atsc_vct_decoder_t*)malloc(sizeof(dvbpsi_atsc_vct_decoder_t));

  if(p_vct_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_atsc_GatherVCTSections;
  p_subdec->p_cb_data = p_vct_decoder;
  p_subdec->i_id = ((uint32_t)i_table_id << 16) | i_extension;
  p_subdec->pf_detach = dvbpsi_atsc_DetachVCT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* VCT decoder information */
  p_vct_decoder->pf_callback = pf_callback;
  p_vct_decoder->p_cb_data = p_cb_data;
  /* VCT decoder initial state */
  p_vct_decoder->b_current_valid = 0;
  p_vct_decoder->p_building_vct = NULL;
  for(i = 0; i <= 255; i++)
    p_vct_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_atsc_DetachVCT
 *****************************************************************************
 * Close a VCT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachVCT(dvbpsi_demux_t * p_demux, uint8_t i_table_id, uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_atsc_vct_decoder_t* p_vct_decoder;

  unsigned int i;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("VCT Decoder",
                     "No such VCT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
    return;
  }

  p_vct_decoder = (dvbpsi_atsc_vct_decoder_t*)p_subdec->p_cb_data;
  if (p_vct_decoder->p_building_vct)
  {
    free(p_vct_decoder->p_building_vct);
  }

  for(i = 0; i <= 255; i++)
  {
    if(p_vct_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_vct_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_atsc_InitVCT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_vct_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitVCT(dvbpsi_atsc_vct_t* p_vct,uint8_t i_version, int b_current_next,
                       uint8_t i_protocol, uint16_t i_ts_id, int b_cable_vct)
{
  p_vct->i_version = i_version;
  p_vct->b_current_next = b_current_next;
  p_vct->i_protocol = i_protocol;
  p_vct->i_ts_id = i_ts_id;
  p_vct->b_cable_vct = b_cable_vct;
  p_vct->p_first_channel = NULL;
  p_vct->p_first_descriptor = NULL;
}


/*****************************************************************************
 * dvbpsi_atsc_EmptyVCT
 *****************************************************************************
 * Clean a dvbpsi_atsc_vct_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyVCT(dvbpsi_atsc_vct_t* p_vct)
{
  dvbpsi_atsc_vct_channel_t* p_channel = p_vct->p_first_channel;

  while(p_channel != NULL)
  {
    dvbpsi_atsc_vct_channel_t* p_tmp = p_channel->p_next;
    dvbpsi_DeleteDescriptors(p_channel->p_first_descriptor);
    free(p_channel);
    p_channel = p_tmp;
  }
  dvbpsi_DeleteDescriptors(p_vct->p_first_descriptor);
  p_vct->p_first_channel = NULL;
  p_vct->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_VCTAddDescriptor
 *****************************************************************************
 * Add a descriptor to the VCT table.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_VCTAddDescriptor(
                                               dvbpsi_atsc_vct_t *p_vct,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_vct->p_first_descriptor == NULL)
    {
      p_vct->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_vct->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}
/*****************************************************************************
 * dvbpsi_atsc_VCTAddChannel
 *****************************************************************************
 * Add a Channel description at the end of the VCT.
 *****************************************************************************/
dvbpsi_atsc_vct_channel_t *dvbpsi_atsc_VCTAddChannel(dvbpsi_atsc_vct_t* p_vct,
                                            uint8_t *p_short_name,
                                            uint16_t i_major_number,
                                            uint16_t i_minor_number,
                                            uint8_t  i_modulation,
                                            uint32_t i_carrier_freq,
                                            uint16_t i_channel_tsid,
                                            uint16_t i_program_number,
                                            uint8_t  i_etm_location,
                                            int      b_access_controlled,
                                            int      b_hidden,
                                            int      b_path_select,
                                            int      b_out_of_band,
                                            int      b_hide_guide,
                                            uint8_t  i_service_type,
                                            uint16_t i_source_id)
{
  dvbpsi_atsc_vct_channel_t * p_channel
                = (dvbpsi_atsc_vct_channel_t*)malloc(sizeof(dvbpsi_atsc_vct_channel_t));

  if(p_channel)
  {
    memcpy(p_channel->i_short_name, p_short_name, sizeof(uint16_t) * 7);
    p_channel->i_major_number = i_major_number;
    p_channel->i_minor_number = i_minor_number;
    p_channel->i_modulation = i_modulation;
    p_channel->i_carrier_freq = i_carrier_freq;
    p_channel->i_channel_tsid = i_channel_tsid;
    p_channel->i_program_number = i_program_number;
    p_channel->i_etm_location = i_etm_location;
    p_channel->b_access_controlled = b_access_controlled;
    p_channel->b_path_select = b_path_select;
    p_channel->b_out_of_band = b_out_of_band;
    p_channel->b_hidden = b_hidden;
    p_channel->b_hide_guide = b_hide_guide;
    p_channel->i_service_type = i_service_type;
    p_channel->i_source_id = i_source_id;

    p_channel->p_first_descriptor = NULL;
    p_channel->p_next = NULL;

    if(p_vct->p_first_channel== NULL)
    {
      p_vct->p_first_channel = p_channel;
    }
    else
    {
      dvbpsi_atsc_vct_channel_t * p_last_channel = p_vct->p_first_channel;
      while(p_last_channel->p_next != NULL)
        p_last_channel = p_last_channel->p_next;
      p_last_channel->p_next = p_channel;
    }
  }

  return p_channel;
}


/*****************************************************************************
 * dvbpsi_VCTTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the VCT table description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_VCTChannelAddDescriptor(
                                               dvbpsi_atsc_vct_channel_t *p_channel,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_channel->p_first_descriptor == NULL)
    {
      p_channel->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_channel->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_atsc_GatherVCTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_atsc_GatherVCTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_atsc_vct_decoder_t * p_vct_decoder
                        = (dvbpsi_atsc_vct_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;
  unsigned int i;

  DVBPSI_DEBUG_ARG("VCT decoder",
                   "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_table_id,
                   p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("VCT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid VCT section */
  if(b_append)
  {
    /* TS discontinuity check */
    if(p_psi_decoder->b_discontinuity)
    {
      b_reinit = 1;
      p_psi_decoder->b_discontinuity = 0;
    }
    else
    {
      /* Perform a few sanity checks */
      if(p_vct_decoder->p_building_vct)
      {
        if(p_vct_decoder->p_building_vct->i_ts_id != p_section->i_extension)
        {
          /* transport_stream_id */
          DVBPSI_ERROR("VCT decoder",
                       "'transport_stream_id' differs"
                       " whereas no TS discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_vct_decoder->p_building_vct->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("VCT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_vct_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("VCT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_vct_decoder->b_current_valid)
            && (p_vct_decoder->current_vct.i_version == p_section->i_version))
        {
          /* Signal a new VCT if the previous one wasn't active */
          if(    (!p_vct_decoder->current_vct.b_current_next)
              && (p_section->b_current_next))
          {
            dvbpsi_atsc_vct_t * p_vct = (dvbpsi_atsc_vct_t*)malloc(sizeof(dvbpsi_atsc_vct_t));

            p_vct_decoder->current_vct.b_current_next = 1;
            *p_vct = p_vct_decoder->current_vct;
            p_vct_decoder->pf_callback(p_vct_decoder->p_cb_data, p_vct);
          }

          /* Don't decode since this version is already decoded */
          b_append = 0;
        }
      }
    }
  }

  /* Reinit the decoder if wanted */
  if(b_reinit)
  {
    /* Force redecoding */
    p_vct_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_vct_decoder->p_building_vct)
    {
      free(p_vct_decoder->p_building_vct);
      p_vct_decoder->p_building_vct = NULL;
    }
    /* Clear the section array */
    for(i = 0; i <= 255; i++)
    {
      if(p_vct_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_vct_decoder->ap_sections[i]);
        p_vct_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_vct_decoder->p_building_vct)
    {
      p_vct_decoder->p_building_vct =
                                (dvbpsi_atsc_vct_t*)malloc(sizeof(dvbpsi_atsc_vct_t));
      dvbpsi_atsc_InitVCT(p_vct_decoder->p_building_vct,
                     p_section->i_version,
                     p_section->b_current_next,
                     p_section->p_payload_start[0],
                     p_section->i_extension,
                     p_section->i_table_id == 0xC9);
      p_vct_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_vct_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("VCT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_vct_decoder->ap_sections[p_section->i_number]);
    }
    p_vct_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(i = 0; i <= p_vct_decoder->i_last_section_number; i++)
    {
      if(!p_vct_decoder->ap_sections[i])
        break;

      if(i == p_vct_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_vct_decoder->current_vct = *p_vct_decoder->p_building_vct;
      p_vct_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_vct_decoder->i_last_section_number)
      {
        for(i = 0; i <= p_vct_decoder->i_last_section_number - 1; i++)
          p_vct_decoder->ap_sections[i]->p_next =
                                        p_vct_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_atsc_DecodeVCTSections(p_vct_decoder->p_building_vct,
                               p_vct_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_vct_decoder->ap_sections[0]);
      /* signal the new VCT */
      p_vct_decoder->pf_callback(p_vct_decoder->p_cb_data,
                                 p_vct_decoder->p_building_vct);
      /* Reinitialize the structures */
      p_vct_decoder->p_building_vct = NULL;
      for(i = 0; i <= p_vct_decoder->i_last_section_number; i++)
        p_vct_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeVCTSection
 *****************************************************************************
 * VCT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DecodeVCTSections(dvbpsi_atsc_vct_t* p_vct,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    uint16_t i_channels_defined = p_section->p_payload_start[1];
    uint16_t i_channels_count = 0;
    uint16_t i_length = 0;

    for(p_byte = p_section->p_payload_start + 2;
        ((p_byte + 6) < p_section->p_payload_end) && (i_channels_count < i_channels_defined);
        i_channels_count ++)
    {
        dvbpsi_atsc_vct_channel_t* p_channel;
        uint16_t i_major_number      = ((uint16_t)(p_byte[14] & 0xf) << 6) | ((uint16_t)(p_byte[15] & 0xfc) >> 2);
        uint16_t i_minor_number      = ((uint16_t)(p_byte[15] & 0x3) << 8) | ((uint16_t) p_byte[16]);
        uint8_t  i_modulation        = p_byte[17];
        uint32_t i_carrier_freq      = ((uint32_t)(p_byte[18] << 24)) |
                                       ((uint32_t)(p_byte[19] << 16)) |
                                       ((uint32_t)(p_byte[20] <<  8)) |
                                       ((uint32_t)(p_byte[21]));
        uint16_t i_channel_tsid      = ((uint16_t)(p_byte[22] << 8)) |  ((uint16_t)p_byte[23]);
        uint16_t i_program_number    = ((uint16_t)(p_byte[24] << 8)) |  ((uint16_t)p_byte[25]);
        uint8_t  i_etm_location      = (uint8_t)((p_byte[26] & 0xC0) >> 6);
        int      b_access_controlled = (p_byte[26] & 0x20) >> 5;
        int      b_hidden            = (p_byte[26] & 0x10) >> 4;
        int      b_path_select       = (p_byte[26] & 0x08) >> 3;
        int      b_out_of_band       = (p_byte[26] & 0x04) >> 2;
        int      b_hide_guide        = (p_byte[26] & 0x02) >> 1;
        uint8_t  i_service_type      = (uint8_t)(p_byte[27] & 0x3f);
        uint16_t i_source_id         = ((uint16_t)(p_byte[28] << 8)) |  ((uint16_t)p_byte[29]);
        i_length = ((uint16_t)(p_byte[30] & 0x3) <<8) | p_byte[31];

        p_channel = dvbpsi_atsc_VCTAddChannel(p_vct, p_byte,
                                    i_major_number, i_minor_number,
                                    i_modulation, i_carrier_freq,
                                    i_channel_tsid, i_program_number,
                                    i_etm_location, b_access_controlled,
                                    b_hidden, b_path_select, b_out_of_band,
                                    b_hide_guide, i_service_type, i_source_id);

        /* Table descriptors */
        p_byte += 32;
        p_end = p_byte + i_length;
        if( p_end > p_section->p_payload_end ) break;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if(i_length + 2 <= p_end - p_byte)
              dvbpsi_atsc_VCTChannelAddDescriptor(p_channel, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }
    }

    /* Table descriptors */
    i_length = ((uint16_t)(p_byte[0] & 0x3) <<8) | p_byte[1];
    p_byte += 2;
    p_end = p_byte + i_length;

    while(p_byte + 2 <= p_end)
    {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_atsc_VCTAddDescriptor(p_vct, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
    }
    p_section = p_section->p_next;
  }
}

