/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
//#include "platform/mbed_thread.h"
#include "Sht31.h"
#define MAXIMUM_BUFFER_SIZE
using namespace std;
//sda, scl
Sht31   temp_sensor(I2C_SDA, I2C_SCL);
    float t;
    float h;

Thread thread1;
Thread thread2;
DigitalOut led1(LED1);

static UnbufferedSerial pc(USBTX, USBRX);
static UnbufferedSerial  dev(D8, D2);


int dev_RxLen=0;
int pc_RxLen=0;
int hh,tt;
    
 char dev_RxBuf[64] = {0};
char pc_RxBuf[64] = {0};
  char buf4[64] = {0};
char buf_datchik[64] = {0};
  

// обработчики прерываний по приему байта с устройства и с компа- просто заполняют буферы свои- при достижении 64 байта в буфере -начинают заполнять снова с нуля
// нужны только для отладки - в рабочем режиме- тключить для экономии энергии
void dev_recv()
{
if (dev_RxLen<63){

    dev.read(&dev_RxBuf[dev_RxLen], sizeof(dev_RxBuf[dev_RxLen]));    //  Got 1 char
         
    dev_RxLen++;
}
else dev_RxLen=0;
    }


 
void pc_recv()
{
    if (pc_RxLen<63){
    pc.read(&pc_RxBuf[pc_RxLen], sizeof(pc_RxBuf[pc_RxLen]));  //  Got 1 char
    pc_RxLen++;
    }
else pc_RxLen=0;
}


void recieve_otvety()
{                   
   
     
        if (dev_RxLen>0)
        {    
    //нужно только для отладки -можно убрать
    //распечатать на консоль то что пришло  с рак811
    for (uint8_t i = 0; i < dev_RxLen; i++) {  
   pc.write(&dev_RxBuf[i], sizeof(dev_RxBuf[i]));}
 
     //распечатать на консоль то что пришло с компа    
 //   for (uint8_t i = 0; i < pc_RxLen; i++) {  
 //   pc.write(&pc_RxBuf[i], sizeof(pc_RxBuf[i]));}
    
        dev_RxLen=0;
        ThisThread::sleep_for(1s);
        }
   }
   

void read_datchik_and_send_to_server_lora()
{
     while (true) {
        t = temp_sensor.readTemperature();
        h = temp_sensor.readHumidity();
        int tt=round(t); //из-за того что %f не работает пришлось посылать округленные показания датчиков
        sprintf(buf_datchik,"temp= %x",tt); 
        for (uint8_t i = 0; i < 8; i++) {  
    pc.write(&buf_datchik[i], sizeof(buf_datchik[i]));}
     int hh=round(h);
        sprintf(buf_datchik,"himi= %x",hh); 
        for (uint8_t i = 0; i < 8; i++) {  
    pc.write(&buf_datchik[i], sizeof(buf_datchik[i]));}
    
    
    
          //послать округленные до целого данные с датчика темп-ры и без пробела с датчика влажности на  рак811
   //  sprintf(buf4,"at+send=lora:1:%d %d\r\n",tt,hh);
      sprintf(buf4,"at+send=lora:1:%d%d\r\n",tt,hh);    
    for (uint8_t i = 0; i < 21; i++) {  
    dev.write(&buf4[i], sizeof(buf4[i]));
    //продублировать это в консоль
    pc.write(&buf4[i], sizeof(buf4[i]));} 
     
          //послать округленные до целого данные с датчика влажности на  рак811 - попытка послать наканал 0 - так не работает 
          //это можно убрать
 //    sprintf(buf4,"at+send=lora:0:%d\r\n",hh);
//    for (uint8_t i = 0; i < 19; i++) {  
 //   dev.write(&buf4[i], sizeof(buf4[i]));
    //продублировать это в консоль
 //   pc.write(&buf4[i], sizeof(buf4[i]));} 
    
    
         
       ThisThread::sleep_for(20s);
}
}




int main()
{
    pc.attach(&pc_recv, UnbufferedSerial::RxIrq);
    dev.attach(&dev_recv, UnbufferedSerial::RxIrq);
        
    pc.baud(115200);
    dev.baud(115200);
    
//фориат передачи -по умолчанию - поэтому не нужно
  //     pc.format(8, BufferedSerial::None,  1    );
   //     dev.format(8, BufferedSerial::None, 1    );
   //
   // подключиться к серверу (параметры подключения уже установлены везде -настройка передатчика, базовой станции, сервера )
   //и ждать 10сек успешного подключения -так как сервер отвечает не сразу
   sprintf(buf4,"at+join\r\n"); 
  for (uint8_t i = 0; i < 9; i++) {  
    dev.write(&buf4[i], sizeof(buf4[i]));
//распечатать на консоль то что послано на  рак811
pc.write(&buf4[i], sizeof(buf4[i]));
        }  
         ThisThread::sleep_for(10s); 
         

    thread1.start(read_datchik_and_send_to_server_lora);
    thread2.start(recieve_otvety);
    
}


