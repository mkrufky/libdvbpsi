/*****************************************************************************
 * gen_crc.c: CRC_32 table generator
 *----------------------------------------------------------------------------
 * (c)2001-2002 VideoLAN
 * $Id: gen_crc.c,v 1.1 2002/01/07 18:30:35 bozo Exp $
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


#include <stdio.h>
#include <inttypes.h>


/*****************************************************************************
 * main
 *****************************************************************************/
int main()
{
  uint32_t table[256];
  uint32_t i, j, k;

  for(i = 0; i < 256; i++)
  {
    k = 0;
    for (j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1)
      k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
    table[i] = k;
  }

  printf("static uint32_t table[256] =\n{\n");
  for(i = 0; i < 64; i++)
  {
    printf("  0x%08x, 0x%08x, 0x%08x, 0x%08x",
           table[i << 2 | 0],
           table[i << 2 | 1],
           table[i << 2 | 2],
           table[i << 2 | 3]);
    if(i < 63)
      printf(",\n");
    else
      printf("\n");
  }
  printf("};\n");

  return 0;
}

