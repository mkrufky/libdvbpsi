## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

lib_LTLIBRARIES = libdvbpsi.la

lib_descriptors_la_SOURCES = descriptors/dr_02.c \
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
                             descriptors/dr_0f.c

libdvbpsi_la_SOURCES = dvbpsi.c psi.c descriptor.c tables/pat.c tables/pmt.c $(lib_descriptors_la_SOURCES)
libdvbpsi_la_LDFLAGS = -version-info 1:0:0

noinst_HEADERS = dvbpsi_private.h tables/pat_private.h tables/pmt_private.h
pkginclude_HEADERS = dvbpsi.h psi.h descriptor.h tables/pat.h tables/pmt.h \
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
                     descriptors/dr.h

