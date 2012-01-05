/*
Copyright (C) 2011  Michael Krufky

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

mgt.c

Decode PSIP Master Guide Table.

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
#include "atsc_mgt.h"

typedef struct dvbpsi_atsc_mgt_decoder_s
{
  dvbpsi_atsc_mgt_callback      pf_callback;
  void *                        p_cb_data;

  dvbpsi_atsc_mgt_t             current_mgt;
  dvbpsi_atsc_mgt_t *           p_building_mgt;

  int                           b_current_valid;

  uint8_t                       i_last_section_number;
  dvbpsi_psi_section_t *        ap_sections [256];

} dvbpsi_atsc_mgt_decoder_t;

dvbpsi_descriptor_t *dvbpsi_atsc_MGTAddDescriptor(
                                               dvbpsi_atsc_mgt_t *p_mgt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

dvbpsi_atsc_mgt_table_t *dvbpsi_atsc_MGTAddTable(dvbpsi_atsc_mgt_t* p_mgt,
						 uint16_t i_table_type,
						 uint16_t i_table_type_pid,
						 uint8_t  i_table_type_version,
						 uint32_t i_number_bytes);

dvbpsi_descriptor_t *dvbpsi_atsc_MGTTableAddDescriptor(
                                               dvbpsi_atsc_mgt_table_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

void dvbpsi_atsc_GatherMGTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section);

void dvbpsi_atsc_DecodeMGTSections(dvbpsi_atsc_mgt_t* p_mgt,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachMGT
 *****************************************************************************
 * Initialize a MGT subtable decoder.
 *****************************************************************************/
int dvbpsi_atsc_AttachMGT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_atsc_mgt_decoder_t*  p_mgt_decoder;
  unsigned int i;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("MGT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);

    return 1;
  }

  p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
  if(p_subdec == NULL)
  {
    return 1;
  }

  p_mgt_decoder = (dvbpsi_atsc_mgt_decoder_t*)malloc(sizeof(dvbpsi_atsc_mgt_decoder_t));

  if(p_mgt_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_atsc_GatherMGTSections;
  p_subdec->p_cb_data = p_mgt_decoder;
  p_subdec->i_id = ((uint32_t)i_table_id << 16) | i_extension;
  p_subdec->pf_detach = dvbpsi_atsc_DetachMGT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* MGT decoder information */
  p_mgt_decoder->pf_callback = pf_callback;
  p_mgt_decoder->p_cb_data = p_cb_data;
  /* MGT decoder initial state */
  p_mgt_decoder->b_current_valid = 0;
  p_mgt_decoder->p_building_mgt = NULL;
  for(i = 0; i <= 255; i++)
    p_mgt_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_atsc_DetachMGT
 *****************************************************************************
 * Close a MGT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachMGT(dvbpsi_demux_t * p_demux, uint8_t i_table_id, uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_atsc_mgt_decoder_t* p_mgt_decoder;

  unsigned int i;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("MGT Decoder",
                     "No such MGT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
    return;
  }

  p_mgt_decoder = (dvbpsi_atsc_mgt_decoder_t*)p_subdec->p_cb_data;
  if (p_mgt_decoder->p_building_mgt)
  {
    free(p_mgt_decoder->p_building_mgt);
  }

  for(i = 0; i <= 255; i++)
  {
    if(p_mgt_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_mgt_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_atsc_InitMGT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_mgt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt,uint8_t i_version, int b_current_next,
			 uint8_t i_protocol, uint16_t i_table_id_extension)
{
  p_mgt->i_version = i_version;
  p_mgt->b_current_next = b_current_next;
  p_mgt->i_protocol = i_protocol;
  p_mgt->i_table_id_ext = i_table_id_extension;
  p_mgt->p_first_table = NULL;
  p_mgt->p_first_descriptor = NULL;
}


/*****************************************************************************
 * dvbpsi_atsc_EmptyMGT
 *****************************************************************************
 * Clean a dvbpsi_atsc_mgt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t* p_mgt)
{
  dvbpsi_atsc_mgt_table_t* p_table = p_mgt->p_first_table;

  while(p_table != NULL)
  {
    dvbpsi_atsc_mgt_table_t* p_tmp = p_table->p_next;
    dvbpsi_DeleteDescriptors(p_table->p_first_descriptor);
    free(p_table);
    p_table = p_tmp;
  }
  dvbpsi_DeleteDescriptors(p_mgt->p_first_descriptor);
  p_mgt->p_first_table = NULL;
  p_mgt->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_MGTAddDescriptor
 *****************************************************************************
 * Add a descriptor to the MGT table.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_MGTAddDescriptor(
                                               dvbpsi_atsc_mgt_t *p_mgt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_mgt->p_first_descriptor == NULL)
    {
      p_mgt->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_mgt->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}
/*****************************************************************************
 * dvbpsi_atsc_MGTAddTable
 *****************************************************************************
 * Add a Table description at the end of the MGT.
 *****************************************************************************/
dvbpsi_atsc_mgt_table_t *dvbpsi_atsc_MGTAddTable(dvbpsi_atsc_mgt_t* p_mgt,
						 uint16_t i_table_type,
						 uint16_t i_table_type_pid,
						 uint8_t  i_table_type_version,
						 uint32_t i_number_bytes)
{
  dvbpsi_atsc_mgt_table_t * p_table
                = (dvbpsi_atsc_mgt_table_t*)malloc(sizeof(dvbpsi_atsc_mgt_table_t));

  if(p_table)
  {
    p_table->i_table_type = i_table_type;
    p_table->i_table_type_pid = i_table_type_pid;
    p_table->i_table_type_version = i_table_type_version;
    p_table->i_number_bytes = i_number_bytes;

    p_table->p_first_descriptor = NULL;
    p_table->p_next = NULL;

    if(p_mgt->p_first_table== NULL)
    {
      p_mgt->p_first_table = p_table;
    }
    else
    {
      dvbpsi_atsc_mgt_table_t * p_last_table = p_mgt->p_first_table;
      while(p_last_table->p_next != NULL)
        p_last_table = p_last_table->p_next;
      p_last_table->p_next = p_table;
    }
  }

  return p_table;
}


/*****************************************************************************
 * dvbpsi_MGTTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the MGT table description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_atsc_MGTTableAddDescriptor(
                                               dvbpsi_atsc_mgt_table_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_table->p_first_descriptor == NULL)
    {
      p_table->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_table->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_atsc_GatherMGTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_atsc_GatherMGTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_atsc_mgt_decoder_t * p_mgt_decoder
                        = (dvbpsi_atsc_mgt_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;
  unsigned int i;

  DVBPSI_DEBUG_ARG("MGT decoder",
                   "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_table_id,
                   p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("MGT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid MGT section */
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
      if(p_mgt_decoder->p_building_mgt)
      {
        if(p_mgt_decoder->p_building_mgt->i_table_id_ext != p_section->i_extension)
        {
          /* transport_stream_id */
          DVBPSI_ERROR("MGT decoder",
                       "'transport_stream_id' differs"
                       " whereas no TS discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_mgt_decoder->p_building_mgt->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("MGT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_mgt_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("MGT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_mgt_decoder->b_current_valid)
            && (p_mgt_decoder->current_mgt.i_version == p_section->i_version))
        {
          /* Signal a new MGT if the previous one wasn't active */
          if(    (!p_mgt_decoder->current_mgt.b_current_next)
              && (p_section->b_current_next))
          {
            dvbpsi_atsc_mgt_t * p_mgt = (dvbpsi_atsc_mgt_t*)malloc(sizeof(dvbpsi_atsc_mgt_t));

            p_mgt_decoder->current_mgt.b_current_next = 1;
            *p_mgt = p_mgt_decoder->current_mgt;
            p_mgt_decoder->pf_callback(p_mgt_decoder->p_cb_data, p_mgt);
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
    p_mgt_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_mgt_decoder->p_building_mgt)
    {
      free(p_mgt_decoder->p_building_mgt);
      p_mgt_decoder->p_building_mgt = NULL;
    }
    /* Clear the section array */
    for(i = 0; i <= 255; i++)
    {
      if(p_mgt_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_mgt_decoder->ap_sections[i]);
        p_mgt_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_mgt_decoder->p_building_mgt)
    {
      p_mgt_decoder->p_building_mgt =
                                (dvbpsi_atsc_mgt_t*)malloc(sizeof(dvbpsi_atsc_mgt_t));
      dvbpsi_atsc_InitMGT(p_mgt_decoder->p_building_mgt,
                     p_section->i_version,
                     p_section->b_current_next,
                     p_section->p_payload_start[0],
                     p_section->i_extension);
      p_mgt_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_mgt_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("MGT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_mgt_decoder->ap_sections[p_section->i_number]);
    }
    p_mgt_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(i = 0; i <= p_mgt_decoder->i_last_section_number; i++)
    {
      if(!p_mgt_decoder->ap_sections[i])
        break;

      if(i == p_mgt_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_mgt_decoder->current_mgt = *p_mgt_decoder->p_building_mgt;
      p_mgt_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_mgt_decoder->i_last_section_number)
      {
        for(i = 0; i <= p_mgt_decoder->i_last_section_number - 1; i++)
          p_mgt_decoder->ap_sections[i]->p_next =
                                        p_mgt_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_atsc_DecodeMGTSections(p_mgt_decoder->p_building_mgt,
                               p_mgt_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_mgt_decoder->ap_sections[0]);
      /* signal the new MGT */
      p_mgt_decoder->pf_callback(p_mgt_decoder->p_cb_data,
                                 p_mgt_decoder->p_building_mgt);
      /* Reinitialize the structures */
      p_mgt_decoder->p_building_mgt = NULL;
      for(i = 0; i <= p_mgt_decoder->i_last_section_number; i++)
        p_mgt_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeMGTSection
 *****************************************************************************
 * MGT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DecodeMGTSections(dvbpsi_atsc_mgt_t* p_mgt,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    uint16_t i_tables_defined = (p_section->p_payload_start[1] << 8) |
                                (p_section->p_payload_start[2]);
    uint16_t i_tables_count = 0;
    uint16_t i_length = 0;

    for(p_byte = p_section->p_payload_start + 3;
        ((p_byte + 6) < p_section->p_payload_end) && (i_tables_count < i_tables_defined);
        i_tables_count ++)
    {
	dvbpsi_atsc_mgt_table_t* p_table;
	uint16_t i_table_type         = ((uint16_t)(p_byte[0]) << 8) |
	                                ((uint16_t)(p_byte[1]));
	uint16_t i_table_type_pid     = ((uint16_t)(p_byte[2] & 0x1f) << 8) |
	                                ((uint16_t)(p_byte[3] & 0xff));
	uint8_t  i_table_type_version = ((uint8_t) (p_byte[4] & 0x1f));
	uint32_t i_number_bytes       = ((uint32_t)(p_byte[5] << 24)) |
                                        ((uint32_t)(p_byte[6] << 16)) |
                                        ((uint32_t)(p_byte[7] <<  8)) |
                                        ((uint32_t)(p_byte[8]));
        i_length = ((uint16_t)(p_byte[9] & 0xf) <<8) | p_byte[10];

        p_table = dvbpsi_atsc_MGTAddTable(p_mgt,
					  i_table_type,
					  i_table_type_pid,
					  i_table_type_version,
					  i_number_bytes);

        /* Table descriptors */
        p_byte += 11;
        p_end = p_byte + i_length;
        if( p_end > p_section->p_payload_end ) break;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if(i_length + 2 <= p_end - p_byte)
              dvbpsi_atsc_MGTTableAddDescriptor(p_table, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }
    }

    /* Table descriptors */
    i_length = ((uint16_t)(p_byte[0] & 0xf) <<8) | p_byte[1];
    p_byte += 2;
    p_end = p_byte + i_length;

    while(p_byte + 2 <= p_end)
    {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_atsc_MGTAddDescriptor(p_mgt, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
    }
    p_section = p_section->p_next;
  }
}
