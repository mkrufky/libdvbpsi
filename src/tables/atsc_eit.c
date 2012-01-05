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

eit.c

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
#include "atsc_eit.h"

typedef struct dvbpsi_atsc_eit_decoder_s
{
  dvbpsi_atsc_eit_callback      pf_callback;
  void *                        p_cb_data;

  dvbpsi_atsc_eit_t             current_eit;
  dvbpsi_atsc_eit_t *           p_building_eit;

  int                           b_current_valid;

  uint8_t                       i_last_section_number;
  dvbpsi_psi_section_t *        ap_sections [256];

} dvbpsi_atsc_eit_decoder_t;


dvbpsi_atsc_eit_event_t *dvbpsi_atsc_EITAddEvent(dvbpsi_atsc_eit_t* p_eit,
                                            uint16_t i_event_id,
                                            uint32_t i_start_time,
                                            uint8_t  i_etm_location,
                                            uint32_t i_length_seconds,
                                            uint8_t i_title_length,
                                            uint8_t *p_title);

dvbpsi_descriptor_t *dvbpsi_atsc_EITEventAddDescriptor(
                                               dvbpsi_atsc_eit_event_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

void dvbpsi_atsc_GatherEITSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section);

void dvbpsi_atsc_DecodeEITSections(dvbpsi_atsc_eit_t* p_eit,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachEIT
 *****************************************************************************
 * Initialize a EIT subtable decoder.
 *****************************************************************************/
int dvbpsi_atsc_AttachEIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_eit_callback pf_callback, void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_atsc_eit_decoder_t*  p_eit_decoder;
  unsigned int i;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("EIT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);

    return 1;
  }

  p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
  if(p_subdec == NULL)
  {
    return 1;
  }

  p_eit_decoder = (dvbpsi_atsc_eit_decoder_t*)malloc(sizeof(dvbpsi_atsc_eit_decoder_t));

  if(p_eit_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_atsc_GatherEITSections;
  p_subdec->p_cb_data = p_eit_decoder;
  p_subdec->i_id = ((uint32_t)i_table_id << 16) | i_extension;
  p_subdec->pf_detach = dvbpsi_atsc_DetachEIT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* EIT decoder information */
  p_eit_decoder->pf_callback = pf_callback;
  p_eit_decoder->p_cb_data = p_cb_data;
  /* EIT decoder initial state */
  p_eit_decoder->b_current_valid = 0;
  p_eit_decoder->p_building_eit = NULL;
  for(i = 0; i <= 255; i++)
    p_eit_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_atsc_DetachEIT
 *****************************************************************************
 * Close a EIT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id, uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_atsc_eit_decoder_t* p_eit_decoder;

  unsigned int i;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("EIT Decoder",
                     "No such EIT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
    return;
  }

  p_eit_decoder = (dvbpsi_atsc_eit_decoder_t*)p_subdec->p_cb_data;
  if (p_eit_decoder->p_building_eit)
  {
    free(p_eit_decoder->p_building_eit);
  }

  for(i = 0; i <= 255; i++)
  {
    if(p_eit_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_atsc_InitEIT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_eit_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitEIT(dvbpsi_atsc_eit_t* p_eit,uint8_t i_version, int b_current_next,
                       uint8_t i_protocol, uint16_t i_source_id)
{
  p_eit->i_version = i_version;
  p_eit->b_current_next = b_current_next;
  p_eit->i_protocol = i_protocol;
  p_eit->i_source_id = i_source_id;
  p_eit->p_first_event = NULL;
}


/*****************************************************************************
 * dvbpsi_atsc_EmptyEIT
 *****************************************************************************
 * Clean a dvbpsi_atsc_eit_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyEIT(dvbpsi_atsc_eit_t* p_eit)
{
  dvbpsi_atsc_eit_event_t* p_event = p_eit->p_first_event;

  while(p_event != NULL)
  {
    dvbpsi_atsc_eit_event_t* p_tmp = p_event->p_next;
    dvbpsi_DeleteDescriptors(p_event->p_first_descriptor);
    free(p_event);
    p_event = p_tmp;
  }
  p_eit->p_first_event = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_EITAddChannel
 *****************************************************************************
 * Add a Channel description at the end of the EIT.
 *****************************************************************************/
dvbpsi_atsc_eit_event_t *dvbpsi_atsc_EITAddEvent(dvbpsi_atsc_eit_t* p_eit,
                                            uint16_t i_event_id,
                                            uint32_t i_start_time,
                                            uint8_t  i_etm_location,
                                            uint32_t i_length_seconds,
                                            uint8_t i_title_length,
                                            uint8_t *p_title)
{
  dvbpsi_atsc_eit_event_t * p_event
                = (dvbpsi_atsc_eit_event_t*)malloc(sizeof(dvbpsi_atsc_eit_event_t));

  if(p_event)
  {
    p_event->i_event_id = i_event_id;
    p_event->i_start_time = i_start_time;
    p_event->i_etm_location = i_etm_location;
    p_event->i_length_seconds = i_length_seconds;
    p_event->i_title_length = i_title_length;

    memcpy(p_event->i_title, p_title, i_title_length);

    p_event->p_first_descriptor = NULL;
    p_event->p_next = NULL;

    if(p_eit->p_first_event== NULL)
    {
      p_eit->p_first_event = p_event;
    }
    else
    {
      dvbpsi_atsc_eit_event_t * p_last_event = p_eit->p_first_event;
      while(p_last_event->p_next != NULL)
        p_last_event = p_last_event->p_next;
      p_last_event->p_next = p_event;
    }
  }

  return p_event;
}


/*****************************************************************************
 * dvbpsi_EITTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the EIT table description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_EITChannelAddDescriptor(
                                               dvbpsi_atsc_eit_event_t *p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_event->p_first_descriptor == NULL)
    {
      p_event->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_event->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_atsc_GatherEITSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_atsc_GatherEITSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_atsc_eit_decoder_t * p_eit_decoder
                        = (dvbpsi_atsc_eit_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;
  unsigned int i;

  DVBPSI_DEBUG_ARG("EIT decoder",
                   "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_table_id,
                   p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("EIT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid EIT section */
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
      if(p_eit_decoder->p_building_eit)
      {
        if(p_eit_decoder->p_building_eit->i_source_id != p_section->i_extension)
        {
          /* transport_stream_id */
          DVBPSI_ERROR("EIT decoder",
                       "'transport_stream_id' differs"
                       " whereas no TS discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_eit_decoder->p_building_eit->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("EIT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_eit_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("EIT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_eit_decoder->b_current_valid)
            && (p_eit_decoder->current_eit.i_version == p_section->i_version))
        {
          /* Signal a new EIT if the previous one wasn't active */
          if(    (!p_eit_decoder->current_eit.b_current_next)
              && (p_section->b_current_next))
          {
            dvbpsi_atsc_eit_t * p_eit = (dvbpsi_atsc_eit_t*)malloc(sizeof(dvbpsi_atsc_eit_t));

            p_eit_decoder->current_eit.b_current_next = 1;
            *p_eit = p_eit_decoder->current_eit;
            p_eit_decoder->pf_callback(p_eit_decoder->p_cb_data, p_eit);
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
    p_eit_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_eit_decoder->p_building_eit)
    {
      free(p_eit_decoder->p_building_eit);
      p_eit_decoder->p_building_eit = NULL;
    }
    /* Clear the section array */
    for(i = 0; i <= 255; i++)
    {
      if(p_eit_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[i]);
        p_eit_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_eit_decoder->p_building_eit)
    {
      dvbpsi_atsc_NewEIT(p_eit_decoder->p_building_eit,
                     p_section->i_version,
                     p_section->b_current_next,
                     p_section->p_payload_start[0],
                     p_section->i_extension);
      p_eit_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_eit_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("EIT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[p_section->i_number]);
    }
    p_eit_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(i = 0; i <= p_eit_decoder->i_last_section_number; i++)
    {
      if(!p_eit_decoder->ap_sections[i])
        break;

      if(i == p_eit_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_eit_decoder->current_eit = *p_eit_decoder->p_building_eit;
      p_eit_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_eit_decoder->i_last_section_number)
      {
        for(i = 0; i <= p_eit_decoder->i_last_section_number - 1; i++)
          p_eit_decoder->ap_sections[i]->p_next =
                                        p_eit_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_atsc_DecodeEITSections(p_eit_decoder->p_building_eit,
                               p_eit_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[0]);
      /* signal the new EIT */
      p_eit_decoder->pf_callback(p_eit_decoder->p_cb_data,
                                 p_eit_decoder->p_building_eit);
      /* Reinitialize the structures */
      p_eit_decoder->p_building_eit = NULL;
      for(i = 0; i <= p_eit_decoder->i_last_section_number; i++)
        p_eit_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeEITSection
 *****************************************************************************
 * EIT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DecodeEITSections(dvbpsi_atsc_eit_t* p_eit,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    uint16_t i_number_events = p_section->p_payload_start[1];
    uint16_t i_events_count = 0;
    uint16_t i_length = 0;

    for(p_byte = p_section->p_payload_start + 2;
        ((p_byte + 4) < p_section->p_payload_end) && (i_events_count < i_number_events);
        i_events_count ++)
    {
        dvbpsi_atsc_eit_event_t* p_event;
        uint16_t i_event_id          = ((uint16_t)(p_byte[0] & 0x3f) << 8) | ((uint16_t) p_byte[1]);
        uint32_t i_start_time        = ((uint32_t)(p_byte[2] << 24)) |
                                       ((uint32_t)(p_byte[3] << 16)) |
                                       ((uint32_t)(p_byte[4] <<  8)) |
                                       ((uint32_t)(p_byte[5]));
        uint8_t  i_etm_location      = (uint8_t)((p_byte[6] & 0x30) >> 4);
        uint32_t i_length_seconds    = ((uint32_t)((p_byte[6] & 0x0f) << 16)) |
                                       ((uint32_t)(p_byte[7] << 8)) |
                                       ((uint32_t)(p_byte[8]));
        uint8_t  i_title_length      = p_byte[9];

        p_byte += 10;
        p_event = dvbpsi_atsc_EITAddEvent(p_eit, i_event_id, i_start_time,
                                i_etm_location, i_length_seconds, i_title_length,
                                p_byte);
        p_byte += i_title_length;
        i_length = ((uint16_t)(p_byte[0] & 0xf) <<8) | p_byte[1];
        /* Table descriptors */
        p_byte += 2;
        p_end = p_byte + i_length;
        if( p_end > p_section->p_payload_end ) break;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if(i_length + 2 <= p_end - p_byte)
              dvbpsi_atsc_EITChannelAddDescriptor(p_event, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }
    }

    p_section = p_section->p_next;
  }
}


