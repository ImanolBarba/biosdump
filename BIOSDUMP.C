/***************************************************************************
 *   BIOSDUMP.C  --  This file is part of biosdump.                        *
 *                                                                         *
 *   Copyright (C) 2023 Imanol-Mikel Barba Sabariego                       *
 *                                                                         *
 *   biosdump is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published     *
 *   by the Free Software Foundation, either version 3 of the License,     *
 *   or (at your option) any later version.                                *
 *                                                                         *
 *   biosdump is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty           *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.   *
 *                                                                         *
 ***************************************************************************/

#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BIOS_SIZE 0x10000
#define DEFAULT_BIOS_OFFSET 0xF0000

typedef struct args {
  const char* path;
  uint32_t bios_size;
  uint32_t bios_offset;
} args;

const char* get_executable_name(const char* path) {
  signed short last_backslash;
  for(last_backslash = strlen(path); path[last_backslash] != '\\' && last_backslash > -1; --last_backslash) {
    // nothing to see here
  }
  if(last_backslash) {
    last_backslash++;
  }
  return path + last_backslash;
}

void print_help(const char* exename) {
  printf("%s [-o OFFSET] [-s SIZE] OUTPUT\n", get_executable_name(exename));
  printf("\n");
  printf("\t-o Memory offset to BIOS (hex, absolute address)\n");
  printf("\t-s BIOS size (hex)\n");
  printf("\n");
}

int parse_num(long* num, const char* num_str, int base) {
  char* endptr;
  long num_ret = strtoul(num_str, &endptr, base);
  if(num == 0 && endptr != (num_str + strlen(num_str))) {
    return 1;
  }
  *num = num_ret;
  return 0;
}

int parse_args(int argc, char** argv, args* cmd, int* last_parsed) {
  int status;
  int i;
  long num;
  cmd->bios_size = DEFAULT_BIOS_SIZE;
  cmd->bios_offset = DEFAULT_BIOS_OFFSET;

  // Parse everything except the last argument, which is the output
  // file name
  for(i = 1; i < argc; ++i) {
    if(!strcmp(argv[i], "/?")) {
      // Fuck the rest of the parsing
      print_help(argv[0]);
      exit(0);
    } else if(!strcmp(argv[i], "-o")) {
      status = parse_num(&num, argv[++i], 16);
      if(status) {
        printf("Invalid offset specified: %s\n", argv[i]);
        return 1;
      }
      cmd->bios_offset = (uint32_t)num;
    } else if(!strcmp(argv[i], "-s")) {
      status = parse_num(&num, argv[++i], 16);
      if(status) {
        printf("Invalid size specified: %s\n", argv[i]);
        return 1;
      }
      cmd->bios_size = (uint32_t)num;
    } else {
      // If this is the last argument, skip, since it's the path
      if(i != argc-1) {
        printf("Unrecognised token: %s\n", argv[i]);
        return 1;
      }
    }
  }
  *last_parsed = i;
  return 0;
}

int dump_bios(uint32_t absolute_offset, uint32_t size, const char* path) {
  uint16_t segment = absolute_offset/16;
  uint16_t  offset = absolute_offset & 0x000F;
  uint16_t size_hi = *(((uint16_t*)(&size))+1);
  uint16_t size_lo = *((uint16_t*)(&size));

  void far* bios;
  int handle;
  unsigned ret;

  unsigned bytes_to_write;
  unsigned bytes_written;
  uint32_t total_bytes_written = 0;

  printf(
    "Dumping BIOS @ %04X:%04X with size %04X%04Xh to %s\n",
    segment,
    offset,
    size_hi,
    size_lo,
    path
  );

  ret = _dos_creat(path, _A_NORMAL, &handle);
  if(ret) {
    printf("Error opening output file: (code %d) %s\n", ret, strerror(errno));
    return ret;
  }

  while(total_bytes_written != size) {
    // Maximum dos write buffer is 0xFFFF
    bytes_to_write = min(0xFFFF, size - total_bytes_written); 

    segment = (absolute_offset + total_bytes_written)/16;
    offset = (absolute_offset + total_bytes_written) & 0x000F;
    bios = MK_FP(segment, offset);

    ret = _dos_write(handle, bios, bytes_to_write, &bytes_written);
    if(ret) {
      printf("Error writing to output file: (code %d) %s\n", ret, strerror(errno));
      return ret;
    }
    total_bytes_written += bytes_written;
  }

  ret = _dos_close(handle);
  if(ret) {
    printf("Error closing output file: (code %d) %s\n", ret, strerror(errno));
    return ret;
  }

  return 0;
}

int main(int argc, char **argv) {
  args cmd;
  int status;
  int last_parsed;

  // Begin

  memset(&cmd, 0x00, sizeof(args));
  status = parse_args(argc, argv, &cmd, &last_parsed);
  if(status) {
    // Don't print help in this occasion so the user can see what
    // went wrong
    printf("For help, run: %s /?\n", get_executable_name(argv[0]));
    return 1;
  }
  cmd.path = argv[argc-1];
  if(last_parsed == argc) {
    printf("No output specified. Dumping in current directory as BIOS.BIN\n");
    cmd.path = "BIOS.BIN";
  }

  status = dump_bios(cmd.bios_offset, cmd.bios_size, cmd.path);
  if(status) {
    printf("Error dumping BIOS\n");
    return status;
  }

  printf("BIOS dumped successfully :)\n");
  return 0;
}
