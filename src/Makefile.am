## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

lib_LTLIBRARIES = libdvbpsi.la

libdvbpsi_la_SOURCES = dvbpsi.c dvbpsi_private.h \
                       psi.c \
		       demux.c \
                       descriptor.c \
                       $(tables_src) \
                       $(descriptors_src)

libdvbpsi_la_LDFLAGS = -version-info 3:0:0

pkginclude_HEADERS = dvbpsi.h psi.h descriptor.h demux.h \
                     tables/pat.h tables/pmt.h tables/sdt.h \
                     descriptors/dr_02.h \
                     descriptors/dr_03.h \
                     descriptors/dr_04.h \
                     descriptors/dr_05.h \
                     descriptors/dr_06.h \
                     descriptors/dr_07.h \
                     descriptors/dr_08.h \
                     descriptors/dr_09.h \
                     descriptors/dr_0a.h \
                     descriptors/dr_0b.h \
                     descriptors/dr_0c.h \
                     descriptors/dr_0d.h \
                     descriptors/dr_0e.h \
                     descriptors/dr_0f.h \
                     descriptors/dr_42.h \
                     descriptors/dr_47.h \
                     descriptors/dr_48.h \
                     descriptors/dr_59.h \
                     descriptors/dr.h

descriptors_src = descriptors/dr_02.c \
                  descriptors/dr_03.c \
                  descriptors/dr_04.c \
                  descriptors/dr_05.c \
                  descriptors/dr_06.c \
                  descriptors/dr_07.c \
                  descriptors/dr_08.c \
                  descriptors/dr_09.c \
                  descriptors/dr_0a.c \
                  descriptors/dr_0b.c \
                  descriptors/dr_0c.c \
                  descriptors/dr_0d.c \
                  descriptors/dr_0e.c \
                  descriptors/dr_0f.c \
                  descriptors/dr_42.c \
                  descriptors/dr_47.c \
                  descriptors/dr_48.c \
                  descriptors/dr_59.c

tables_src = tables/pat.c tables/pat_private.h \
             tables/pmt.c tables/pmt_private.h \
	     tables/sdt.c tables/sdt_private.h

