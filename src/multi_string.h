/*****************************************************************************
 * multi_string.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: multi_string.h,v 1.5 2002/05/08 13:00:40 bozo Exp $
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
 *****************************************************************************/

/*!
 * \file <multi_string.h>
 * \author Yonathan Yusim <yonathan@boxee.tv>
 * \brief Common multi_string tools.
 *
 * Multi string structure and its Manipulation tools.
 *
 * NOTE: Descriptor generators and decoder functions return a pointer on success
 * and NULL on error. They do not use a dvbpsi_t handle as first argument.
 */

#ifndef _DVBPSI_MULTI_STRING_H_
#define _DVBPSI_MULTI_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * multi_string_segment_s
 *****************************************************************************/
/*!
 * \struct multi_string_segment_s
 * \brief Multi string segment structure.
 *
 * This structure is used to store a multi string.
 * (ISO/IEC 13818-1 section 2.6).
 */
/*!
 * \typedef struct multi_string_segment_s multi_string_segment_t
 * \brief multi_string_segment_t type definition.
 */
typedef struct multi_string_segment_s
{
  uint8_t                        i_compression_type;
  uint8_t                        i_mode;
  uint8_t                        i_number_bytes;
  uint8_t*                       compressed_string;

  struct multi_string_segment_s*  p_next;

} multi_string_segment_t;

/*****************************************************************************
 * multi_string_s
 *****************************************************************************/
/*!
 * \struct multi_string_s
 * \brief Multi string structure.
 *
 * This structure is used to store a multi string.
 * (ISO/IEC 13818-1 section 2.6).
 */
/*!
 * \typedef struct multi_string_s multi_string_t
 * \brief multi_string_segment_t type definition.
 */
typedef struct multi_string_s
{
  char                            ISO_639_language_code[3];

  multi_string_segment_t*         p_first_segment;

  struct multi_string_s*          p_next;

} multi_string_t;


/*****************************************************************************
 * dvbpsi_BuildMultiString
 *****************************************************************************
 * Build multiple string structure
 *****************************************************************************/
multi_string_t * dvbpsi_BuildMultiString(uint8_t* p_buf);

/*****************************************************************************
 * dvbpsi_DeleteMultiString
 *****************************************************************************
 * Free multiple string structure
 *****************************************************************************/
void dvbpsi_DeleteMultiString(multi_string_t* p_multi_string);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of muilti_string.h"
#endif
