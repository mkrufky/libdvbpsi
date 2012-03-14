/*****************************************************************************
 * dvbpsi.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Jean-Paul Saman <jpsaman@videolan.org>
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
 * \file <dvbpsi.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for all DVB/PSI decoders.
 *
 * Application interface for all DVB/PSI decoders. The generic decoder
 * structure is public so that external decoders are allowed.
 */

#ifndef _DVBPSI_DVBPSI_H_
#define _DVBPSI_DVBPSI_H_

#define DVBPSI_VERSION      1.0.0              /*!< Human readible DVBPSI version*/
#define DVBPSI_VERSION_INT  ((1<<16)+(0<<8)+0) /*!< Machine readible DVBPSI version */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_handle
 *****************************************************************************/
/*!
 * \typedef struct dvbpsi_s dvbpsi_t
 * \brief DVBPSI handle structure abstration.
 */
typedef struct dvbpsi_s dvbpsi_t;

/*!
 * \enum dvbpsi_msg_level
 * \brief DVBPSI message level enum
 */
typedef enum dvbpsi_msg_level
{
    DVBPSI_MSG_NONE  = -1, /*!< No messages */
    DVBPSI_MSG_ERROR =  0, /*!< Error messages only */
    DVBPSI_MSG_WARN  =  1, /*!< Error and Warning messages */
    DVBPSI_MSG_DEBUG =  2, /*!< Error, warning and debug messages */
} dvbpsi_msg_level_t;

/*****************************************************************************
 * dvbpsi_message_cb
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_message_cb)(dvbpsi_handle p_decoder,
 *                                     const dvbpse_msg_level_t level,
 *                                     const char* msg)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_message_cb)(dvbpsi_t *handle,
                                   const dvbpsi_msg_level_t level,
                                   const char* msg);

/*****************************************************************************
 * dvbpsi_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_s
 * \brief DVBPSI handle structure
 *
 * This structure provides a handle to libdvbpsi API and should be used instead
 * of dvbpsi_decoder_t.
 */
/*!
 * \typedef struct dvbpsi_s dvbpsi_t
 * \brief dvbpsi_t type definition.
 */
struct dvbpsi_s
{
    void                         *p_private;            /*!< private pointer to
                                                          specific decoder or
                                                          encoder */
    /* Messages callback */
    dvbpsi_message_cb             pf_message;           /*!< Log message callback */
    enum dvbpsi_msg_level         i_msg_level;          /*!< Log level */
};

/*****************************************************************************
 * dvbpsi_NewHandle
 *****************************************************************************/
/*!
 * \fn dvbpsi_t *dvbpsi_NewHandle(dvbpsi_message_cb callback, enum dvbpsi_msg_level level)
 * \brief Create a new dvbpsi_t handle to be used by PSI decoders or encoders
 * \param callback message callback handler, if NULL then no errors, warnings
 *        or debug messages will be sent to the caller application
 * \param level enum dvbpsi_msg_level for filtering logging messages
 * \return pointer to dvbpsi_t malloced data
 *
 * Creates a handle to use with PSI decoder and encoder API functions. The
 * handle must be freed with dvbpsi_DeleteHandle().
 */
dvbpsi_t *dvbpsi_NewHandle(dvbpsi_message_cb callback, enum dvbpsi_msg_level level);

/*****************************************************************************
 * dvbpsi_DeleteHandle
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DeleteHandle(dvbpsi_t *handle)
 * \brief Deletes a dvbpsi_t handle created with dvbpsi_NewHandle
 * \param handle pointer to dvbpsi_t malloced data
 * \return nothing
 *
 * Delets a dvbpsi_t handle by calling free(handle). Make sure to detach any
 * decoder of encoder before deleting the dvbpsi handle.
 */
void dvbpsi_DeleteHandle(dvbpsi_t *handle);

/*****************************************************************************
 * dvbpsi_PushPacket
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_PushPacket(dvbpsi_t *p_dvbpsi, uint8_t* p_data)
 * \brief Injection of a TS packet into a PSI decoder.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_data pointer to a 188 bytes playload of a TS packet
 * \return true when packet has been handled, false on error.
 *
 * Injection of a TS packet into a PSI decoder.
 */
bool dvbpsi_PushPacket(dvbpsi_t *p_dvbpsi, uint8_t* p_data);

/*****************************************************************************
 * The following definitions are just here to allow external decoders but
 * shouldn't be used for any other purpose.
 *****************************************************************************/

/*!
 * \typedef struct dvbpsi_psi_section_s dvbpsi_psi_section_t
 * \brief dvbpsi_psi_section_t type definition.
 */
typedef struct dvbpsi_psi_section_s dvbpsi_psi_section_t;

/*!
 * \typedef struct dvbpsi_decoder_s dvbpsi_decoder_t
 * \brief dvbpsi_decoder_t type definition.
 */
typedef struct dvbpsi_decoder_s dvbpsi_decoder_t;

/*****************************************************************************
 * dvbpsi_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_callback)(dvbpsi_t *p_dvbpsi,
                                     dvbpsi_psi_section_t* p_section)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_callback)(dvbpsi_t *p_dvbpsi,
                                 dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_decoder_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_decoder_s
 * \brief PSI decoder structure.
 *
 * This structure shouldn't be used but if you want to write an external
 * decoder.
 */
#define DVBPSI_DECODER_COMMON                                                     \
    dvbpsi_callback  pf_callback;  /*!< PSI decoder's callback */                 \
    int      i_section_max_size;   /*!< Max size of a section for this decoder */ \
    uint8_t  i_continuity_counter; /*!< Continuity counter */                     \
    bool     b_discontinuity;      /*!< Discontinuity flag */                     \
    dvbpsi_psi_section_t *p_current_section; /*!< Current section */              \
    int      i_need;               /*!< Bytes needed */                           \
    bool     b_complete_header;    /*!< Flag for header completion */

struct dvbpsi_decoder_s
{
    DVBPSI_DECODER_COMMON
};

/*****************************************************************************
 * dvbpsi_NewDecoder
 *****************************************************************************/
/*!
 * \fn dvbpsi_decoder_t *dvbpsi_NewDecoder(dvbpsi_t *p_dvbpsi, dvbpsi_callback *callback,
 *     const int i_section_max_size, const bool b_discontinuity, const size_t struct_size);
 * \brief Create a new dvbpsi_decoder_t.
 * \param callback dvbpsi_callback handler
 * \param i_section_max_size Max size of a section for this decoder
 * \param b_discontinuity Discontinuity flag
 * \param psi_size size of new PSI struct, eg: sizeof(dvbpsi_pat_t)
 * \return pointer to dvbpsi_decoder_t&
 *
 * Creates a dvbpsi_decoder_t pointer to struct dvbpsi_decoder_s. It should be
 * delete with dvbpsi_DeleteDecoder() function.
 */
dvbpsi_decoder_t *dvbpsi_NewDecoder(dvbpsi_callback callback,
    const int i_section_max_size, const bool b_discontinuity, const size_t psi_size);

/*****************************************************************************
 * dvbpsi_DeleteDecoder
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DeleteDecoder(dvbpsi_decoder_t *p_decoder);
 * \brief Deletes decoder struct and frees its memory
 * \param p_decoder pointer to dvbpsi_decoder_t with decoder
 * \return nothing
 *
 * Delets a dvbpsi_t handle by calling free(handle). Make sure to detach any
 * decoder of encoder before deleting the dvbpsi handle.
 */
void dvbpsi_DeleteDecoder(dvbpsi_decoder_t *p_decoder);

/*****************************************************************************
 * dvbpsi_HasDecoder
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_HasDecoder(dvbpsi_t *p_dvbpsi);
 * \brief Determines if a decoder has been attached to dvbpsi_t handle
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \return true when decoder is attached, else it will return false.
 *
 * Determines if a decoder is attached to this dvbpsi_t handle. When the dvbpsi
 * handle is invalid the fuction will return false.
 */
bool dvbpsi_HasDecoder(dvbpsi_t *p_dvbpsi);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dvbpsi.h"
#endif
