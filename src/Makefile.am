## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

lib_LTLIBRARIES = libdvbpsi.la

libdvbpsi_la_SOURCES = dvbpsi.c psi.c pat.c descriptor.c pmt.c
libdvbpsi_la_LDFLAGS = -release @VERSION@

noinst_HEADERS = dvbpsi_private.h psi_private.h pat_private.h pmt_private.h
pkginclude_HEADERS = dvbpsi.h psi.h pat.h descriptor.h pmt.h

