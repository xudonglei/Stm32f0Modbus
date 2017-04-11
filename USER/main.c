#include "main.h"

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	SysTick_Init(48);
    LED_Init();
	Uart_Init(115200);
	
	while(1)
	{  
	   Modbus_Poll();
	}
}
