#!/bin/bash
#
# Use as: $0 > src/descriptors/dr.h
#
dr_h="src/descriptors/dr.h"
dr=`ls src/descriptors/*.h`

# check directory
if ! test -d `dirname $dr_h`; then
	echo "Directory `dirname $dr_h` does not exist!!."
	echo "Are you sure to be in the libdvbpsi top directory?"
	exit 1;
fi

echo "/*****************************************************************************"
echo " * dr.h"
echo " * Copyright (C) 2001-2010 VideoLAN"
echo " *"
echo " * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>"
echo " *"
echo " * This library is free software; you can redistribute it and/or"
echo " * modify it under the terms of the GNU Lesser General Public"
echo " * License as published by the Free Software Foundation; either"
echo " * version 2.1 of the License, or (at your option) any later version."
echo " *"
echo " * This library is distributed in the hope that it will be useful,"
echo " * but WITHOUT ANY WARRANTY; without even the implied warranty of"
echo " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU"
echo " * Lesser General Public License for more details."
echo " *"
echo " * You should have received a copy of the GNU Lesser General Public"
echo " * License along with this library; if not, write to the Free Software"
echo " * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA"
echo " *"
echo " *****************************************************************************/"
echo ""
echo "/*!"
echo " * \file <dr.h>"
echo " * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>"
echo " * \brief Gather all dr_*.h into one."
echo " *"
echo " * Gathers all dr_*.h into one. Use this header if you need a lot of them."
echo " */"
echo ""
echo "#ifndef _DVBPSI_DR_H_"
echo "#define _DVBPSI_DR_H_"
echo ""

for h in $dr; do
	f=`basename $h`
	if ! test "$f" = "dr.h"; then
   		echo "#include \"$f\""
	fi
done

echo ""
echo "#else"
echo "#error \"Multiple inclusions of dr.h\""
echo "#endif"

exit 0
