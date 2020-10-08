/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "sleep.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "xil_cache.h"
#include "xtmrctr.h"
#include "xsysmon.h"
#include "xgpio.h"
#include "PmodAD2.h"
#include "debounce.h"
#include "hppsi_board.h"


#define _XSM_CH_AUX_0    16
#define _XSM_CH_AUX_1    17
#define _XSM_CH_AUX_2    18
#define _XSM_CH_AUX_3    19
#define _XSM_CH_AUX_4    20
#define _XSM_CH_AUX_5    21
#define _XSM_CH_AUX_6    22
#define _XSM_CH_AUX_7    23
#define _XSM_CH_AUX_8    24
#define _XSM_CH_AUX_9    25
#define _XSM_CH_AUX_10   26
#define _XSM_CH_AUX_11   27
#define _XSM_CH_AUX_12   28
#define _XSM_CH_AUX_13   29
#define _XSM_CH_AUX_14   30
#define _XSM_CH_AUX_15   31

#define CORA_A0		_XSM_CH_AUX_1
#define CORA_A1		_XSM_CH_AUX_9
#define CORA_A2		_XSM_CH_AUX_6
#define CORA_A3		_XSM_CH_AUX_15
#define CORA_A4		_XSM_CH_AUX_5
#define CORA_A5		_XSM_CH_AUX_13
#define CORA_A6		_XSM_CH_AUX_12
#define CORA_A8		_XSM_CH_AUX_0
#define CORA_A10	_XSM_CH_AUX_8
#define CORA_VP		XSM_CH_VPVN

#define NUM_OF_CHANNELS 80
#define NUM_OF_INTMUX_CHANNEL 10
#define NUM_OF_EXTMUX_CHANNEL 8
#define MAX_SAMPLES_PER_CHANNEL 64

#define BUTTON_GPIO0_CHANNEL 1
#define LED_GPIO0_CHANNEL 2

#define MUX_SE_GPIO1_CHANNEL 1

#define DEBUG_PRINT
//#define MEASURE_TIMING


XGpio Gpio0; // The Instance of the GPIO driver for BTN & LEDs
XGpio Gpio1; // The Instance of the GPIO driver for MUX_SE

static XSysMon SysMonInst;      // System Monitor driver instance
XSysMon_Config *ConfigPtr;
XSysMon *SysMonInstPtr = &SysMonInst;

struct netif netifInst; // Network Interface driver instance
struct netif *netifPtr = &netifInst;

XTmrCtr TimerCounter; //instance of the Tmrctr device
XTmrCtr *TmrCtrInstancePtr = &TimerCounter;

ip_addr_t ipaddr, netmask, gw, forward_ip; //network addresses

unsigned char mac_ethernet_address[] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02}; //mac address

struct udp_pcb *udpPcb; // UDP structure
int udpPort; // UDP port
struct pbuf * udpBuffer; // UDP data buffer

u16 sampleBuffer[NUM_OF_CHANNELS*MAX_SAMPLES_PER_CHANNEL]; // sample buffer

Debounce Btn0_Db, Btn1_Db; // button sw debounce structures




#define ALL_MUX_DISABLE() XGpio_DiscreteWrite(&Gpio1,1,0x88888888);XGpio_DiscreteWrite(&Gpio1,2,0x88);
#define ALL_MUX_CH_1() XGpio_DiscreteWrite(&Gpio1,1,0x00000000);XGpio_DiscreteWrite(&Gpio1,2,0x00);
#define ALL_MUX_CH_2() XGpio_DiscreteWrite(&Gpio1,1,0x11111111);XGpio_DiscreteWrite(&Gpio1,2,0x11);
#define ALL_MUX_CH_3() XGpio_DiscreteWrite(&Gpio1,1,0x22222222);XGpio_DiscreteWrite(&Gpio1,2,0x22);
#define ALL_MUX_CH_4() XGpio_DiscreteWrite(&Gpio1,1,0x33333333);XGpio_DiscreteWrite(&Gpio1,2,0x33);
#define ALL_MUX_CH_5() XGpio_DiscreteWrite(&Gpio1,1,0x44444444);XGpio_DiscreteWrite(&Gpio1,2,0x44);
#define ALL_MUX_CH_6() XGpio_DiscreteWrite(&Gpio1,1,0x55555555);XGpio_DiscreteWrite(&Gpio1,2,0x55);
#define ALL_MUX_CH_7() XGpio_DiscreteWrite(&Gpio1,1,0x66666666);XGpio_DiscreteWrite(&Gpio1,2,0x66);
#define ALL_MUX_CH_8() XGpio_DiscreteWrite(&Gpio1,1,0x77777777);XGpio_DiscreteWrite(&Gpio1,2,0x77);


#define MUX_SEL_GPIO Gpio1
#define MUX_EN_GPIO Gpio1


#define MUX_ALL_OFF() XGpio_DiscreteWrite(&MUX_EN_GPIO,2,0x3FF);
#define MUX_ALL_ON() 	XGpio_DiscreteWrite(&MUX_EN_GPIO,2,0);


#define MUX_ALL_CH1()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00000000000000000000000000000000);
#define MUX_ALL_CH2()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00001001001001001001001001001001);
#define MUX_ALL_CH3()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00010010010010010010010010010010);
#define MUX_ALL_CH4()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00011011011011011011011011011011);
#define MUX_ALL_CH5()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00100100100100100100100100100100);
#define MUX_ALL_CH6()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00101101101101101101101101101101);
#define MUX_ALL_CH7()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00110110110110110110110110110110);
#define MUX_ALL_CH8()	XGpio_DiscreteWrite(&MUX_SEL_GPIO,1,0b00111111111111111111111111111111);



int main()
{

	int status = 0;

    init_platform();

    print("Hello World\n\r");

	#ifdef DEBUG_PRINT
	xil_printf("Starting Initialization...\n\r");
	#endif


	//GPIO
	status =+ XGpio_Initialize(&Gpio0, XPAR_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&Gpio0, BUTTON_GPIO0_CHANNEL, 0x3); // 2 LSB inputs
	XGpio_SetDataDirection(&Gpio0, LED_GPIO0_CHANNEL, 0x0); // 6 LSB outputs

	status =+ XGpio_Initialize(&Gpio1, XPAR_GPIO_1_DEVICE_ID);
	XGpio_SetDataDirection(&Gpio1, 1, 0x0); // 4 LSB outputs
	XGpio_SetDataDirection(&Gpio1, 2, 0x0); // 4 LSB outputs



	// XADC
	ConfigPtr = XSysMon_LookupConfig(XPAR_SYSMON_0_DEVICE_ID);
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
	status =+ XSysMon_SelfTest(SysMonInstPtr);
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE);
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);
	XSysMon_SetAvg(SysMonInstPtr, XSM_AVG_0_SAMPLES);
	status =+ XSysMon_SetSeqInputMode(SysMonInstPtr, 0x0);
	status =+ XSysMon_SetSeqAcqTime(SysMonInstPtr, XSM_SEQ_CH_AUX01 | XSM_SEQ_CH_AUX09 | XSM_SEQ_CH_AUX06 | XSM_SEQ_CH_AUX15 | XSM_SEQ_CH_AUX05 | XSM_SEQ_CH_AUX13 | XSM_SEQ_CH_AUX12 | XSM_SEQ_CH_AUX00 | XSM_SEQ_CH_AUX08 | XSM_SEQ_CH_VPVN); // Can be changed to speed up
	status =+  XSysMon_SetSeqAvgEnables(SysMonInstPtr, 0x0);
	status =+  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_AUX01 | XSM_SEQ_CH_AUX09 | XSM_SEQ_CH_AUX06 | XSM_SEQ_CH_AUX15 | XSM_SEQ_CH_AUX05 | XSM_SEQ_CH_AUX13 | XSM_SEQ_CH_AUX12 | XSM_SEQ_CH_AUX00 | XSM_SEQ_CH_AUX08 | XSM_SEQ_CH_VPVN);
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 2);
	XSysMon_SetCalibEnables(SysMonInstPtr, XSM_CFR1_CAL_PS_GAIN_OFFSET_MASK | XSM_CFR1_CAL_ADC_GAIN_OFFSET_MASK);
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	//TIMER
	status =+ XTmrCtr_Initialize(TmrCtrInstancePtr, XPAR_TMRCTR_0_DEVICE_ID);
	XTmrCtr_SetOptions(TmrCtrInstancePtr, 0, XTC_AUTO_RELOAD_OPTION);

	//ETHERNET
/*	IP4_ADDR(&ipaddr,  192, 168,   0, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   0,  1);
	IP4_ADDR(&forward_ip,  192, 168,   0, 186);
	lwip_init();
	if (!xemac_add(netifPtr, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
		status++;
	}
	netif_set_default(netifPtr);
	platform_enable_interrupts();
	netif_set_up(netifPtr);

	//UDP
	udpPcb = udp_new();
	udpPort = 8001;
	udp_bind(udpPcb, IP_ADDR_ANY, udpPort);
*/
	//DEBOUNCE
	Debounce_Init(&Btn0_Db, 3);
	Debounce_Init(&Btn1_Db, 10);

	#ifdef MEASURE_TIMING
	u32 Value0;
	u32 Value1;
	u32 Value2;
	u32 Value3;
	#endif

	if(status != 0){
		#ifdef DEBUG_PRINT
		xil_printf("Initialization failed\n\r");
		#endif
		return -1;
	}

	#ifdef DEBUG_PRINT
	xil_printf("Initialization completed successfully\n\r");
	#endif



#define MUX_UNIT 0
#define coraChannel CORA_A0
#define muxShift MUX_UNIT*3


	MUX_ALL_ON();


	while(1){

		MUX_ALL_CH1();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");


		MUX_ALL_CH2();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");



		MUX_ALL_CH3();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");



		MUX_ALL_CH4();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");



		MUX_ALL_CH5();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");


		MUX_ALL_CH6();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");



		MUX_ALL_CH7();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");



		MUX_ALL_CH8();
		usleep(500);
		while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) != XSM_SR_EOS_MASK);
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A0) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A1) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A2) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A3) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A4) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A5) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A10) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A8) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_A6) >> 4));
		xil_printf("%d\t", (XSysMon_GetAdcData(SysMonInstPtr, CORA_VP) >> 4));
		xil_printf("\r\n");

		xil_printf("\r\n");
		xil_printf("\r\n");

		usleep(100000);
		//XGpio_DiscreteWrite(&Gpio1,1,1<<4);
		//usleep(3000000);
		//XGpio_DiscreteWrite(&Gpio1,1,0);
		//usleep(3000000);
	}

    cleanup_platform();
    return 0;
}
