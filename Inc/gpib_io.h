/*
 * Copyright (c) 2015, M.Naruoka (fenrir)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the naruoka.org nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __GPIB_IO_H__
#define __GPIB_IO_H__



#define GPIB_WRITE_USE_EOI 0x01
#define GPIB_READ_UNTIL_EOI 0x01


#define GPIB_GETCHAR_IS_ERROR(x) (x < 0)
#define GPIB_GETCHAR_IS_EOI(x) (x >= 0x100)
#define GPIB_GETCHAR_IS_CMD(x) (x >= 0x200)
#define GPIB_GETCHAR_TO_DATA(x) ((uint8_t)(x & 0xFF))

enum gpib_command_t {
  GPIB_CMD_DCL = 0x14,
  GPIB_CMD_UNL = 0x3F,
  GPIB_CMD_UNT = 0x5F,
  GPIB_CMD_GET = 0x08,
  GPIB_CMD_SDC = 0x04,
  GPIB_CMD_LLO = 0x11,
  GPIB_CMD_GTL = 0x01,
  GPIB_CMD_SPE = 0x18,
  GPIB_CMD_SPD = 0x19,
};
#define GPIB_CMD_LAD(x) (x | 0x20)
#define GPIB_CMD_IS_LAD_OR_UNL(x) ((x & 0x60) == 0x20)
#define GPIB_CMD_TAD(x) (x | 0x40)
#define GPIB_CMD_IS_TAD_OR_UNT(x) ((x & 0x60) == 0x40)



typedef struct {
  uint8_t item[15][2];
  uint8_t valid_items;
} address_t;

typedef struct {
  address_t address;
  uint8_t read_after_write;
  uint8_t eoi;
  uint8_t eos;
  uint8_t eot;
  char eot_char;
  uint8_t listen_only;
  uint8_t is_controller;
  uint16_t timeout_ms;
  uint8_t status;
  uint8_t debug;
} gpib_config_t;





enum uniline_message_t {
  GPIB_UNI_CMD_START,
  GPIB_UNI_CMD_END,
  GPIB_UNI_BUS_CLEAR_START,
  GPIB_UNI_BUS_CLEAR_END,
  GPIB_UNI_CHECK_SRQ_ASSERT,
  GPIB_UNI_SRQ_ASSERT,
  GPIB_UNI_SRQ_DEASSERT,
};


extern gpib_config_t gpib_config;


void gpib_io_init(void);
int8_t gpib_putchar(char c, uint8_t flags);
uint16_t gpib_write(char *buf, uint16_t length, uint8_t flags);
int16_t gpib_getchar(void);
uint16_t gpib_read(char *buf, uint8_t flags);
int8_t gpib_uniline(enum uniline_message_t msg);
int8_t gpib_cmd(uint8_t listen_addr, uint8_t talk_addr);

#endif /* __GPIB_IO_H__ */
