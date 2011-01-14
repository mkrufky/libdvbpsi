/*****************************************************************************
 * dvbpsi_private.h: main private header
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#ifndef _DVBPSI_DVBPSI_PRIVATE_H_
#define _DVBPSI_DVBPSI_PRIVATE_H_

#include <stdarg.h>

extern uint32_t dvbpsi_crc32_table[];

/*****************************************************************************
 * Error management
 *
 * libdvbpsi messages have the following format:
 * "libdvbpsi [error | warning | debug] (<component>): <msg>"
 *****************************************************************************/
#define DVBPSI_MSG_NONE   -1 /* No messages */
#define DVBPSI_MSG_ERROR   0 /* Error messages, only */
#define DVBPSI_MSG_WARNING 1 /* Error and Warning messages */
#define DVBPSI_MSG_DEBUG   2 /* Error, warning and debug messages */

#ifdef HAVE_VARIADIC_MACROS
void message(dvbpsi_handle dvbpsi, const int level, const char *fmt, ...);

#  define dvbpsi_error(hnd, src, str, x...)                             \
        message(hnd, DVBPSI_MSG_ERROR, "libdvbpsi error ("src"): " str, x)
#  define dvbpsi_warning(hnd, str, x...)                                \
        message(hnd, DVBPSI_MSG_WARNING, "libdvbpsi warning ("src"): " str, x)
#  define dvbpsi_debug(hnd, str, x...)                                  \
        message(hnd, DVBPSI_MSG_DEBUG, "libdvbpsi debug ("src"): " str, x)
#else
void dvbpsi_error(dvbpsi_handle dvbpsi, const char *src, const char *fmt, ...);
void dvbpsi_warning(dvbpsi_handle dvbpsi, const char *src, const char *fmt, ...);
void dvbpsi_debug(dvbpsi_handle dvbpsi, const char *src, const char *fmt, ...);
#endif

#else
#error "Multiple inclusions of dvbpsi_private.h"
#endif

