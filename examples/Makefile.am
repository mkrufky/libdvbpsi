## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = decode_pat decode_pmt decode_sdt decode_mpeg

decode_pat_SOURCES = decode_pat.c
decode_pat_LDFLAGS = -L../src -ldvbpsi

decode_pmt_SOURCES = decode_pmt.c
decode_pmt_LDFLAGS = -L../src -ldvbpsi -lm

decode_sdt_SOURCES = decode_sdt.c
decode_sdt_LDFLAGS = -L../src -ldvbpsi

decode_mpeg_SOURCES = connect.c decode_mpeg.c
decode_mpeg_LDFLAGS = -L../src -ldvbpsi -lm

