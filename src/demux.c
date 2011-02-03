/*****************************************************************************
 * demux.c: DVB subtables demux functions.
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"
#include "demux.h"

/*****************************************************************************
 * dvbpsi_AttachDemux
 *****************************************************************************
 * Creation of the demux structure
 *****************************************************************************/
dvbpsi_t *dvbpsi_AttachDemux(dvbpsi_t *            p_dvbpsi,
                             dvbpsi_demux_new_cb_t pf_new_cb,
                             void *                p_new_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t*)malloc(sizeof(dvbpsi_demux_t));
    if (p_demux == NULL)
        return NULL;

    /* PSI decoder configuration */
    p_demux->pf_callback = &dvbpsi_Demux;
    p_demux->i_section_max_size = 4096;

    /* PSI decoder initial state */
    p_demux->i_continuity_counter = 31;
    p_demux->b_discontinuity = 1;
    p_demux->p_current_section = NULL;

    /* Subtables demux configuration */
    p_demux->p_first_subdec = NULL;
    p_demux->pf_new_callback = pf_new_cb;
    p_demux->p_new_cb_data = p_new_cb_data;

    p_dvbpsi->p_private = (void *)p_demux;
    return p_dvbpsi;
}

/*****************************************************************************
 * dvbpsi_demuxGetSubDec
 *****************************************************************************
 * Finds a subtable decoder given the table id and extension
 *****************************************************************************/
dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t * p_demux,
                                              uint8_t i_table_id,
                                              uint16_t i_extension)
{
    uint32_t i_id = (uint32_t)i_table_id << 16 |(uint32_t)i_extension;
    dvbpsi_demux_subdec_t * p_subdec = p_demux->p_first_subdec;

    while (p_subdec)
    {
        if (p_subdec->i_id == i_id)
            break;

        p_subdec = p_subdec->p_next;
    }

    return p_subdec;
}

/*****************************************************************************
 * dvbpsi_Demux
 *****************************************************************************
 * Sends a PSI section to the right subtable decoder
 *****************************************************************************/
void dvbpsi_Demux(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t * p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_private;
    dvbpsi_demux_subdec_t * p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
                                                             p_section->i_extension);

    if (p_subdec == NULL)
    {
        /* Tell the application we found a new subtable, so that it may attach a
         * subtable decoder */
        p_demux->pf_new_callback(p_demux->p_new_cb_data, p_dvbpsi,
                                 p_section->i_table_id, p_section->i_extension);

        /* Check if a new subtable decoder is available */
        p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
                                         p_section->i_extension);
    }

    if (p_subdec)
    {
        p_subdec->pf_callback(p_dvbpsi, p_subdec->p_cb_data, p_section);
    }
    else
    {
        dvbpsi_DeletePSISections(p_section);
    }
}

/*****************************************************************************
 * dvbpsi_DetachDemux
 *****************************************************************************
 * Destroys a demux structure
 *****************************************************************************/
void dvbpsi_DetachDemux(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_private);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_private;
    dvbpsi_demux_subdec_t* p_subdec = p_demux->p_first_subdec;

    while (p_subdec)
    {
        dvbpsi_demux_subdec_t* p_subdec_temp = p_subdec;
        p_subdec = p_subdec->p_next;

        if (p_subdec_temp->pf_detach)
            p_subdec_temp->pf_detach(p_dvbpsi, (p_subdec_temp->i_id >> 16) & 0xFFFF,
                                     p_subdec_temp->i_id & 0xFFFF);
        else free(p_subdec_temp);
    }

    if (p_demux->p_current_section)
        dvbpsi_DeletePSISections(p_demux->p_current_section);

    p_dvbpsi->p_private = NULL;
    free(p_demux);
}
