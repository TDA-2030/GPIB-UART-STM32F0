/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "string.h"
#include "shell.h"

#define BUFFER_MAX_SIZE 256    //缓冲区大小
typedef struct{

uint16_t head;                    //缓冲区头部位置
uint16_t tail;                    //缓冲区尾部位置
uint8_t ringBuf[BUFFER_MAX_SIZE]; //缓冲区数组

}ringBuffer_t;



static ringBuffer_t buffer={0};  //定义一个结构体


/**
* @brief 写一个字节到环形缓冲区
* @param data：待写入的数据
* @return none
*/
int8_t RingBuf_Write(uint8_t data)
{
	buffer.ringBuf[buffer.tail] = data; //从尾部追加
	if (++buffer.tail >= BUFFER_MAX_SIZE ) //尾节点偏移
	{
		buffer.tail = 0; //大于数组最大长度 归零形成环形队列
	}
	if (buffer.tail == buffer.head) //如果尾部节点追到头部节点，则修改头节点偏移位置丢弃早期数据
	{
		//TODO:溢出了咋办
		SYS_DbgLog("!!!RingBuf Write OVER!!!\r\n");
		return 1;

//		if (++buffer.head >= BUFFER_MAX_SIZE)
//		{
//			buffer.head = 0;
//		}
	}
	return 0;
}
/**
* @brief 读取环形缓冲区的一个字节的数据
* @param *pData:指针，用于保存读取到的数据
* @return -1表示缓冲区是空的，0表示读取数据成功
*/
signed char RingBuf_Read(char *pData)
{
	if(buffer.head == buffer.tail)  //如果头尾接触表示缓冲区为空
	{
		return -1;//返回1，环形缓冲区是空的
	}
	else
	{
		*pData=buffer.ringBuf[buffer.head];  //如果缓冲区非空则取头节点值并偏移头节点
		if(++buffer.head>=BUFFER_MAX_SIZE)
		{
			buffer.head=0;
		}
		return 0;//返回0，表示读取数据成功
	}
}

/**
  * @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
	if(LL_USART_IsActiveFlag_RXNE(USART1))
	{
		uint8_t d = LL_USART_ReceiveData8(USART1);
		RingBuf_Write(d);
	}
	if(LL_USART_IsActiveFlag_ORE(USART1))
	{
		LL_USART_ClearFlag_ORE(USART1);
	}
}
/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**USART1 GPIO Configuration
  PB6   ------> USART1_TX
  PB7   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, 0);
  NVIC_EnableIRQ(USART1_IRQn);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART1);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);

}

/* USER CODE BEGIN 1 */




/**
  * @brief  串口2发送一字节数据
  * @param  data：数据
  * @retval result
  */
void usart1_send_byte(const char data)
{
	while(LL_USART_IsActiveFlag_TXE(USART1) == RESET);
	LL_USART_TransmitData8(USART1, data);
}
/**
  * @brief  串口2发送数据
  * @param  data：数据指针，length：数据长度
  * @retval result
  */
int8_t usart1_send_data(uint8_t *data, uint16_t length)
{
	while(length--)
	{
		while(LL_USART_IsActiveFlag_TXE(USART1) == RESET);
		LL_USART_TransmitData8(USART1, *data++);
	}
	return 0;
}




SHELL_TypeDef shell;

void shell_init(void)
{
	LL_USART_EnableIT_RXNE(USART1);
	shell.read = RingBuf_Read;
	shell.write = usart1_send_byte;
	shellInit(&shell);
}


void shell_exec(void)
{
	shellTask(&shell);

}



/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
