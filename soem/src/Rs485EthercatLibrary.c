
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>


#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatcoe.h"
#include "ethercatfoe.h"
#include "ethercatconfig.h"
#include "ethercatdc.h"

#include "Rs485EthercatLibrary.h"

#define RS485_ID		0x17863052


#define RS485_CONTROL_INDEX		0
#define RS485_STATUS_INDEX		0
#define RS485_LENGTH_INDEX		1
#define RS485_DATA_INDEX		2

#define RS485_NUMBER_OF_CHANNEL_PER_DEVICE		2
#define RS485_MAX_NUMBER_OF_TRIES				20

#define RS485_PRINT_DEBUG_INFO			0
#define RS485_PRINT_DEBUG_INFO_EX		0



enum{
	RS485_INACTIVE   = 0,
 	RS485_INIT,
  	RS485_RUN
}RS485_STATES;

enum{
	RS485_ISTATE_0 = 0,
	RS485_ISTATE_1,
	RS485_ISTATE_2,
	RS485_ISTATE_3
}RS485_ISTATES;
 
  

typedef struct{
	uint8 *InputData;
	uint8 *OutputData;
	boolean TxReady;
	boolean RxSignal;
	uint8 ReqState;
	uint8 State;
	uint8 Istate;
	uint8 Tries;
	boolean ComLock;
	boolean Enable;
}RS485_EC_CONTROL;

typedef struct{
	RS485_EC_CONTROL Control[RS485_NUMBER_OF_CHANNEL_PER_DEVICE];
	uint16 EthercatSlaveIndex;
}RS485_EC_HANDLE;


RS485_EC_HANDLE *RS485EcHandles;
uint16 G_NumberOfRS485Ec = 0;


void RS485EcOpen(uint32* RS485SlaveIds, uint16 NumberOfRS485){
	int i,cnt;

#if RS485_PRINT_DEBUG_INFO
	printf("RS485EcOpen\n");
#endif
	
	if(G_NumberOfRS485Ec){
		printf("RS485 Error: Driver already open\n");
		return;
	}

	G_NumberOfRS485Ec = NumberOfRS485;

	RS485EcHandles = malloc(G_NumberOfRS485Ec * sizeof(RS485_EC_HANDLE));
	memset(RS485EcHandles, 0, sizeof(RS485_EC_HANDLE));

	for( i = 0;i < NumberOfRS485 ;i++)
	{
        cnt = RS485SlaveIds[i];
        RS485EcHandles[i].Control[RS485_CHANNEL_1].OutputData = ec_slave[cnt].outputs;
        RS485EcHandles[i].Control[RS485_CHANNEL_1].InputData = ec_slave[cnt].inputs;
        RS485EcHandles[i].Control[RS485_CHANNEL_2].OutputData = ec_slave[cnt].outputs + (ec_slave[cnt].Obytes / 2);
        RS485EcHandles[i].Control[RS485_CHANNEL_2].InputData = ec_slave[cnt].inputs + (ec_slave[cnt].Ibytes / 2);

        RS485EcHandles[i].EthercatSlaveIndex = cnt;
	}
}
 
void RS485EcGetNumberOfDevices(uint16 *NumberOfRS485){
	*NumberOfRS485 = G_NumberOfRS485Ec;
}

void RS485EcClose(void){
	
	if(!G_NumberOfRS485Ec){
		printf("RS485 Error: Driver not open\n");
		return;
	}
		
	G_NumberOfRS485Ec = 0;
	free(RS485EcHandles);
}

uint32 RS485EcConfig(uint16 RS485EcIndex, uint16 Channel, RS485_SETTINGS Settings){
	
	uint16 index = (Channel == RS485_CHANNEL_2) ? 0x8010 : 0x8000;
		
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	
	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = ~0;

	ec_SDOwrite(RS485EcHandles[RS485EcIndex].EthercatSlaveIndex, index, 0x02, FALSE, sizeof(Settings.SendHandshake), &Settings.SendHandshake, EC_TIMEOUTRXM);
	ec_SDOwrite(RS485EcHandles[RS485EcIndex].EthercatSlaveIndex, index, 0x03, FALSE, sizeof(Settings.ReceiveHandshake), &Settings.ReceiveHandshake, EC_TIMEOUTRXM);
	ec_SDOwrite(RS485EcHandles[RS485EcIndex].EthercatSlaveIndex, index, 0x06, FALSE, sizeof(Settings.Duplex), &Settings.Duplex, EC_TIMEOUTRXM);
	ec_SDOwrite(RS485EcHandles[RS485EcIndex].EthercatSlaveIndex, index, 0x11, FALSE, sizeof(Settings.Baudrate), &Settings.Baudrate, EC_TIMEOUTRXM);
	ec_SDOwrite(RS485EcHandles[RS485EcIndex].EthercatSlaveIndex, index, 0x15, FALSE, sizeof(Settings.DataFrame), &Settings.DataFrame, EC_TIMEOUTRXM);
	
	RS485EcHandles[RS485EcIndex].Control[Channel].State = RS485_INACTIVE;
	RS485EcHandles[RS485EcIndex].Control[Channel].ReqState = RS485_INIT;
	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = 0;
	return RS485_NO_ERROR;
}

uint32 RS485EcEnable(uint16 RS485EcIndex, uint16 Channel){
	
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(RS485EcIndex >= G_NumberOfRS485Ec) return RS485_INVALID_INDEX;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	
	RS485EcHandles[RS485EcIndex].Control[Channel].Enable = 1;
	
	return RS485_NO_ERROR;
}

uint32 RS485EcDisable(uint16 RS485EcIndex, uint16 Channel){
	
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(RS485EcIndex >= G_NumberOfRS485Ec) return RS485_INVALID_INDEX;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	
	RS485EcHandles[RS485EcIndex].Control[Channel].Enable = 0;
	
	return RS485_NO_ERROR;
}


uint32 RS485EcTransmit(uint16 RS485EcIndex, uint16 Channel, uint16 BytesToWrite, uint16 *BytesWritten, uint8 *TransmitData, boolean *FlagTx){
	int i;
	
	*BytesWritten = 0;
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(RS485EcIndex >= G_NumberOfRS485Ec) return RS485_INVALID_INDEX;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	if(!RS485EcHandles[RS485EcIndex].Control[Channel].Enable) return RS485_NOT_ENABLED;
	if(!RS485EcHandles[RS485EcIndex].Control[Channel].TxReady) return RS485_TX_NOT_READY;
	if(RS485EcHandles[RS485EcIndex].Control[Channel].ComLock) return RS485_COM_LOCKED;
#if RS485_PRINT_DEBUG_INFO
	printf("------> Transmit data device %i, Channel %i\n", RS485EcIndex, Channel + 1);
	
#endif

    //-GJ: write flag transmit succeeded-//
    *FlagTx = RS485EcHandles[RS485EcIndex].Control[Channel].TxReady;
    //-Einde-//

	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = ~0;
	if(BytesToWrite > RS485_MAX_DATA_LENGTH) BytesToWrite = RS485_MAX_DATA_LENGTH;
	
	for(i = 0; i < BytesToWrite; i++)
		RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[i + RS485_DATA_INDEX] =  TransmitData[i]; 
	
	RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[RS485_CONTROL_INDEX] = RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[RS485_CONTROL_INDEX] ^ 0x01; // Control channel 1, Transmit request
	RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[RS485_LENGTH_INDEX] = BytesToWrite; // datalength channel 1
	
	RS485EcHandles[RS485EcIndex].Control[Channel].TxReady = 0;
	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = 0;
	
	*BytesWritten = BytesToWrite;
#if RS485_PRINT_DEBUG_INFO
	printf("------> Number of bytes in ec tranamit : %i\n", *BytesWritten );
#endif   
	return RS485_NO_ERROR;
}

uint32 RS485EcReceive(uint16 RS485EcIndex, uint16 Channel, uint16 *BytesRead, uint8 *ReceiveData){
	
	int i;
	
	*BytesRead = 0;
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(RS485EcIndex >= G_NumberOfRS485Ec) return RS485_INVALID_INDEX;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	if(!RS485EcHandles[RS485EcIndex].Control[Channel].Enable) return RS485_NOT_ENABLED;
	if(!RS485EcHandles[RS485EcIndex].Control[Channel].RxSignal) return RS485_RX_NO_SIGNAL;
	if(RS485EcHandles[RS485EcIndex].Control[Channel].ComLock) return RS485_COM_LOCKED;
#if RS485_PRINT_DEBUG_INFO
	printf("------> Receive data device %i, Channel %i\n", RS485EcIndex, Channel + 1);
#endif
	
	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = ~0;
	*BytesRead = RS485EcHandles[RS485EcIndex].Control[Channel].InputData[RS485_LENGTH_INDEX];
    if(*BytesRead > RS485_MAX_DATA_LENGTH)
        *BytesRead = RS485_MAX_DATA_LENGTH;
#if RS485_PRINT_DEBUG_INFO
	printf("------> Number of bytes in ec packet : %i\n", *BytesRead );
#endif    
	for(i = 0; i < *BytesRead; i++)
		ReceiveData[i] = RS485EcHandles[RS485EcIndex].Control[Channel].InputData[i + RS485_DATA_INDEX]; 
    
	RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[RS485_CONTROL_INDEX] = RS485EcHandles[RS485EcIndex].Control[Channel].OutputData[RS485_CONTROL_INDEX] ^ 0x02; // Control channel 1, Transmit request
	
	RS485EcHandles[RS485EcIndex].Control[Channel].RxSignal = 0;
	RS485EcHandles[RS485EcIndex].Control[Channel].ComLock = 0;
	
	
	return RS485_NO_ERROR;
}

uint32 RS485EcGetStatus(uint16 RS485EcIndex, uint16 Channel, RS485_STATUS *Status){
	if(!G_NumberOfRS485Ec) return RS485_NOT_OPEN;
	if(RS485EcIndex >= G_NumberOfRS485Ec) return RS485_INVALID_INDEX;
	if(Channel > RS485_CHANNEL_2) return RS485_INVALID_CHANNEL;
	
	Status->RxSignal = RS485EcHandles[RS485EcIndex].Control[Channel].RxSignal;
	Status->TxReady = RS485EcHandles[RS485EcIndex].Control[Channel].TxReady;
	Status->ComLock = RS485EcHandles[RS485EcIndex].Control[Channel].ComLock;
	Status->Enable = RS485EcHandles[RS485EcIndex].Control[Channel].Enable;

	return RS485_NO_ERROR;
}


uint32 RS485EcProcessStateMachine(uint16 RS485Index){

	int j, k;
#if RS485_PRINT_DEBUG_INFO_EX
	uint16 transmit_accepted;
	uint16 receive_request;
	uint16 init_accepted;
	uint16 buffer_full;
	uint16 parity_error;
	uint16 framing_error;
	uint16 overrun_error;
	uint16 input_length;
#endif
    if(RS485Index >= G_NumberOfRS485Ec){
        return RS485_INVALID_INDEX;
    }
    for(k = 0; k < RS485_NUMBER_OF_CHANNEL_PER_DEVICE; k++){
        if(RS485EcHandles[RS485Index].Control[k].Enable && !RS485EcHandles[RS485Index].Control[k].ComLock){

            RS485EcHandles[RS485Index].Control[k].TxReady  = (RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x01) == (RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX] & 0x01);
            RS485EcHandles[RS485Index].Control[k].RxSignal = (RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x02) != (RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX] & 0x02);      
            
#if RS485_PRINT_DEBUG_INFO_EX
                    // analyze status channel 1
            transmit_accepted   = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x01; 
            receive_request     = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x02;
            init_accepted       = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x04;
            buffer_full         = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x08;
            parity_error        = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x10;
            framing_error       = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x20;
            overrun_error       = RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x40;

                                    // Receive datalength channel 1
            input_length        = RS485EcHandles[RS485Index].Control[k].InputData[RS485_LENGTH_INDEX];

            printf("**** Control Device %i, Channel %i ****\n", RS485Index, k + 1);
            printf("   transmit_accepted : %d\n",transmit_accepted);
            printf("   receive_request : %d\n",receive_request);
            printf("   init_accepted : %d\n",init_accepted);
            printf("   buffer_full : %d\n",buffer_full);
            printf("   parity_error : %d\n",parity_error);
            printf("   framing_error : %d\n",framing_error);
            printf("   overrun_error : %d\n",overrun_error);
            printf("   input_length : %d\n",input_length);

            printf("     ReqState %i\n", RS485EcHandles[RS485Index].Control[k].ReqState);
            printf("     status     %i\n", RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX]);
            printf("     command    %i\n", RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX]);
            printf("     TxReady  %i\n", RS485EcHandles[RS485Index].Control[k].TxReady);
            printf("     RxSignal %i\n", RS485EcHandles[RS485Index].Control[k].RxSignal);
#endif


            if((RS485EcHandles[RS485Index].Control[k].ReqState == RS485_INIT) && (RS485EcHandles[RS485Index].Control[k].State == RS485_INACTIVE)){
                switch(RS485EcHandles[RS485Index].Control[k].Istate){
                    case RS485_ISTATE_0:
                        for(j = RS485_DATA_INDEX; j < RS485_MAX_DATA_LENGTH + RS485_DATA_INDEX; j++){
                            RS485EcHandles[RS485Index].Control[k].OutputData[j] = 0x00; // data channel 1
                        }
#if RS485_PRINT_DEBUG_INFO
                        printf("              Init Request channel %i\n", k + 1);
#endif
                        RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX] = 0x04;
                        RS485EcHandles[RS485Index].Control[k].OutputData[RS485_LENGTH_INDEX] = 0;
                        RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_1;
                        RS485EcHandles[RS485Index].Control[k].Tries = 0;
                        break;
                    case RS485_ISTATE_1:
                        RS485EcHandles[RS485Index].Control[k].Tries++;
                        if(RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x04){
#if RS485_PRINT_DEBUG_INFO
                            printf("              Init Request Accepted channel %i\n", k + 1);
#endif
                            RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_3;
                        }
                        if(RS485EcHandles[RS485Index].Control[k].Tries > RS485_MAX_NUMBER_OF_TRIES){
#if RS485_PRINT_DEBUG_INFO
                            printf("              Too many Tries channel %i\n", k + 1);
#endif
                            RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_2;
                        }
                        break;
                    case RS485_ISTATE_2:
                        RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX] = 0x00;
                        RS485EcHandles[RS485Index].Control[k].ReqState = RS485_INACTIVE;
                        RS485EcHandles[RS485Index].Control[k].State = RS485_INACTIVE;
                        RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_0;
                        break;
                    case RS485_ISTATE_3:
                        RS485EcHandles[RS485Index].Control[k].ReqState = RS485_RUN;
                        RS485EcHandles[RS485Index].Control[k].State = RS485_INIT;
                        RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_0;
                        RS485EcHandles[RS485Index].Control[k].Tries = 0;
                        break;
                }
            }
            if((RS485EcHandles[RS485Index].Control[k].ReqState == RS485_RUN) && (RS485EcHandles[RS485Index].Control[k].State == RS485_INIT)){
                switch(RS485EcHandles[RS485Index].Control[k].Istate){
                    case RS485_ISTATE_0:
#if RS485_PRINT_DEBUG_INFO
                        printf("              Clear Init Request channel %i\n", k + 1);
#endif
                        RS485EcHandles[RS485Index].Control[k].OutputData[RS485_CONTROL_INDEX] = 0x00;
                        RS485EcHandles[RS485Index].Control[k].OutputData[RS485_LENGTH_INDEX] = 0;
                        RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_1;
                        RS485EcHandles[RS485Index].Control[k].Tries = 0;
                        break;
                    case RS485_ISTATE_1:
                        RS485EcHandles[RS485Index].Control[k].Tries++;
                        if(!(RS485EcHandles[RS485Index].Control[k].InputData[RS485_STATUS_INDEX] & 0x04)){
#if RS485_PRINT_DEBUG_INFO
                            printf("              Init Request Accepted Released channel %i\n", k + 1);
#endif
                            RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_3;
                        }
                        if(RS485EcHandles[RS485Index].Control[k].Tries > RS485_MAX_NUMBER_OF_TRIES){
#if RS485_PRINT_DEBUG_INFO
                            printf("              Too many Tries channel %i\n", k + 1);
#endif
                            RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_2;
                        }
                        break;
                    case RS485_ISTATE_2:
                        RS485EcHandles[RS485Index].Control[k].ReqState = RS485_INIT;
                        RS485EcHandles[RS485Index].Control[k].State = RS485_INIT;
                        RS485EcHandles[RS485Index].Control[k].Istate = 0;
                        break;
                    case RS485_ISTATE_3:
                        RS485EcHandles[RS485Index].Control[k].ReqState = RS485_RUN;
                        RS485EcHandles[RS485Index].Control[k].State = RS485_RUN;
                        RS485EcHandles[RS485Index].Control[k].Istate = RS485_ISTATE_0;
                        RS485EcHandles[RS485Index].Control[k].Tries = 0;
                        break;
                }
            }
        }
    }
    return RS485_NO_ERROR;
}
