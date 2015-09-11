# Libraries
##################################################
noinst_LTLIBRARIES += tools/libdfu.la
tools_libdfu_la_SOURCES = tools/dfu.c \
                          tools/utils.c

# Programs
##################################################
noinst_PROGRAMS += tools/hex2dfu \
                   tools/uid2dfu

tools_hex2dfu_SOURCES = tools/hex2dfu.c
tools_hex2dfu_LDADD = tools/libdfu.la

tools_uid2dfu_SOURCES = tools/uid2dfu.c
tools_uid2dfu_LDADD = tools/libdfu.la
