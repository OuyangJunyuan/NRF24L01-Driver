#include "nrf24l01.h"


NRF24L01_HandleTypeDef hnrf24l01;

uint8_t SPI_ReadWriteByte(uint8_t byte)
{
	uint8_t read,send=byte;
	if(HAL_SPI_TransmitReceive(hnrf24l01.hspin,&send,&read,1,100)!=HAL_OK)
		read=0;
	return read;
}

//SPI写寄存器
uint8_t nrf24l01_write_reg(uint8_t nrf24l01_reg,uint8_t value)
{
	uint8_t status;
	CSN_0;
	status =SPI_ReadWriteByte(nrf24l01_reg);
	SPI_ReadWriteByte(value);
	CSN_1;
	return (status);
}

//SPI读寄存器
uint8_t nrf24l01_read_reg(uint8_t nrf24l01_reg)
{
	uint8_t	reg_value;
	CSN_0;
	SPI_ReadWriteByte(nrf24l01_reg);
	reg_value=SPI_ReadWriteByte(0XFF);
	CSN_1;
	return (reg_value);
}


uint8_t nrf24l01_read_buf(uint8_t nrf24l01_reg,uint8_t *pbuf,uint8_t size)
{
	uint8_t status,i;
	CSN_0;
	status=SPI_ReadWriteByte(nrf24l01_reg);
	for(i=0; i<size;++i)
	{
		pbuf[i]=SPI_ReadWriteByte(0xff);
	}
	CSN_1;
	return status;
}


uint8_t nrf24l01_write_buf(uint8_t nrf24l01_reg,uint8_t *pbuf,uint8_t size)
{
	uint8_t status,i;
	CSN_0;
	status=SPI_ReadWriteByte(nrf24l01_reg);
	for(i=0;i<size;++i)
	{
		SPI_ReadWriteByte(*pbuf++);
	}
	CSN_1;
	return status;
}



uint8_t NRF24L01_Check(void)
{
	uint8_t get[5];
	uint8_t i;
	nrf24l01_write_buf(NRF_WRITE_REG + TX_ADDR,hnrf24l01.rx_addr,5);
	nrf24l01_read_buf(NRF_READ_REG + TX_ADDR,get,5);
	for(i=0;i<5;++i)
	{
		if(get[i]!=hnrf24l01.rx_addr[i])
			return 0;
	}
	return 1;
}


void NRF24L01_RxMode()
{
	CE_0;
	nrf24l01_write_buf(NRF_WRITE_REG + RX_ADDR_P0,(uint8_t *)hnrf24l01.rx_addr,5);
	nrf24l01_write_reg(NRF_WRITE_REG + RX_PW_P0,hnrf24l01.RX_PLOAD_WIDTH);
	nrf24l01_write_reg(NRF_WRITE_REG + CONFIG,0x0f);
	CE_1;
}

void NRF24L01_TxMode()
{
	CE_0;

	nrf24l01_write_buf(NRF_WRITE_REG + TX_ADDR,(uint8_t *)hnrf24l01.tx_addr,5 );
	nrf24l01_write_reg(NRF_WRITE_REG + SETUP_RETR,0x0A);
	nrf24l01_write_reg(NRF_WRITE_REG + CONFIG,0x0e);
	nrf24l01_write_reg(NRF_WRITE_REG + RX_PW_P0,hnrf24l01.RX_PLOAD_WIDTH);
	CE_1;
}

uint8_t NRF24L01_Transmit(uint8_t *tbuf)
{
	uint8_t status;
	CE_0;
	nrf24l01_write_buf(NRF_WRITE_REG + RX_ADDR_P0,(uint8_t *)hnrf24l01.tx_addr,5 );
	nrf24l01_write_buf(WRITE_TX_PAYLOAD,tbuf,hnrf24l01.TX_PLOAD_WIDTH);
	CE_1;
	while(GPIO_PIN_RESET != READ_IRQ );
	status=nrf24l01_read_reg(STATUS);
    nrf24l01_write_reg(NRF_WRITE_REG+STATUS,status);

	if(status & STATUS_MAX_RT)
	{
		nrf24l01_write_reg(FLUSH_TX,0XFF); //0xff=nop
		return STATUS_MAX_RT;
	}
	if(status & STATUS_TX_OK)
	{
		return STATUS_TX_OK;
	}
	return 0xff;
}


uint8_t NRF24L01_Recieve(uint8_t *rbuf)
{
	uint8_t status;
	do
	{
		status=nrf24l01_read_reg(STATUS);
	}while( !(status & STATUS_RX_OK) );

	nrf24l01_write_reg(NRF_WRITE_REG+STATUS,status);

	nrf24l01_read_buf(READ_RX_PAYLOAD,rbuf,hnrf24l01.RX_PLOAD_WIDTH);
	nrf24l01_write_reg(FLUSH_RX,0xff);
	return STATUS_RX_OK;
}


void NRF24L01_Config()
{
	CE_0;
	nrf24l01_write_reg(NRF_WRITE_REG + EN_AA,hnrf24l01.en_aa);
	nrf24l01_write_reg(NRF_WRITE_REG + EN_RXADDR,hnrf24l01.en_rxaar);
	nrf24l01_write_reg(NRF_WRITE_REG + RF_CH,hnrf24l01.rf_ch);
	nrf24l01_write_reg(NRF_WRITE_REG + RF_SETUP,hnrf24l01.rf_setup);
	CE_1;

	while(NRF24L01_Check()==0)
	{
		hnrf24l01.status=NRF24L01_STATUS_DISCONNECT;
		HAL_Delay(1);
	}

	if(hnrf24l01.MODE==INIT_PTX_MODE)
	{
		NRF24L01_TxMode();
	}
	if(hnrf24l01.MODE==INIT_PRX_MODE)
	{
		NRF24L01_RxMode();
	}
}
















//设置发送机地址和接收机地址------------------------------------------------------------------------------------------------------------------------------------------
uint8_t Tx_addr[5]={0xff,0xff,0xff,0xff,0xff};
uint8_t Rx_addr[5]={0xff,0xff,0xff,0xff,0xff};


//用户配置地方
void NRF24L01_Init(void)
{

    //**********************************可以不用修改********************************************//
    //******************************************************************************************//
	hnrf24l01.TX_PLOAD_WIDTH=tx_payload_width(0x20);  //设置发送buf的字数，1-32字。
	hnrf24l01.RX_PLOAD_WIDTH=rx_payload_width(0x20);	//设置接收buf的字数，1-32字
	//该模块有6个通道可以使用，而且可以6个同时发送给1个接收机，接收机扫描寄存器中的6个RX_FIFO，来接收6个通道数据。
	hnrf24l01.en_aa=en_aa_channel(0x01);							//对第n个通道的自动应答使能
	hnrf24l01.en_rxaar=en_rxaar_channel(0x01);			  //对第n个通道的接收地址使能
	hnrf24l01.rf_ch=rf_ch_frequencychannel(0x00);				//设置频率通道2.4GHz+n*20Hz，可设置位n=0-5;
	hnrf24l01.rf_setup= ( air_data_rate_2Mbps | tx_output_power_0dBm | enable_LNA_gain );  //设置通信速度，设置信号增益，设置低噪声放大器
    //******************************************************************************************//
    //******************************************************************************************//







    //**********************************根据具体定义填写****************************************//
    //******************************************************************************************//
	hnrf24l01.MODE=INIT_PRX_MODE;     //收发模式，INIT_PRX_MODE/INIT_PTX_MODE

	hnrf24l01.CEport=GPIOA;         //GPIO_CE--需要设置GPIO为Output
	hnrf24l01.CEpin=GPIO_PIN_4;

	hnrf24l01.CSNport=GPIOB;		//GPIO_CSN--需要设置GPIO为Output
	hnrf24l01.CSNpin=GPIO_PIN_12;

	hnrf24l01.IRQport=GPIOA;		//GPIO_IRQ--需要设置GPIO为Input
	hnrf24l01.IRQpin=GPIO_PIN_8;

	hnrf24l01.rx_addr=Rx_addr;      //设置接收地址
	hnrf24l01.tx_addr=Tx_addr;	    //设置发送地址

    hnrf24l01.hspin=&hspi2;         //hspin———设置SPI为主机，8字节模式，使用时钟相位为低，相位为1edg，片选使能为软件控制，高位字节优先模式。

    //******************************************************************************************//
    //******************************************************************************************//



    //**************************然后在main中使用NRF24L01_Init()**********************************//
    //******************就可以使用NRF24L01_Recieve()和NRF24L01_Transmit() 收发数据了*************//
    //******************************收发循环最好设置一个10ms的延时*******************************//


	NRF24L01_Config();                    //执行配置
}












