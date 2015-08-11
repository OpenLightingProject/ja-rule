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
 * CRC polynomial 0xedb88320 – Contributed unknowingly by Gary S. Brown.
 * "Copyright (C) 1986 Gary S. Brown. You may use this program, or code or
 * tables extracted from it, as desired without restriction."
 *
 * The updcrc macro (referred to here as CalculateCRC) is derived from an
 * article Copyright 1986 by Stephen Satchell.
 *
 * Remainder portions of the code are Copyright (C) 2015 Simon Newton.
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
#include <unistd.h>

typedef struct {
  char *input_file;
  uint32_t lower_address;
  uint32_t upper_address;
  uint16_t vendor_id;
  uint16_t product_id;
  bool help;
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

static const uint32_t DEFAULT_LOWER_ADDRESS = 0x1d006000;
static const uint32_t DEFAULT_UPPER_ADDRESS = 0x1d07ffff;
static const char HEX_SUFFIX[] = ".hex";
static const char DFU_SUFFIX[] = ".dfu";

static const uint16_t DEFAULT_VENDOR_ID = 0x1209;
static const uint16_t DEFAULT_PRODUCT_ID = 0xacee;

// From the DFU 1.1 standard, see copyright above.
static const unsigned long CRC_POLYNOMIAL[] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint8_t *g_data = NULL;
unsigned int g_data_size = 0;

/**
 * @brief Calculate the DFU CRC.
 */
uint32_t CalculateCRC(uint32_t crc, uint8_t data) {
  return CRC_POLYNOMIAL[(crc ^ data) & 0xff] ^ (crc >> 8);
}

/**
 * @brief Convert a string to a uint16_t.
 * @returns true if the input was within range, false otherwise.
 *
 * The string can either be decimal or hex (prefixed with 0x).
 */
bool StringToUInt16(const char *input, uint16_t *output) {
  long i = strtol(input, NULL, 0);
  if (i < 0 || i > UINT16_MAX) {
    return false;
  }
  *output = (uint16_t) i;
  return true;
}

/**
 * @brief Convert a string to a uint32_t.
 * @returns true if the input was within range, false otherwise.
 *
 * The string can either be decimal or hex (prefixed with 0x).
 */
bool StringToUInt32(const char *input, uint32_t *output) {
  long i = strtol(input, NULL, 0);
  if (i < 0 || i > UINT32_MAX) {
    return false;
  }
  *output = (uint32_t) i;
  return true;
}

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
};

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
};

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
    char hex_data[hex_data_size];
    r = read(fd, &hex_data, hex_data_size);
    if (r != hex_data_size) {
      printf("Failed to read %d bytes on line %d\n", hex_data_size, line);
      return;
    }

    uint8_t calculated_checksum = (
        byte_count + (address >> 8) + address + record_type);

    // convert hex data to actual data
    uint8_t data[byte_count];
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

void WriteDFUFile(const Options *options, const char *file) {
  int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    printf("Failed to open %s: %s\n", file, strerror(errno));
    return;
  }

  ssize_t r = write(fd, g_data, g_data_size);
  if (r != g_data_size) {
    printf("Failed: only wrote %zd / %d bytes to %s\n", r, g_data_size, file);
    close(fd);
    return;
  }
  printf("Wrote %zd / %d bytes of data to %s\n", r, g_data_size, file);

  // Calculate CRC
  uint32_t crc = 0xffffffff;
  for (unsigned int i = 0; i < g_data_size; i++) {
    crc = CalculateCRC(crc, g_data[i]);
  }

  // The DFU suffix, without the CRC.
  struct {
    uint8_t bLength;
    uint8_t dfu_signature[3];
    uint16_t dfu_specification;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device;
  } suffix = {
    .bLength = 16,
    .dfu_signature = {0x44, 0x46, 0x55},
    .dfu_specification = htons(0x0100),
    .vendor_id = htons(options->vendor_id),
    .product_id = htons(options->product_id),
    .device = 0xffff,
  };

  // Write the suffix out backwards.
  const uint8_t *suffix_ptr = (const uint8_t*) &suffix;
  for (int i = sizeof(suffix) - 1; i >= 0; i--) {
    crc = CalculateCRC(crc, suffix_ptr[i]);
    r = write(fd, suffix_ptr + i, 1);
    if (r != 1) {
      printf("Failed to write DFU suffix\n");
      close(fd);
      return;
    }
  }

  // Write the CRC
  r = write(fd, (uint8_t*) &crc, sizeof(crc));
  if (r != sizeof(crc)) {
    printf("Failed to write DFU CRC\n");
  }

  close(fd);
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

  static struct option long_options[] = {
      {"pid", required_argument, 0, 'p'},
      {"help", no_argument, 0, 'h'},
      {"vid", required_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  int c;
  int option_index = 0;

  while (1) {
    c = getopt_long(argc, argv, "p:hv:", long_options, &option_index);

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
        ;
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
  char output_file[strlen(options.input_file) + 1];
  strncpy(output_file, options.input_file, strlen(options.input_file));

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
    WriteDFUFile(&options, output_file);
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
