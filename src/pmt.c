/*****************************************************************************
 * pmt.c: PMT decoder/generator
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: pmt.c,v 1.1 2002/01/07 18:30:35 bozo Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "dvbpsi.h"
#include "psi.h"
#include "dvbpsi_private.h"
#include "psi_private.h"
#include "descriptor.h"
#include "pmt.h"
#include "pmt_private.h"


/*****************************************************************************
 * dvbpsi_AttachPMT
 *****************************************************************************
 * Initialize a PMT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPMT(uint16_t i_program_number,
                               dvbpsi_pmt_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  dvbpsi_pmt_decoder_t* p_pmt_decoder;

  if(h_dvbpsi == NULL)
    return NULL;

  p_pmt_decoder = (dvbpsi_pmt_decoder_t*)malloc(sizeof(dvbpsi_pmt_decoder_t));

  if(p_pmt_decoder == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }

  /* PSI decoder configuration */
  h_dvbpsi->pf_callback = &dvbpsi_DecodePMTSection;
  h_dvbpsi->p_private_decoder = p_pmt_decoder;
  h_dvbpsi->i_section_max_size = 1024;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* PMT decoder configuration */
  p_pmt_decoder->i_program_number = i_program_number;
  p_pmt_decoder->pf_callback = pf_callback;
  p_pmt_decoder->p_cb_data = p_cb_data;
  /* PMT decoder initial state */
  dvbpsi_InitPMT(&p_pmt_decoder->current_pmt, 1, 0, 0, 0);
  p_pmt_decoder->b_building_next = 0;
  p_pmt_decoder->b_builded_next = 0;

  return h_dvbpsi;
}


/*****************************************************************************
 * dvbpsi_DetachPMT
 *****************************************************************************
 * Close a PMT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_DetachPMT(dvbpsi_handle h_dvbpsi)
{
  dvbpsi_pmt_decoder_t* p_pmt_decoder
                        = (dvbpsi_pmt_decoder_t*)h_dvbpsi->p_private_decoder;

  dvbpsi_EmptyPMT(&p_pmt_decoder->current_pmt);
  dvbpsi_EmptyPMT(&p_pmt_decoder->next_pmt);

  free(h_dvbpsi->p_private_decoder);
  free(h_dvbpsi);
}


/*****************************************************************************
 * dvbpsi_InitPMT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_InitPMT(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, int b_current_next, uint16_t i_pcr_pid)
{
  p_pmt->i_program_number = i_program_number;
  p_pmt->i_version = i_version;
  p_pmt->b_current_next = b_current_next;
  p_pmt->i_pcr_pid = i_pcr_pid;
  p_pmt->p_first_descriptor = NULL;
  p_pmt->p_first_es = NULL;
  p_pmt->b_complete = 0;
}


/*****************************************************************************
 * dvbpsi_EmptyPMT
 *****************************************************************************
 * Clean a dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_EmptyPMT(dvbpsi_pmt_t* p_pmt)
{
  dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;

  dvbpsi_DeleteDescriptor(p_pmt->p_first_descriptor);

  while(p_es != NULL)
  {
    dvbpsi_pmt_es_t* p_tmp = p_es->p_next;
    dvbpsi_DeleteDescriptor(p_es->p_first_descriptor);
    free(p_es);
    p_es = p_tmp;
  }

  p_pmt->p_first_descriptor = NULL;
  p_pmt->p_first_es = NULL;
}
                        

/*****************************************************************************
 * dvbpsi_PMTAddDescriptor
 *****************************************************************************
 * Add a descriptor in the PMT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTAddDescriptor(dvbpsi_pmt_t* p_pmt,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_pmt->p_first_descriptor == NULL)
    {
      p_pmt->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_pmt->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_PMTAddES
 *****************************************************************************
 * Add an ES in the PMT.
 *****************************************************************************/
dvbpsi_pmt_es_t* dvbpsi_PMTAddES(dvbpsi_pmt_t* p_pmt,
                                 uint8_t i_type, uint16_t i_pid)
{
  dvbpsi_pmt_es_t* p_es = (dvbpsi_pmt_es_t*)malloc(sizeof(dvbpsi_pmt_es_t));

  if(p_es)
  {
    p_es->i_type = i_type;
    p_es->i_pid = i_pid;
    p_es->p_first_descriptor = NULL;
    p_es->p_next = NULL;

    if(p_pmt->p_first_es == NULL)
    {
      p_pmt->p_first_es = p_es;
    }
    else
    {
      dvbpsi_pmt_es_t* p_last_es = p_pmt->p_first_es;
      while(p_last_es->p_next != NULL)
        p_last_es = p_last_es->p_next;
      p_last_es->p_next = p_es;
    }
  }

  return p_es;
}


/*****************************************************************************
 * dvbpsi_PMTESAddDescriptor
 *****************************************************************************
 * Add a descriptor in the PMT ES.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTESAddDescriptor(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_es->p_first_descriptor == NULL)
    {
      p_es->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_es->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_DecodePMTSection
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_DecodePMTSection(dvbpsi_decoder_t* p_decoder,
                             dvbpsi_psi_section_t* p_section)
{
  dvbpsi_pmt_decoder_t* p_pmt_decoder
                        = (dvbpsi_pmt_decoder_t*)p_decoder->p_private_decoder;
  int b_decode = 1;
  int b_reinit = 0;
  uint8_t* p_byte, * p_end;

  DVBPSI_DEBUG_ARG("PMT decoder",
                   "Table version %2d," "i_extension %5d,"
                   "section %3d up to %3d," "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(p_section->i_table_id != 0x02)
  {
    /* Invalid table_id value */
    DVBPSI_ERROR_ARG("PMT decoder",
                     "invalid section (table_id == 0x%02x)",
                     p_section->i_table_id);
    b_decode = 0;
    b_reinit = 1;
  }
  else if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("PMT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_decode = 0;
    b_reinit = 1;
  }
  else if(p_pmt_decoder->i_program_number != p_section->i_extension)
  {
    /* Invalid program_number */
    DVBPSI_ERROR("PMT decoder",
                 "'program_number' don't match");
    b_decode = 0;
  }
  else if(p_decoder->b_discontinuity)
  {
    /* A TS discontinuity has occured */
    b_reinit = 1;
    p_decoder->b_discontinuity = 0;
  }
  else
  {
    /* No TS discontinuity has occured */

    /* Perform some few sanity checks */
    if(p_pmt_decoder->b_building_next)
    {
      if(p_pmt_decoder->next_pmt.i_version != p_section->i_version)
      {
        /* version_number */
        DVBPSI_ERROR("PMT decoder",
                     "'version_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = 1;
      }
      else if(p_pmt_decoder->i_last_section_number != p_section->i_last_number)
      {
        /* last_section_number */
        DVBPSI_ERROR("PMT decoder",
                     "'last_section_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = 1;
      }
      else if(p_pmt_decoder->i_previous_section_number >= p_section->i_number)
      {
        /* section_number */
        DVBPSI_ERROR("PMT decoder",
                     "backward 'section_number'"
                     " whereas no discontinuity has occured");
        b_reinit = 1;
      }
    }
    else
    {
      if(p_pmt_decoder->current_pmt.i_version == p_section->i_version)
      {
        /* signal a new PMT if the previous one wasn't active */
        if(    (p_pmt_decoder->current_pmt.b_complete)
            && (!p_pmt_decoder->current_pmt.b_current_next)
            && (p_section->b_current_next))
        {
          dvbpsi_pmt_t* p_pmt = (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));

          p_pmt_decoder->current_pmt.b_current_next = 1;
          *p_pmt = p_pmt_decoder->current_pmt;
          p_pmt_decoder->current_pmt.p_first_descriptor = NULL;
          p_pmt_decoder->current_pmt.p_first_es = NULL;
          p_pmt_decoder->pf_callback(p_pmt_decoder->p_cb_data, p_pmt);
        }

        /* Don't decode if the version is already decoded */
        if(p_pmt_decoder->current_pmt.b_complete)
          b_decode = 0;
      }
    }
  }

  /* Reinit the decoder if wanted */
  if(b_reinit)
  {
    dvbpsi_EmptyPMT(&p_pmt_decoder->current_pmt);
    p_pmt_decoder->current_pmt.b_complete = 0; /* force redecoding */
    dvbpsi_EmptyPMT(&p_pmt_decoder->next_pmt);
    p_pmt_decoder->b_building_next = 0;
  }

  /* Decode the section if wanted */
  if(b_decode)
  {
    /* Initialization */
    if(!p_pmt_decoder->b_building_next)
    {
      p_pmt_decoder->next_pmt.i_program_number = p_section->i_extension;
      p_pmt_decoder->next_pmt.i_version = p_section->i_version;
      p_pmt_decoder->next_pmt.b_current_next = p_section->b_current_next;
      p_pmt_decoder->next_pmt.i_pcr_pid =
                  ((uint16_t)(p_section->p_payload_start[0] & 0x1f) << 8)
                | p_section->p_payload_start[1];
      p_pmt_decoder->i_last_section_number = p_section->i_last_number;
      p_pmt_decoder->b_section_lost = (p_section->i_number != 0);
      p_pmt_decoder->b_building_next = 1;
    }
    else
    {
      p_pmt_decoder->b_section_lost =
                        (    p_section->i_number
                          != p_pmt_decoder->i_previous_section_number + 1);
    }

    /* Decode section */
    /* - PMT descriptors */
    p_byte = p_section->p_payload_start + 4;
    p_end = p_byte + (   ((uint16_t)(p_section->p_payload_start[2] & 0x0f) << 8)
                       | p_section->p_payload_start[3]);
    while(p_byte + 2 < p_end)
    {
      uint8_t i_tag = p_byte[0];
      uint8_t i_length = p_byte[1];
      if(i_length + 2 <= p_end - p_byte)
      {
        dvbpsi_PMTAddDescriptor(&p_pmt_decoder->next_pmt,
                                i_tag, i_length, p_byte + 2);
      }
      p_byte += 2 + i_length;
    }

    /* - ESs */
    p_pmt_decoder->i_previous_section_number = p_section->i_number;
    for(p_byte = p_end; p_byte < p_section->p_payload_end;)
    {
      uint8_t i_type = p_byte[0];
      uint16_t i_pid = ((uint16_t)(p_byte[1] & 0x1f) << 8) | p_byte[2];
      uint16_t i_length = ((uint16_t)(p_byte[3] & 0x0f) << 8) | p_byte[4];
      dvbpsi_pmt_es_t* p_es = dvbpsi_PMTAddES(&p_pmt_decoder->next_pmt,
                                              i_type, i_pid);
      /* - ES descriptors */
      p_byte += 5;
      p_end = p_byte + i_length;
      while(p_byte + 2 < p_end)
      {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
        {
          dvbpsi_PMTESAddDescriptor(p_es, i_tag, i_length, p_byte + 2);
        }
        p_byte += 2 + i_length;
      }
    }

    if(p_section->i_number == p_pmt_decoder->i_last_section_number)
    {
      /* PMT is finished */
      dvbpsi_pmt_t* p_pmt = (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));

      p_pmt_decoder->next_pmt.b_complete = !p_pmt_decoder->b_section_lost;

      *p_pmt = p_pmt_decoder->next_pmt;

      dvbpsi_EmptyPMT(&p_pmt_decoder->current_pmt);
      p_pmt_decoder->current_pmt.i_program_number = p_pmt->i_program_number;
      p_pmt_decoder->current_pmt.i_version = p_pmt->i_version;
      p_pmt_decoder->current_pmt.b_current_next = p_pmt->b_current_next;
      p_pmt_decoder->current_pmt.i_pcr_pid = p_pmt->i_pcr_pid;
      p_pmt_decoder->current_pmt.b_complete = p_pmt->b_complete;
      /* signal the new PMT */
      p_pmt_decoder->pf_callback(p_pmt_decoder->p_cb_data, p_pmt);
      p_pmt_decoder->next_pmt.p_first_descriptor = NULL;
      p_pmt_decoder->next_pmt.p_first_es = NULL;
      p_pmt_decoder->b_building_next = 0;
    }
  }

  dvbpsi_DeletePSISection(p_section);
}

