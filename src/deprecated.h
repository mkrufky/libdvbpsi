/*****************************************************************************
 * deprecated.h
 * Copyright (C) 2012 VideoLAN
 * $Id$
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
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

#ifndef _DVBPSI_DEPRECATED_H_
#define _DVBPSI_DEPRECATED_H_

/*!
 * \file <deprecated.h>
 * \author Jean-Paul Saman <jpsaman@videolan.org>
 * \brief Deprecated Application interface for all DVB/PSI decoders.
 *
 * The application interface for DVBPSI version < 1.0.0 has changed.
 * The affected functions are mentioned in this headerfile. Developers
 * are urged to use the current application interface instead. It offers
 * the same functionality and a bit more.
 */

#ifdef __cplusplus
extern "C" {
#endif


/* dvbpsi_private.h */
/* descriptor.h */
/* psi.h */

/* demux.h */

/*****************************************************************************
 * Decoders
 *****************************************************************************/



/*****************************************************************************
 * Descriptors
 *****************************************************************************/

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of deprecated.h"
#endif
