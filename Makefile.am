## Process this file with automake to produce Makefile.in

SUBDIRS = src examples misc
DIST_SUBDIRS = doc debian

EXTRA_DIST = libdvbpsi.spec bootstrap

AUTOMAKE_OPTIONS = foreign dist-bzip2 subdir-objects

doc-dummy:

doc: doc-dummy
	$(MAKE) -C doc doc

test_dr.c:
	$(MAKE) -C misc test_dr.c

