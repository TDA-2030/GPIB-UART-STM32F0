/*****************************************************************************
File name: gpib_io.c
Description: gpib底层控制，移植自github开源项目
Author: zhouli
Version: V1.0
Date: 2019.12.25
History:

***************************************************************************/

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


#include "main.h"
#include "gpib_io.h"
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DIOX  0xFF00
static void CtrlPortOUT(uint32_t pin_mask)
{
	LL_GPIO_SetPinMode(ATN_GPIO_Port, pin_mask, LL_GPIO_MODE_OUTPUT);
}
static void CtrlPortIN(uint32_t pin_mask)
{
	LL_GPIO_SetPinMode(ATN_GPIO_Port, pin_mask, LL_GPIO_MODE_INPUT);
}


static void DataPortOUT(void)
{
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO1_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO2_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO3_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO4_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO5_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO6_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO7_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO8_Pin, LL_GPIO_MODE_OUTPUT);

}
static void DataPortIN(void)
{
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO1_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO2_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO3_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO4_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO5_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO6_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO7_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinMode(DIO1_GPIO_Port, DIO8_Pin, LL_GPIO_MODE_INPUT);
}

static void CtrlPinHigh(uint32_t pin_mask)
{
	LL_GPIO_SetOutputPin(ATN_GPIO_Port, pin_mask);
}
static void CtrlPinLow(uint32_t pin_mask)
{
	LL_GPIO_ResetOutputPin(ATN_GPIO_Port, pin_mask);
}
static uint8_t CtrlPinRead(uint32_t pin_mask)
{
	return (LL_GPIO_ReadInputPort(ATN_GPIO_Port)&pin_mask) ? 1 : 0;
}


static void DataPortWrite(uint8_t pin_mask)
{
	uint32_t reg = DIO1_GPIO_Port->ODR & (~DIOX);
	reg |= pin_mask<<8;
	DIO1_GPIO_Port->ODR = reg;
}
static uint8_t DataPortRead(void)
{
	return (LL_GPIO_ReadInputPort(DIO1_GPIO_Port)&DIOX) >> 8;
}



static uint8_t is_talker;

gpib_config_t gpib_config = {
  {{{0},}, 0,}, // address
  0, // read_after_write
  0, // eoi
  0, // eos
  0, // eot
  (char)0, // eot_char
  0, // listen_only
  1, // is_controller
  20, // timeout_ms
  0, // status
  0, // debug
};


static void set_talker(void)
{
    if (!is_talker)
    {
		is_talker = TRUE;
		DataPortOUT();
		CtrlPinHigh(DAV_Pin | EOI_Pin);
		CtrlPortOUT(DAV_Pin);
		CtrlPortOUT(EOI_Pin);
		CtrlPortIN(NRFD_Pin);
		CtrlPortIN(NDAC_Pin);

    }
}

static void set_listener(void)
{
    if (is_talker)
    {
        is_talker = FALSE;
		DataPortIN();
		CtrlPortIN(DAV_Pin);
		CtrlPortIN(EOI_Pin);

        CtrlPinLow(NDAC_Pin);
		CtrlPinHigh(NRFD_Pin);
		CtrlPortOUT(NRFD_Pin);
		CtrlPortOUT(NDAC_Pin);
    }
}

// Puts all the GPIB pins into their correct initial states.
void gpib_io_init(void)
{
    set_talker();

    if (gpib_config.is_controller)
    {
		CtrlPortIN(SRQ_Pin);

		CtrlPortOUT(ATN_Pin);
		CtrlPortOUT(REN_Pin);
		CtrlPortOUT(IFC_Pin);
		CtrlPinHigh(ATN_Pin);
		CtrlPinHigh(IFC_Pin);
		CtrlPinLow(REN_Pin);

    }
    else
    {
		CtrlPortIN(ATN_Pin);
		CtrlPortIN(REN_Pin);
		CtrlPortIN(IFC_Pin);
		CtrlPortOUT(SRQ_Pin);
		CtrlPinHigh(SRQ_Pin);
    }
}



/**
  * @brief  gpib信号线某些控制
  * @param  msg：信号
  * @retval 总是返回0
  */
int8_t gpib_uniline(enum uniline_message_t msg)
{
    switch (msg)
    {
    case GPIB_UNI_CMD_START:
        CtrlPinLow(ATN_Pin);
        break;
    case GPIB_UNI_CMD_END:
        CtrlPinHigh(ATN_Pin);
        break;
    case GPIB_UNI_BUS_CLEAR_START:
        CtrlPinLow(IFC_Pin);
        break;
    case GPIB_UNI_BUS_CLEAR_END:
        CtrlPinHigh(IFC_Pin);
        break;
    case GPIB_UNI_CHECK_SRQ_ASSERT:
        return !CtrlPinRead(SRQ_Pin);
    case GPIB_UNI_SRQ_ASSERT:
        CtrlPinLow(SRQ_Pin);
        break;
    case GPIB_UNI_SRQ_DEASSERT:
        CtrlPinHigh(SRQ_Pin);
        break;
    }
    return 0;
}
/**
  * @brief  等待引脚出现期望电平
  * @param  state：期望的电平
  * @param  mask：引脚掩码
  * @retval 0:成功，其他值：超时失败
  */
static int8_t wait_pin(uint8_t state, uint32_t mask)
{
	uint32_t timeout_max;
	int8_t res = 1;
	timeout_max = HAL_GetTick() + gpib_config.timeout_ms;
    do
    {
        if (CtrlPinRead(mask) == state)
        {
            res = 0;
			break;
        }
    }while (timeout_max > HAL_GetTick());
    return res; // timeout
}

/**
  * @brief  gpib发送一个字节
  * @param  c:发送的字符
  * @param  flags：某些标志位，目前只有GPIB_WRITE_USE_EOI表示最后一个字节给出EOI信号
  * @retval 0:成功，其他值：失败
  */
static int8_t putchar_internal(uint8_t c, uint8_t flags)
{
	int8_t res = 0;
	LL_GPIO_SetOutputPin(LED_T_GPIO_Port, LED_T_Pin);
	do{

		DataPortWrite(~c);// Put the byte on the data lines
		if (flags & GPIB_WRITE_USE_EOI)
		{
			CtrlPinLow(EOI_Pin); // Assert EOI
		}

		// Make sure that NRFD is high
		if (wait_pin(1, NRFD_Pin))
		{
			gpib_io_init();
			res = -1;
			break;
		}
		CtrlPinLow(DAV_Pin); // Inform listeners that the data is ready to be read

		// Wait for NDAC to go high, all listeners have accepted the byte
		if (wait_pin(1, NDAC_Pin))
		{
			gpib_io_init();
			res = -2;
			break;
		}
		CtrlPinHigh(DAV_Pin | EOI_Pin); // Byte has been accepted by all, indicate byte is no longer valid
	}while(0);
	LL_GPIO_ResetOutputPin(LED_T_GPIO_Port, LED_T_Pin);
    return res;
}


/**
  * @brief  gpib发送一个字节
  * @param  c:发送的字符
  * @param  flags：某些标志位，目前只有GPIB_WRITE_USE_EOI表示最后一个字节给出EOI信号
  * @retval 0:成功，其他值：失败
  */
int8_t gpib_putchar(char c, uint8_t flags)
{
    set_talker();
    return putchar_internal(c, flags);
}
/**
  * @brief  gpib发送
  * @param  buf：发送的数据
  * @param  length：发送的数据长度
  * @param  flags：某些标志位，目前只有GPIB_WRITE_USE_EOI表示最后一个字节给出EOI信号
  * @retval 返回实际发送的数据长度
  */
uint16_t gpib_write(char *buf, uint16_t length, uint8_t flags)
{
    uint16_t remain = length;
    uint8_t flags_without_eoi = flags & ~(GPIB_WRITE_USE_EOI);

    if (length == 0)
    {
        return 0;
    }

    set_talker();
    do
    {
        // Loop through each character, write to bus
        if (0 != putchar_internal(*buf++, (remain > 1) ? flags_without_eoi : flags))
        {
            break;
        }
    }
    while (--remain);

    return length - remain;
}

/**
  * @brief  gpib读取一个字节
  * @param  None
  * @retval 读取到的数据，第0~7位是数据，第8、9位是标志位
  */
static int16_t getchar_internal(void)
{
    int16_t res;

	LL_GPIO_SetOutputPin(LED_L_GPIO_Port, LED_L_Pin);
	do{
		// Assuming both NDAC and NRFD are LOW.

		// Raise NRFD, telling the talker we are ready for the next byte
		CtrlPinHigh(NRFD_Pin);
		// Wait for DAV to go low (talker informing us the byte is ready)
		if (wait_pin(0, DAV_Pin))
		{
			gpib_io_init();
			res = -1; // timeout
			break;
		}

		// Assert NRFD, informing talker to not change the data lines
		CtrlPinLow(NRFD_Pin);

		// Read port, and flip all bits since GPIB uses negative logic.
		res = 0xff&(~DataPortRead());

		if (!CtrlPinRead(EOI_Pin))
		{
			res += 0x100;
		} // END
		if (!CtrlPinRead(ATN_Pin))
		{
			res += 0x200;
		} // CMD

		// Un-assert NDAC, informing talker that we have accepted the byte
		CtrlPinHigh(NDAC_Pin);
		// Wait for DAV to go high (talker knows that we have read the byte)

		if (wait_pin(1, DAV_Pin))
		{
			gpib_io_init();
			res = -2; // timeout
			break;
		}
		// Low NDAC to prepare for next byte
		CtrlPinLow(NDAC_Pin);
	}while(0);
	LL_GPIO_ResetOutputPin(LED_L_GPIO_Port, LED_L_Pin);
    return res;
}
/**
  * @brief  gpib读取一个字节
  * @param  None
  * @retval 读取到的数据，第0~7位是数据，第8、9位是标志位
  */
int16_t gpib_getchar(void)
{
    set_listener();
    return getchar_internal();
}

/**
  * @brief  gpib读取
  * @param  buf：读出的数据
  * @param  flags：某些标志位，目前只有GPIB_READ_UNTIL_EOI表示以EOI线的信号判断结束
  * @retval 返回实际读取到的数据长度
  */
uint16_t gpib_read(char *buf, uint8_t flags)
{
    uint16_t read_count = 0;
    int16_t res;
    char c;
    uint8_t last_char_is_cr = FALSE;

    // Make correspond receiving and transmitting terminators, different from Prologic impl.
    uint8_t terminator = (flags & GPIB_READ_UNTIL_EOI) ? 3 : gpib_config.eos;

    set_listener();
    while (1)
    {
        res = getchar_internal();
        if (GPIB_GETCHAR_IS_ERROR(res))
        {
			SYS_DbgLog("GETCHAR_IS_ERROR res=%d\r\n",res);
            break;
        }
        read_count++;
        c = GPIB_GETCHAR_TO_DATA(res);
        *buf++ = (c);
        if (terminator == 3 && GPIB_GETCHAR_IS_EOI(res))
        {
            break;
        }
        if (c == '\r')
        {
            if (terminator == 1)
            {
				SYS_DbgLog("terminator == 1\r\n");
                break;
            }
            last_char_is_cr = TRUE;
        }
        else
        {
            if (c == '\n')
            {
                if ((terminator == 2) || ((last_char_is_cr) && (terminator == 0)))
                {
					SYS_DbgLog("terminator == 2\r\n");
                    break;
                }
            }
            last_char_is_cr = FALSE;
        }
    }

    return read_count;
}


/**
  * @brief  向gpib总线发送命令，但是现在被限制死只是发送talk和listen的地址
  * @param  listen_addr：听者的地址
  * @retval talk_addr：讲者的地址
  */
int8_t gpib_cmd(uint8_t listen_addr, uint8_t talk_addr)
{
	int8_t res=0;

    gpib_uniline(GPIB_UNI_CMD_START);
	do{
		if(0 != gpib_putchar(GPIB_CMD_UNL, 0))
		{
			res = -1;
			break;
		}
		if(0 != gpib_putchar(GPIB_CMD_LAD(listen_addr), 0))
		{
			res = -2;
			break;
		}
		if(0 != gpib_putchar(GPIB_CMD_TAD(talk_addr), 0))
		{
			res = -3;
			break;
		}
	}while(0);

    gpib_uniline(GPIB_UNI_CMD_END);

	return res;
}

