## Process this file with automake to produce Makefile.in

pkgincludedir = $(includedir)/dvbpsi

SUBDIRS = tables

lib_LTLIBRARIES = libdvbpsi.la

libdvbpsi_la_SOURCES = dvbpsi.c psi.c descriptor.c
libdvbpsi_la_LIBADD = tables/lib_tables.la
libdvbpsi_la_LDFLAGS = -release @VERSION@

noinst_HEADERS = dvbpsi_private.h
pkginclude_HEADERS = dvbpsi.h psi.h descriptor.h

