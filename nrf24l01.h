#ifndef __24L01_H
#define __24L01_H

#include "spi.h"


#define INIT_PTX_MODE 0x00
#define INIT_PRX_MODE 0X01
#define rx_payload_width(n)  n
#define tx_payload_width(n)  n

#define en_aa_channel(n) n
#define en_rxaar_channel(n) n
#define rf_ch_frequencychannel(n)  n

#define air_data_rate_2Mbps 0x08
#define air_data_rate_1Mbps 0x00
#define tx_output_power_Negative18dBm 0x00
#define tx_output_power_Negative12dBm 0x02
#define tx_output_power_Negative06dBm 0x04
#define tx_output_power_0dBm  0x06

#define enable_LNA_gain 0x01
#define unable_LNA_gain 0x00




typedef struct
{
	GPIO_TypeDef *CEport;
	GPIO_TypeDef *CSNport;
  GPIO_TypeDef *IRQport;

	uint16_t CEpin;
	uint16_t CSNpin;
	uint16_t IRQpin;

	uint8_t MODE;

	uint8_t en_rxaar;
	uint8_t en_aa;
	uint8_t rf_ch;
	uint8_t rf_setup;

	uint8_t *rx_addr;
	uint8_t *tx_addr;

	uint8_t RX_PLOAD_WIDTH;
	uint8_t TX_PLOAD_WIDTH;

	SPI_HandleTypeDef *hspin;

	uint8_t status;
}NRF24L01_HandleTypeDef;

#define NRF24L01_STATUS_DISCONNECT 0Xff


//SPI对nrf24l01寄存器操作指令
#define	NRF_READ_REG	0x00    //读取状态和操作寄存器操作
#define NRF_WRITE_REG 0x20    //写状态和操作寄存器操作，只在掉电和待机模式下可用

#define READ_RX_PAYLOAD 0x61	//读取RX有效数据 1-32字节,读操作从字节0开始，读取完毕后FIFO寄存器有效数据清零。在接受模式下使用
#define WRITE_TX_PAYLOAD 0xA0 //写TX有效数据 1-32字节，写字节都是从0开始的，在发射模式下使用

#define FLUSH_TX 0xE1  //清除TX FIFO寄存器，在发射模式下使用
#define FLUSH_RX 0xE2	 //清除RX FIFO寄存器，在接受模式下使用	在传输应答信号时不应执行，否则应答信号无法被完整传输

#define REUSE_TX_PAYLOAD  0xE3 //重新使用上一个传输的数据包，当CE为高过程中，数据包被不断的重新发射。直到W_TX_PAYLOAD or FLUSH TX 被执行。数据重发功能必须在发射数据包过程中禁止

#define NOP 0xFF  //空操作，用来读取状态寄存器

//nrf24l01寄存器地址
#define CONFIG 0x00  // 配置-寄存器  的地址
											//0位PRIM_RX：1：接收模式，0：发射模式
											//1位PWR_UP:  1:上电， 0：掉电
											//2位CRCO：	 0：8位CRC校验， 1：16位CRC校验
											//3位EN_CRC: CRC使能，如果EN-AA有1个位高，则EN_CRC同步为高
											//4为MASK_MAX_RT: 0:MAX_RT中断(最大重发次数)产生IRQ引脚电平为低，1：IRQ脚不产生MAX_RT中断
											//5位MASK_TX_DS:	0:IRQ中断显示TX_DS(transmit data sent)  1：屏蔽
											//6位MASK_RX_DR:	0:IRQ中断显示RX_DR(recieve data ready)  1；屏蔽
											//7位无： 只能位0


#define EN_AA 0x01 	  //ENABLE-AUTO-ACKKNOWLEGEMENT 使能自动应答功能
											//0-5位：0-5通道： 1：对应通道启动自动应答
											//6和7位：只能是00


#define EN_RXADDR 0x02//接收地址使能
											//0-5位：0-5通道： 0：使能对应通道
											//6-7位：00 only

#define SETUP_AW  0x03//设置地址宽度(对所有通道)
											//1-0两位:	01：3字节 10：4字节  11：5字节宽度

#define	SETUP_RETR 0x04 //建立自动重发功能(setup auto retransmit)
											  //3：0位：0000：不自动重发，0001：自动重发1次 --- 1111：自动重发15次
												//7：4位：自动重发延时 0000：250+86um，	0001：500+86us ....1111：4000+86us

#define RF_CH		0x05	//RF CHANNEL
											//7	设置：只能是0
											//6：0设置： 设置工作通道频率

#define RF_SETUP 0x06 //射频寄存器
											//0位：设置低噪声增益
											//2：1位：发射功率	00：-18dbm，01：-12dbm，10：-6dbm，11：0dbm
											//3位：数据传输率 0：1mbps，1：2mbps
											//其余位0

#define STATUS	0x07	//状态寄存器
											//0位：TX FIFO(TX_FULL)  0：TXFIFO寄存器未满，1：满了
											//3：1位：接收数据通道，000-101：数据通道号，110：未使用，111：RX FIFO 满了
											//4位：MAX_RT 达到最大重发次数中断，写1：清除中断，必须清除后才能继续进行通信。
											//5位：TX_DS 数据发送完成中断：发送完后自动产生，写1为清除
											//6位：RX_DR 接收数据中断：收到有效数据后位1。写1清除。
											//7位：0	only
#define STATUS_MAX_RT 0x10
#define STATUS_TX_OK 0x20
#define STATUS_RX_OK 0x40


#define OBSERVE_TX 0x08//发送检测寄存器
											 //7：4位：数据包丢失计数器，当写RF_CH寄存器时复位，丢失15个后自动重启
											 //3：0位：重发计数器，发送新数据包时此寄存器复位

#define	CD 0x09				//载波检测：0位：载波检测
											//7：1位：全0


#define RX_ADDR_P0 0x0A// 39：0位：通道0接收地址数据，最大5字节长，低字节先写，所写字节由SETUUP_AW决定
#define RX_ADDR_P1 0x0B//	39：0位：通道1接收地址数据，最大5字节长，...
#define RX_ADDR_P2 0x0C//	7：0位：通道2接收地址数据，最低字节可设置，膏滋节必须和RX_ADDR_P1[39:8]相等
#define RX_ADDR_P3 0x0D//	7：0位：通道3接收地址数据...
#define RX_ADDR_P4 0x0E// 7：0位：通道4接收地址数据...
#define RX_ADDR_P5 0x0F// 7：0位：通道5接收地址数据...


#define TX_ADDR	0x10	 //39：0位：40=8*5发送地址(先写低字节)在增强型 ShockBurstTM 模式下 RX_ADDR_P0 与此地址相等。

#define RX_PW_P0	0x11 //PAYLOAD-WIDTH 5:0位：接收数据通道0有效数据宽度，1-32：1-32字节宽
											 //7：6位：00
#define	RX_PW_P1	0x12
#define	RX_PW_P2	0x13
#define	RX_PW_P3	0x14
#define	RX_PW_P4	0x15
#define	RX_PW_P5	0x16

#define FIFO_STATUS	0x17//FIFO寄存器状态
												//0位：RX_EMPTY	0：RX FIFO寄存器非空，1：RX FIFO寄存器空了
												//1位：RX_FULL 0：RX FIFO 未满，1：满了
												//3：2位：默认00
												//4位：TX_EMPTY	0：TX FIFO寄存器非空，1：空
												//5位：TX_FULL	0:TX	FIFO未满，1：满了
												//6位：TX_REUSE 若 TX_REUSE=1 则当 CE 位高电平状态时不断 发送上一数据包。TX_REUSE 通过 SPI 指令 REUSE_TX_PL 设置，通过 W_TX_PALOAD 或 FLUSH_TX 复位。
												//7位：默认0



#define CSN_0 	HAL_GPIO_WritePin(hnrf24l01.CSNport, hnrf24l01.CSNpin,GPIO_PIN_RESET)
#define CSN_1  	HAL_GPIO_WritePin(hnrf24l01.CSNport, hnrf24l01.CSNpin,GPIO_PIN_SET)
#define CE_0 	  HAL_GPIO_WritePin(hnrf24l01.CEport, hnrf24l01.CEpin,GPIO_PIN_RESET)
#define CE_1  	HAL_GPIO_WritePin(hnrf24l01.CEport, hnrf24l01.CEpin,GPIO_PIN_SET)
#define READ_IRQ			HAL_GPIO_ReadPin(hnrf24l01.IRQport,   hnrf24l01.IRQpin)






//SPI和nrf24l01交互
uint8_t nrf24l01_write_buf(uint8_t nrf24l01_reg,uint8_t *pbuf,uint8_t size);
uint8_t nrf24l01_read_buf(uint8_t nrf24l01_reg,uint8_t *pbuf,uint8_t size);

uint8_t nrf24l01_write_reg(uint8_t nrf24l01_reg,uint8_t value);
uint8_t nrf24l01_read_reg(uint8_t nrf24l01_reg);
void NRF24L01_Config(void);
void NRF24L01_Init(void);
uint8_t NRF24L01_Check(void);
void NRF24L01_RxMode(void);
void NRF24L01_TxMode(void );

uint8_t NRF24L01_Transmit(uint8_t *tbuf);
uint8_t NRF24L01_Recieve(uint8_t *rbuf);


//------------------------------------------------------------------------------------




extern NRF24L01_HandleTypeDef hnrf24l01;

#endif
