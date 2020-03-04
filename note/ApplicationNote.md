# NRF24l01-lib-3.0

​																																		                欧阳俊源，于2020/02/20

更新日志：

1. 学习HAL库在寄存器映射方面的书写习惯，比1.0更新了数据结构，配置更方便。
2. 增加了中断服务函数，可以进行中断接收，比轮询接收来的高效，减小MCU时间占用。
3. 完成了Enhance Shockburst的 驱动代码。可以自动完成应答和应答带数据包。
4. 根据以上完成片上时分双工的通信。发射方发送信息后接收方发回带应答的数据包，并以中断通知。

目前功能：

1. 普通发送/接收
2. 中断接收
3. 带数据的应答信号，即发送方发送一包数据，接收方应答一包可编写的反馈数据(否则只能产生不带数据包应答)。发射方接收到应答信号后执行回调函数，接收方完成反馈数据发送后执行回调函数。

To do list ：

1. 增加接收中断模式
2. 增加配对模式
3. 增加调频模式
4. 增加接收方检波，和信号功率指示来进行低功率模式通信。0dBm时的收发模式电流为10ms，而下电模式只有900nA待机模式为300uA，对比赛等功耗无要求场合可不计较。

# 使用配置

## 接口匹配配：

SPI接口配置：

注意点：1. 8字节数据包。2. 高bit先行。3. 波特率控制在8MHz附近。4. 时钟极性低。5. 时钟相位为第一沿。 6. 软件控制 

![image-20200220120128043](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220120128043.png)

---

GPIO配置：供需配置3个GPIO

IRQ：可选择上拉输入。如果想用中断式函数，则选用上拉**<u>下降沿</u>**外部中断输入模式

CSN：SPI的从机片选信号。推挽上拉**<u>输出</u>**。

CE:  IC的启动信号。使用推挽**<u>输出</u>**。



## 软件配置：

---

进入nrf24l01.c的最下方 **<u>line 399</u>** 处

只需修改此项：修改接收还是发送： NRF_InitStruct.Mode                       = NRF_MODE_TX 。

如需修改地址，则接收方的pipeadress的第一个地址和txmsgadress必须一致。

---

使用中断形式函数：

在stm32f1xx_it.c中对应管脚的中断服务函数处增加

![image-20200220123505448](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220123505448.png)

使用带数据包应答：此处要指定存放地址和长度。

![image-20200220123431813](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220123431813.png)

其余函数：



![image-20200220123153568](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220123153568.png)

这三个都是弱定义，用户可以自己实现。

* 第一个是发送方接收到应答数据包的回调函数。应答数据放在句柄下的pAckBuffPtr地址中。
* 第二个是接收回调函数，使用NRF24L01_Recieve_IT。
* 第三个是接收方的发送应答数据包完毕回调函数，必须使用NRF24L01_Recieve_IT。



---

主要的两个函数：为非堵塞式，在里面是循环轮询，所以需要设定超时时间，否则未接受或发送成功就死循环。

![image-20200220123640808](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220123640808.png)

NRF_InitStruct.Mode                       = NRF_MODE_TX ;

---



![image-20200220121303209](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220121303209.png)

* pipeadress：数据通道接收地址。

  ​		每行为1个5位的地址，共6个。第一个地址可以是独立5Bytes，第二到第六个共用后4个Bytes，所以第三到第六个地址的尾4Bytes填0。然后这六个地址开头的第一个Bytes要相互不同。

* txmsgadress：发送地址。

  ​		发送方该地址必须与自身数据通道接收**<u>第一地址</u>**相同

* pipepayloadwidth： 数据通道的数据包长度，最大1-32之间



![image-20200220121318751](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220121318751.png)

* 绑定管脚： 软硬件结合的地方，修改后面的GPIOB,GPIO_PIN_xx 为对应引脚即可。



![image-20200220121413536](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220121413536.png)

* AutoReTransmitDelayTime：自动重发时间，为等待接收方应答的超时时间。取值为250us-4000us，步进250us。如果使用带数据包应答，则最好将此值改为500us以上。
* AutoRetransmitCountMax：最大重发次数。
* FrequncyChannel：使用的信道。IC占用的通信频率2.4-2.525Hz，从2.4GHz开始步进1MHz。拥有125个1MHz带宽的信道。如果使用空中数据速率2MHz的，则只拥有一半的2MHz信道。
* RfAirDataRate：空中数据速率，倒数即GFSK解调无线信号1个bit的维持时间。
* RfPower：无线电发射频率。
* UseLNA：使用接收LNA。



![image-20200220122435835](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220122435835.png)

* Mode：初始化为接收模式还是发送模式
* EnablePipe：使用数据通道几。
* AutoAckPipe：启动数据通道x的自动应答



![image-20200220122645361](C:\Users\70654\AppData\Roaming\Typora\typora-user-images\image-20200220122645361.png)

* EnableAckPayLoad：使能带数据包应答
* EnableDyanmeicPayLoadWidth：使能动态长度数据包，如果使用带数据包应答则必须使能此项。
* EnableDynamicPayLoadPipe：对于接收方只需要使能P0，发送方使能希望的通道号。



---

