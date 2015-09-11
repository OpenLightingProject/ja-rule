noinst_LTLIBRARIES += Bootloader/firmware/src/libbootloader.la

Bootloader_firmware_src_libbootloader_la_SOURCES = \
    Bootloader/firmware/src/bootloader.c
Bootloader_firmware_src_libbootloader_la_CFLAGS = $(BUILD_FLAGS)
