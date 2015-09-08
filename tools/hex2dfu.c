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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "constants.h"
#include "dfu.h"
#include "utils.h"

typedef struct {
  char *input_file;
  uint32_t lower_address;
  uint32_t upper_address;
  uint16_t vendor_id;
  uint16_t product_id;
  bool help;
  bool force;
} Options;

typedef enum {
  DATA = 0,
  END_OF_FILE = 1,
  EXTENDED_SEGMENT_ADDRESS = 2,
  START_SEGMENT_ADDRESS = 3,
  EXTENDED_LINEAR_ADDRESS = 4,
  START_LINEAR_ADDRESS = 5
} RecordType;

typedef struct {
  uint8_t byte_count;
  uint16_t address;
  uint8_t record_type;
  uint8_t *data;
} HexRecord;

static const uint32_t DEFAULT_LOWER_ADDRESS = 0x1d007000;
static const uint32_t DEFAULT_UPPER_ADDRESS = 0x1d07ffff;
static const char HEX_SUFFIX[] = ".hex";
static const char DFU_SUFFIX[] = ".dfu";

uint8_t *g_data = NULL;
unsigned int g_data_size = 0;

/**
 * @brief Convert a pair of characters to a byte.
 */
bool HexToUInt8(const char *str, uint8_t *output) {
  *output = 0;
  if (!isxdigit(str[0]) || !isxdigit(str[1])) {
    return false;
  }

  *output = (digittoint(str[0]) * 16) + digittoint(str[1]);
  return true;
}

/**
 * @brief Convert 4 characters to a uint16_t.
 */
bool HexToUInt16(const char *str, uint16_t *output) {
  uint8_t upper, lower;
  if (!HexToUInt8(str, &upper) || !HexToUInt8(str + 2, &lower)) {
    return false;
  }
  *output = (upper << 8) + lower;
  return true;
}

/**
 * @brief Process a block of data at the given address.
 */
void ProcessData(uint32_t address, const uint8_t *data, uint8_t size,
                 const Options *options) {
  if (address < options->lower_address || address > options->upper_address) {
    return;
  }

  unsigned int offset = address - options->lower_address;
  memcpy(g_data + offset, data, size);

  if (offset + size> g_data_size) {
    g_data_size = offset + size;
  }
}

/*
 * @brief Process a hex record.
 * @returns true if we should continue processing the hex file, false if we
 * should stop.
 */
bool ProcessRecord(const HexRecord *record, unsigned int line,
                   const Options *options) {
  static uint16_t upper_address = 0;

  switch (record->record_type) {
    case DATA:
      ProcessData((upper_address << 16) + record->address, record->data,
                  record->byte_count, options);
     break;
    case END_OF_FILE:
      if (record->byte_count != 0) {
        printf("Line %d contains END_OF_FILE with non-0 byte count\n", line);
        return false;
      }
      return false;  // stop processing
    case EXTENDED_LINEAR_ADDRESS:
      if (record->byte_count != 2) {
        printf("Line %d contains EXTENDED_LINEAR_ADDRESS without 2 data "
               "bytes\n", line);
        return false;
      }
      upper_address = (record->data[0] << 8) + record->data[1];
      break;
    default:
      return true;
  }
  return true;
}

void ProcessHexFile(int fd, const Options *options) {
  typedef struct {
    char start_code;  // must be ';'
    char byte_count[2];
    char address[4];
    char record_type[2];
  } HexRecordHeader;

  unsigned int line = 0;

  while (1) {
    line++;
    uint8_t header_data[sizeof(HexRecordHeader)];
    ssize_t r = read(fd, &header_data, sizeof(HexRecordHeader));
    if (r != sizeof(HexRecordHeader)) {
      printf("Failed to read hex header from line %d\n", line);
      return;
    }

    const HexRecordHeader *header = (HexRecordHeader*) header_data;
    if (header->start_code != ':') {
      printf("Invalid start code '%c' on line %d\n", header->start_code,
             line);
      return;
    }

    uint8_t byte_count;
    if (!HexToUInt8(header->byte_count, &byte_count)) {
      printf("Invalid data size on line %d\n", line);
      return;
    }

    uint16_t address;
    if (!HexToUInt16(header->address, &address)) {
      printf("Invalid address on line %d\n", line);
      return;
    }

    uint8_t record_type;
    if (!HexToUInt8(header->record_type, &record_type)) {
      printf("Invalid record type on line %d\n", line);
      return;
    }

    unsigned int hex_data_size = 2 * byte_count;
    char hex_data[hex_data_size];  // NOLINT(runtime/arrays)
    r = read(fd, &hex_data, hex_data_size);
    if (r != hex_data_size) {
      printf("Failed to read %d bytes on line %d\n", hex_data_size, line);
      return;
    }

    uint8_t calculated_checksum = (
        byte_count + (address >> 8) + address + record_type);

    // convert hex data to actual data
    uint8_t data[byte_count];  // NOLINT(runtime/arrays)
    for (unsigned int i = 0; i < byte_count; i++) {
      if (!HexToUInt8(hex_data + (i * 2), (uint8_t*) &data + i)) {
        printf("Invalid data on line %d\n", line);
        return;
      }
      calculated_checksum += data[i];
    }

    // read the checksum & new line
    char checksum_data[3];
    r = read(fd, &checksum_data, sizeof(checksum_data));
    if (r != sizeof(checksum_data)) {
      printf("Failed to checksum on line %d\n", line);
      free(data);
      return;
    }

    uint8_t checksum;
    if (!HexToUInt8(checksum_data, &checksum)) {
      printf("Invalid checksum on line %d\n", line);
      free(data);
      return;
    }

    calculated_checksum = ((~calculated_checksum + 1) & 0xff);

    if (checksum != calculated_checksum) {
      printf("Incorrect checksum on line %d, read %x, was %x\n", line,
             checksum, calculated_checksum);
      return;
    }

    if (checksum_data[2] != '\n') {
      printf("Missing \\n on line %d\n", line);
      return;
    }

    HexRecord record = {
      .byte_count = byte_count,
      .address = address,
      .record_type = record_type,
      .data = data
    };
    if (!ProcessRecord(&record, line, options)) {
      return;
    }
  }
}

void DisplayHelpAndExit(const char *arg0, int exit_code) {
  printf("Usage: %s [options] <hex-file>\n", arg0);
  printf("  -h, --help   Show the help message\n");
  printf("  -l, --lower  The lower bound of the memory to extract, "
         "default 0x%x\n", DEFAULT_LOWER_ADDRESS);
  printf("  -p, --pid    The USB Product ID, default 0x%x\n",
         DEFAULT_PRODUCT_ID);
  printf("  -u, --upper  The upper bound of the memory to extract, "
         "default 0x%x\n", DEFAULT_UPPER_ADDRESS);
  printf("  -v, --vid    The USB Vendor ID, default 0x%x\n", DEFAULT_VENDOR_ID);
  exit(exit_code);
}

bool InitOptions(Options *options, int argc, char *argv[]) {
  options->input_file = NULL;
  options->lower_address = DEFAULT_LOWER_ADDRESS;
  options->upper_address = DEFAULT_UPPER_ADDRESS;
  options->vendor_id = DEFAULT_VENDOR_ID;
  options->product_id = DEFAULT_PRODUCT_ID;
  options->help = false;
  options->force = false;

  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"lower", required_argument, 0, 'l'},
      {"pid", required_argument, 0, 'p'},
      {"upper", required_argument, 0, 'u'},
      {"vid", required_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  int c;
  int option_index = 0;

  while (1) {
    c = getopt_long(argc, argv, "hl:p:u:v:", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
      case 0:
        break;
      case 'h':
        options->help = true;
        break;
      case 'l':
        if (!StringToUInt32(optarg, &options->lower_address)) {
          printf("Invalid lower address id\n");
          exit(EX_USAGE);
        }
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
      case 'u':
        if (!StringToUInt32(optarg, &options->upper_address)) {
          printf("Invalid upper address id\n");
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

  if (optind == argc) {
    printf("Missing input file\n");
    exit(EX_USAGE);
  }

  if (options->upper_address <= options->lower_address) {
    printf("Upper address must be greater than lower address\n");
    exit(EX_USAGE);
  }

  unsigned int data_size = options->upper_address - options->lower_address;
  if (data_size > (1 << 19) && options->force == false) {
    printf("Memory size is large: %d, pass --force to override\n", data_size);
    exit(EX_USAGE);
  }

  options->input_file = argv[optind];
  return true;
}

int main(int argc, char *argv[]) {
  Options options;
  if (!InitOptions(&options, argc, argv)) {
    return EX_USAGE;
  }

  // Setup the output file path, and make sure the input file ends in
  // HEX_SUFFIX.
  char output_file[strlen(options.input_file) + 1];  // NOLINT(runtime/arrays)
  strncpy(output_file, options.input_file, strlen(options.input_file) + 1);

  char *ext = strrchr(output_file, '.');
  if (ext == NULL || strlen(ext) != strlen(HEX_SUFFIX) ||
      strncmp(ext, HEX_SUFFIX, strlen(HEX_SUFFIX)) != 0) {
    printf("Input file does not end in .hex\n");
    return EX_USAGE;
  }
  strncpy(ext, DFU_SUFFIX, strlen(DFU_SUFFIX));

  int fd = open(options.input_file, O_RDONLY);
  if (fd < 0) {
    printf("Failed to open %s: %s\n", options.input_file, strerror(errno));
    return EX_USAGE;
  }

  unsigned int data_size = options.upper_address - options.lower_address;
  g_data = malloc(data_size);
  memset(g_data, 0xff, data_size);  // set everything to 0xff
  ProcessHexFile(fd, &options);

  if (g_data_size) {
    FirmwareOptions fw_options = {
      .vendor_id = options.vendor_id,
      .product_id = options.product_id
    };
    WriteDFUFile(&fw_options, g_data, g_data_size, output_file);
  }
  free(g_data);
  g_data = NULL;

  off_t current = lseek(fd, 0, SEEK_CUR);
  off_t end = lseek(fd, 0, SEEK_END);
  if (current != end) {
    printf("%zd bytes remain in hex file\n", end - current);
  }
  close(fd);
}
