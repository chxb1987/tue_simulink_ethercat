#ifndef RS485_ETHERCAT_LIB
#define RS485_ETHERCAT_LIB


typedef struct{
	uint8 Baudrate;
	uint8 DataFrame;
	uint8 Duplex;
	uint8 SendHandshake;
	uint8 ReceiveHandshake;
}RS485_SETTINGS;


typedef struct{
	boolean TxReady;
	boolean RxSignal;
	boolean ComLock;
	boolean Enable;
}RS485_STATUS;

enum{
	RS485_NO_ERROR    = 0,
	RS485_NOT_OPEN,
	RS485_INVALID_INDEX,
	RS485_INVALID_CHANNEL,
	RS485_NOT_ENABLED,
	RS485_TX_NOT_READY,
	RS485_RX_NO_SIGNAL,
	RS485_COM_LOCKED,
}RS485_RETURN_CODE;

enum{
	RS485_300_BAUD		= 1,
 	RS485_600_BAUD,
  	RS485_1200_BAUD,
	RS485_2400_BAUD,
	RS485_4800_BAUD,
	RS485_9600_BAUD,
	RS485_19200_BAUD,
	RS485_38400_BAUD,
	RS485_57600_BAUD,
	RS485_115200_BAUD
}RS485_BAUDRATE;

enum{
	RS485_7B_EP_1S		= 1,
	RS485_7B_OP_1S,
	RS485_8B_NP_1S,
	RS485_8B_EP_1S,
	RS485_8B_OP_1S,
	RS485_7B_EP_2S,
	RS485_7B_OP_2S,
	RS485_8B_NP_2S,
	RS485_8B_EP_2S,
	RS485_8B_OP_2S
}RS485_DATA_FRAME;


enum{
	XON_XOFF_DISABLE	= 0,
	XON_XOFF_ENABEL
}RS485_HANSHAKE;
  
enum{
	RS485_FULL_DUPLEX		 = 0,
	RS485_HALF_DUPLEX
}RS485_DUPLEX;

#define RS485_CHANNEL_1			0
#define RS485_CHANNEL_2			1

#define RS485_MAX_DATA_LENGTH		22


/* Internal Ethercat master functions, do not use */
void RS485EcOpen(uint32*, uint16 );
void RS485EcClose(void);

/* Ethercat user functions */
uint32 RS485EcProcessStateMachine(uint16);
void RS485EcGetNumberOfDevices(uint16* );
uint32 RS485EcEnable(uint16 , uint16 );
uint32 RS485EcDisable(uint16 , uint16 );
uint32 RS485EcConfig(uint16 , uint16 , RS485_SETTINGS );
uint32 RS485EcTransmit(uint16 , uint16 , uint16 , uint16 *, uint8 *,boolean *);
uint32 RS485EcReceive(uint16 , uint16 , uint16 *, uint8 *);
uint32 RS485EcGetStatus(uint16 , uint16 , RS485_STATUS *);

#endif // RS485_ETHERCAT_LIB
 
