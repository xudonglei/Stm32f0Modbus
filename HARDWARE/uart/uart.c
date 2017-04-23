#include "uart.h"

UART_HandleTypeDef UART1_Handler;
void Uart_Init(uint32_t bound)
{
	UART1_Handler.Instance = USART1;
	UART1_Handler.Init.BaudRate = bound;
	UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART1_Handler.Init.StopBits = UART_STOPBITS_1;
	UART1_Handler.Init.Parity = UART_PARITY_NONE;
	UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART1_Handler.Init.Mode = UART_MODE_TX_RX;
	
	HAL_UART_Init(&UART1_Handler);
	__HAL_UART_ENABLE_IT(&UART1_Handler,UART_IT_RXNE);
	USART1->CR2 |= 0x00800000;          //RTOE set 1£¬Receive timeout detection enable
	USART1->CR1 |= 0x04000000;          //RTOIE set 1£¬Receive timeout interrupt enable
	USART1->RTOR &=0;
	USART1->RTOR |=4;                   //Set timeout register	
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
	GPIO_InitTypeDef  GPIO_Initure;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();
	GPIO_Initure.Pin=GPIO_PIN_9; 
    GPIO_Initure.Mode=GPIO_MODE_AF_PP; 
	GPIO_Initure.Pull=GPIO_PULLUP; 
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH; 
	GPIO_Initure.Alternate=GPIO_AF1_USART1; 
	HAL_GPIO_Init(GPIOA,&GPIO_Initure); 
	GPIO_Initure.Pin=GPIO_PIN_10;
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	HAL_NVIC_EnableIRQ(USART1_IRQn);      //Enable USART1 interrupt channel
	HAL_NVIC_SetPriority(USART1_IRQn,1,0);
}

T_ModbusStruct tModbusStruct;
void USART1_IRQHandler(void)
{
	uint8_t res;
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))
	{
		if(tModbusStruct.RecEndFlag != SET)
		{
			if(tModbusStruct.RecLen < MAXRECLEN)
			{
				HAL_UART_Receive(&UART1_Handler,&res,1,1000);
				tModbusStruct.RecData[tModbusStruct.RecLen++]=res;
			}
		}
	}
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RTOF)!=RESET))
	{   
		USART1->ICR |= 0x00000800;          //Clear RTOF interrupt flag
		tModbusStruct.RecEndFlag=SET;        //Receive complete       
	}
	//HAL_UART_IRQHandler(&UART1_Handler);//do not need
}

void Rs485SendBuf(uint8_t *buf,uint8_t len)
{
	HAL_UART_Transmit(&UART1_Handler, buf, len, 1000);
}
