/*****************************************************************************
 * psi_private.h: common private PSI structures
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: psi_private.h,v 1.1 2002/01/07 18:30:35 bozo Exp $
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

#ifndef _DVBPSI_PSI_PRIVATE_H_
#define _DVBPSI_PSI_PRIVATE_H_


/*****************************************************************************
 * dvbpsi_ValidPSISection
 *****************************************************************************
 * Check the CRC_32 if the section has b_syntax_indicator set.
 *****************************************************************************/
int dvbpsi_ValidPSISection(dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_BuildPSISection
 *****************************************************************************
 * Build the section based on the information in the structure.
 *****************************************************************************/
void dvbpsi_BuildPSISection(dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of psi_private.h"
#endif

