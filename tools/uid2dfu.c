/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2015 Simon Newton.
 */

#include <arpa/inet.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "constants.h"
#include "dfu.h"
#include "utils.h"

typedef struct {
  const char *output_file;
  uint16_t manufacturer_id;
  uint16_t product_id;
  uint16_t vendor_id;
  uint32_t device_id;
  bool help;
} Options;

static const char DEFAULT_FILE[] = "uid.dfu";

void DisplayHelpAndExit(const char *arg0, int exit_code) {
  printf("Usage: %s [options] -m <manufacturer-id> -d <device-id>\n", arg0);
  printf("  -d, --device <id>  The device ID\n");
  printf("  -h, --help   Show the help message\n");
  printf("  -m, --manufacturer <id>  The manufacturer ID\n");
  printf("  -o, --output Output file, default to uid.dfu\n");
  printf("  -p, --pid    The USB Product ID, default 0x%x\n",
         DEFAULT_PRODUCT_ID);
  printf("  -v, --vid    The USB Vendor ID, default 0x%x\n", DEFAULT_VENDOR_ID);
  exit(exit_code);
}

bool InitOptions(Options *options, int argc, char *argv[]) {
  options->output_file = DEFAULT_FILE;
  options->manufacturer_id = 0;
  options->device_id = 0;
  options->help = false;
  options->vendor_id = DEFAULT_VENDOR_ID;
  options->product_id = DEFAULT_PRODUCT_ID;

  bool got_device = false;
  bool got_manufacturer = false;

  static struct option long_options[] = {
      {"device", required_argument, 0, 'd'},
      {"help", no_argument, 0, 'h'},
      {"manufacturer", required_argument, 0, 'm'},
      {"output", required_argument, 0, 'o'},
      {"pid", required_argument, 0, 'p'},
      {"vid", required_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  int c;
  int option_index = 0;

  while (1) {
    c = getopt_long(argc, argv, "d:hm:o:p:v:", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
      case 0:
        break;
      case 'd':
        if (!StringToUInt32(optarg, &options->device_id)) {
          printf("Invalid device id\n");
          exit(EX_USAGE);
        }
        got_device = true;
        break;
      case 'h':
        options->help = true;
        break;
      case 'm':
        if (!StringToUInt16(optarg, &options->manufacturer_id)) {
          printf("Invalid manufacturer id\n");
          exit(EX_USAGE);
        }
        got_manufacturer = true;
        break;
      case 'o':
        options->output_file = optarg;
        break;
      case 'p':
        if (!StringToUInt16(optarg, &options->product_id)) {
          printf("Invalid product id\n");
          exit(EX_USAGE);
        }
        break;
      case 'v':
        if (!StringToUInt16(optarg, &options->vendor_id)) {
          printf("Invalid vendor id\n");
          exit(EX_USAGE);
        }
        break;
      default:
        {}
    }
  }

  if (options->help) {
    DisplayHelpAndExit(argv[0], 0);
  }

  if (!got_device) {
    printf("Missing device ID\n");
    exit(EX_USAGE);
  }

  if (!got_manufacturer) {
    printf("Missing manufacturer ID\n");
    exit(EX_USAGE);
  }
  return true;
}

int main(int argc, char *argv[]) {
  Options options;
  if (!InitOptions(&options, argc, argv)) {
    return EX_USAGE;
  }

  struct UIDData {
    uint16_t manufacturer_id;
    uint32_t device_id;
  } __attribute__((__packed__));

  struct UIDData uid_data = {
    .manufacturer_id = htons(options.manufacturer_id),
    .device_id = htonl(options.device_id)
  };

  printf("UID: %04x:%08x\n", options.manufacturer_id, options.device_id);

  FirmwareOptions fw_options = {
    .vendor_id = options.vendor_id,
    .product_id = options.product_id
  };
  WriteDFUFile(&fw_options, (uint8_t*) &uid_data, sizeof(uid_data),
               options.output_file);
}
