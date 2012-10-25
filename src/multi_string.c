/*****************************************************************************
 * multi_string.c: multi_string functions
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: multi_string.c,v 1.0 2012/7/15 14:15:14 sam Exp $
 *
 * Authors: Yonathan Yusim <yonathan@boxee.tv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "dvbpsi.h"
#include "multi_string.h"

/*****************************************************************************
 * dvbpsi_BuildMultiString
 *****************************************************************************
 * Build multiple string structure
 *****************************************************************************/
multi_string_t * dvbpsi_BuildMultiString(uint8_t* p_buf)
{
    uint32_t i_number_strings;
    uint32_t i;
    multi_string_t* p_multi_string = NULL, **p_multi_string_head;

    p_multi_string_head = &p_multi_string;
    i_number_strings = *p_buf++;

    for (i=0; i<i_number_strings; i++)
    {
      uint32_t i_number_segments;
      uint32_t j;
      uint8_t* p_segment;
      uint32_t i_number_bytes_segments = 0;

      p_multi_string = (multi_string_t*)calloc(1, sizeof(multi_string_t));
      if (p_multi_string == NULL)
        return NULL;

      memcpy(&p_multi_string->ISO_639_language_code, p_buf, 3);
      i_number_segments = p_buf[3];

      p_segment = &p_buf[4];
      for (j=0; j<i_number_segments; j++)
      {
        multi_string_segment_t* p_multi_string_segment;

        p_multi_string_segment = (multi_string_segment_t*)calloc(1, sizeof(multi_string_segment_t));
        if (p_multi_string_segment == NULL)
          return NULL;

        p_multi_string_segment->i_compression_type = p_segment[0];
        p_multi_string_segment->i_mode = p_segment[1];
        p_multi_string_segment->i_number_bytes = p_segment[2];

        if (p_multi_string_segment->i_number_bytes > 0)
        {
          p_multi_string_segment->compressed_string = (u_int8_t*)calloc(1, p_multi_string_segment->i_number_bytes + 1);
          memcpy(p_multi_string_segment->compressed_string, &p_segment[3], p_multi_string_segment->i_number_bytes);
        }

        if (p_multi_string->p_first_segment == NULL)
          p_multi_string->p_first_segment = p_multi_string_segment;
        else
        {
          multi_string_segment_t* p_last_segment = p_multi_string->p_first_segment;
            while (p_last_segment->p_next != NULL)
              p_last_segment = p_last_segment->p_next;
            p_last_segment->p_next = p_multi_string_segment;
        }

        p_segment += 3 + p_multi_string_segment->i_number_bytes;

        i_number_bytes_segments += 3 + p_multi_string_segment->i_number_bytes;
      }

      if (*p_multi_string_head != p_multi_string)
      {
        multi_string_t* p_last_multi_string = (*p_multi_string_head)->p_next;
        while(p_last_multi_string->p_next != NULL)
          p_last_multi_string = p_last_multi_string->p_next;

        p_last_multi_string->p_next = p_last_multi_string;
      }

      p_buf += 4 + i_number_bytes_segments;
    }

    return *p_multi_string_head;
}

/*****************************************************************************
 * dvbpsi_DeleteMultiString
 *****************************************************************************
 * Free multiple string structure
 *****************************************************************************/
void dvbpsi_DeleteMultiString(multi_string_t* p_multi_string)
{
  multi_string_segment_t* p_segment;

    if (p_multi_string == NULL)
      return;

    p_segment = p_multi_string->p_first_segment;

    while(p_segment != NULL)
    {
        multi_string_segment_t* p_next = p_segment->p_next;

        if (p_segment->compressed_string != NULL)
          free(p_segment->compressed_string);

        free(p_segment);
        p_segment = p_next;
    }

    while(p_multi_string != NULL)
    {
      multi_string_t* p_next = p_multi_string->p_next;

      free(p_next);
      p_multi_string = p_next;
    }
}
