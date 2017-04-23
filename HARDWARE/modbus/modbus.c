#include "modbus.h"
T_ModbusReg tModbusReg;
extern T_ModbusStruct tModbusStruct; 

static uint16_t Get_Crc16(volatile uint8_t *ptr,uint8_t len) 
{ 
    uint8_t i; 
    uint16_t crc = 0xFFFF; 
    if(len==0) 
    {
        len = 1;
    } 
    while(len--)  
    {   
        crc ^= *ptr; 
        for(i=0; i<8; i++)  
    	{ 
            if(crc&1) 
        	{ 
                crc >>= 1;  
                crc ^= 0xA001; 
        	}  
        	else 
    		  {
                crc >>= 1;
        	} 
        }         
        ptr++; 
    } 
    return(crc); 
} 

void DataCopy(uint8_t *dest,const uint8_t *source,uint8_t len)
{
    uint8_t i;
    for(i=0; i<len ; i++)
    {
       dest[i] = source[i];
    }
}

static void ModbusSendWithCrc(uint8_t *pBuf,uint8_t len)
{
	uint16_t crcresult=0;
	uint8_t buf[MAXSENDLEN];
	DataCopy(buf,pBuf,len);
	crcresult = Get_Crc16(pBuf,len);
	buf[len++]=crcresult & 0xff;
	buf[len++]=(crcresult >> 8) & 0xff;
	
    Rs485SendBuf(buf,len);	
}

static void ModbusSendAckErr(uint8_t errcode)
{
	uint8_t temp[3];
	temp[0]=tModbusStruct.RecData[0];
	temp[1]=tModbusStruct.RecData[1] | 0x80;
	temp[2]=errcode;
	ModbusSendWithCrc(temp,3);
}

static void ModbusSendAckOK(void)
{
	uint8_t temp[6];
	uint8_t i;
	for(i=0;i<6;i++)
	{
		temp[i] = tModbusStruct.RecData[i];
	}
	ModbusSendWithCrc(temp, 6);	
}

static void ReadRegValue(uint8_t addr,uint8_t *regvalue)
{
	//tModbusReg.RegHoldingBuf[0]=0x1122;           //for debug
	//tModbusReg.RegHoldingBuf[1]=0x3344;
	regvalue[0]=tModbusReg.RegHoldingBuf[addr]>>8;	
  regvalue[1]=tModbusReg.RegHoldingBuf[addr];		
}

static void HandleReadHoldReg(uint8_t* RecBuf)
{
	uint16_t reg,num;
	uint8_t i;
	uint8_t RegValue[64];
	
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);//for debug
	tModbusStruct.Response=RTUOK;
    if(tModbusStruct.RecLen!=8)    //the length for 03H must be 8
    {
	   tModbusStruct.Response=RTUERRVALUE;
	   goto ERR;
	}
    reg=((uint16_t)tModbusStruct.RecData[2] << 8) | tModbusStruct.RecData[3];
	  num=((uint16_t)tModbusStruct.RecData[4] << 8) | tModbusStruct.RecData[5];
    if(num>sizeof(RegValue)/2)
	{
	   tModbusStruct.Response=RTUERRVALUE;
	   goto ERR;
	}
	
	for(i=0;i<num;i++)
	{
		ReadRegValue(reg,&RegValue[2*i]);
		reg++;
	}
ERR:
    if(tModbusStruct.Response==RTUOK)
    {
			tModbusStruct.SendLen=0;
			tModbusStruct.SendData[tModbusStruct.SendLen++]=tModbusStruct.RecData[0];
			tModbusStruct.SendData[tModbusStruct.SendLen++]=tModbusStruct.RecData[1];
			tModbusStruct.SendData[tModbusStruct.SendLen++]=num*2;
			for(i=0;i<num;i++)
			{
				tModbusStruct.SendData[tModbusStruct.SendLen++]=RegValue[2*i];
				tModbusStruct.SendData[tModbusStruct.SendLen++]=RegValue[2*i+1];	
			}
			ModbusSendWithCrc(tModbusStruct.SendData,tModbusStruct.SendLen);
	  }
    else
    {
		  ModbusSendAckErr(tModbusStruct.Response);
	  }		
}

static void WriteRegValue(uint16_t addr,uint16_t value)
{
	tModbusReg.RegHoldingBuf[addr]=value;
}

static void HandleWriteMultiHoldReg()
{
	uint16_t reg,num;
	uint16_t value;
	uint8_t i;
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);//for debug
	tModbusStruct.Response=RTUOK;
	if(tModbusStruct.RecLen!=13)    //the length for 10H must be 13
  {
	   tModbusStruct.Response=RTUERRVALUE;
	   goto ERR;
	}
	reg=((uint16_t)tModbusStruct.RecData[2] << 8) | tModbusStruct.RecData[3];
	num=((uint16_t)tModbusStruct.RecData[4] << 8) | tModbusStruct.RecData[5];
  for(i=0;i<num;i++)
	{ 
		value=((uint16_t)tModbusStruct.RecData[7+2*i]<<8)| tModbusStruct.RecData[8+2*i];
		WriteRegValue(reg,value);
		reg++;
	}
ERR:
  if(tModbusStruct.Response==RTUOK)	
	{
		 ModbusSendAckOK();
	}
	else
	{
     ModbusSendAckErr(tModbusStruct.Response);		
	}
}

static void ModbusApp(void)
{
   switch(tModbusStruct.RecData[1])
   {
	  case READHOLDREG:
		    HandleReadHoldReg(tModbusStruct.RecData);
		    break;
	  case WRITEMULTIHOLDREG:
		    HandleWriteMultiHoldReg();
		    break;
	  default:
		    break;
   }	
}
	
void Modbus_Poll(void) 
{  
	  uint16_t crcresult=0; 
	  uint8_t  temp[2]; 
	  if(tModbusStruct.RecEndFlag == SET)//Receive end then handle,otherwise quit
    { 
		  tModbusStruct.RecEndFlag = RESET;
      if(tModbusStruct.RecData[0] == ID)
	    {
				 crcresult = Get_Crc16(tModbusStruct.RecData,tModbusStruct.RecLen-2); 
				 temp[1] = crcresult & 0xff; 
				 temp[0] = (crcresult >> 8) & 0xff;
				 if((tModbusStruct.RecData[tModbusStruct.RecLen-1] == temp[0])&&(tModbusStruct.RecData[tModbusStruct.RecLen-2] == temp[1]))
				 {
								ModbusApp();
				 }
				 else
				 {
					//CRC fail
				 }
		  }
			tModbusStruct.RecLen=0;
	  }
}
