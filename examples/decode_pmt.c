/*****************************************************************************
 * decode_pmt.c: PAT decoder example
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: decode_pmt.c,v 1.3 2002/10/07 14:15:14 sam Exp $
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
#include <unistd.h>
#include <fcntl.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

/* The libdvbpsi distribution defines DVBPSI_DIST */
#ifdef DVBPSI_DIST
#include "../src/dvbpsi.h"
#include "../src/psi.h"
#include "../src/descriptor.h"
#include "../src/tables/pmt.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/psi.h>
#include <dvbpsi/descriptor.h>
#include <dvbpsi/pmt.h>
#endif


/*****************************************************************************
 * ReadPacket
 *****************************************************************************/
int ReadPacket(int i_fd, uint8_t* p_dst)
{
  int i = 187;
  int i_rc = 1;

  p_dst[0] = 0;

  while((p_dst[0] != 0x47) && (i_rc > 0))
  {
    i_rc = read(i_fd, p_dst, 1);
  }

  while((i != 0) && (i_rc > 0))
  {
    i_rc = read(i_fd, p_dst + 188 - i, i);
    if(i_rc >= 0)
      i -= i_rc;
  }

  return (i == 0) ? 1 : 0;
}


/*****************************************************************************
 * DumpDescriptors
 *****************************************************************************/
void DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
  while(p_descriptor)
  {
    int i;
    printf("%s 0x%02x : \"", str, p_descriptor->i_tag);
    for(i = 0; i < p_descriptor->i_length; i++)
      printf("%c", p_descriptor->p_data[i]);
    printf("\"\n");
    p_descriptor = p_descriptor->p_next;
  }
};


/*****************************************************************************
 * DumpPMT
 *****************************************************************************/
void DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt)
{
  dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
  printf(  "\n");
  printf(  "New active PMT\n");
  printf(  "  program_number : %d\n",
         p_pmt->i_program_number);
  printf(  "  version_number : %d\n",
         p_pmt->i_version);
  printf(  "  PCR_PID        : 0x%x (%d)\n",
         p_pmt->i_pcr_pid, p_pmt->i_pcr_pid);
  DumpDescriptors("    ]", p_pmt->p_first_descriptor);
  printf(  "    | type @ elementary_PID\n");
  while(p_es)
  {
    printf("    | 0x%02x @ 0x%x (%d)\n",
           p_es->i_type, p_es->i_pid, p_es->i_pid);
    DumpDescriptors("    |  ]", p_es->p_first_descriptor);
    p_es = p_es->p_next;
  }
  dvbpsi_DeletePMT(p_pmt);
}


/*****************************************************************************
 * main
 *****************************************************************************/
int main(int i_argc, char* pa_argv[])
{
  int i_fd;
  uint8_t data[188];
  dvbpsi_handle h_dvbpsi;
  int b_ok;
  uint16_t i_program_number, i_pmt_pid;

  if(i_argc != 4)
    return 1;

  i_fd = open(pa_argv[1], 0);
  i_program_number = atoi(pa_argv[2]);
  i_pmt_pid = atoi(pa_argv[3]);

  h_dvbpsi = dvbpsi_AttachPMT(i_program_number, DumpPMT, NULL);

  b_ok = ReadPacket(i_fd, data);

  while(b_ok)
  {
    uint16_t i_pid = ((uint16_t)(data[1] & 0x1f) << 8) + data[2];
    if(i_pid == i_pmt_pid)
      dvbpsi_PushPacket(h_dvbpsi, data);
    b_ok = ReadPacket(i_fd, data);
  }

  dvbpsi_DetachPMT(h_dvbpsi);

  return 0;
}

