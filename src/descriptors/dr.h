/*****************************************************************************
 * dr.h
 * (c)2001-2002 VideoLAN
 * $Id: dr.h,v 1.2 2002/05/10 22:58:53 bozo Exp $
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
 *****************************************************************************/

/*!
 * \file <dr.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Gather all dr_*.h into one.
 *
 * Gathers all dr_*.h into one. Use this header if you need a lot of them.
 */

#ifndef _DVBPSI_DR_H_
#define _DVBPSI_DR_H_

#include "../src/descriptors/dr_02.h"
#include "../src/descriptors/dr_03.h"
#include "../src/descriptors/dr_04.h"
#include "../src/descriptors/dr_05.h"
#include "../src/descriptors/dr_06.h"
#include "../src/descriptors/dr_07.h"
#include "../src/descriptors/dr_08.h"
#include "../src/descriptors/dr_09.h"
#include "../src/descriptors/dr_0a.h"
#include "../src/descriptors/dr_0b.h"
#include "../src/descriptors/dr_0c.h"
#include "../src/descriptors/dr_0d.h"
#include "../src/descriptors/dr_0e.h"
#include "../src/descriptors/dr_0f.h"

#else
#error "Multiple inclusions of dr.h"
#endif

