#include <LUFA/Drivers/Peripheral/Serial.h>
#include "SCSI.h"

typedef PROGMEM struct _FAT_BOOT_RECORD
{
		uint8_t		bootstrap[3];			//eb 3c 90
		uint8_t		OEM[8];
		uint16_t	iBytesPerSector;		//512
		uint8_t		iSectorsPerCluster;		//1
		uint16_t	iReservedSectors;		//1
		uint8_t		iFATs;					//2
		uint16_t	iRootEntries;
		uint16_t 	iTotalSectors;
		uint8_t		iMediaDescr;
		uint16_t	iSectorsPerFAT;
		uint16_t	iSectorsPerTrack;
		uint16_t	iHeads;
		uint32_t	iHiddenSectors;
		uint32_t	iTotalSectorsEx;
		uint16_t	iLogicDriveNumber;
		uint8_t		extSignature;			//29 (hex)
		uint32_t	serialNumber;
		uint8_t		volumeLabel[11];
		uint8_t		fatName[8];				//FAT16
		//uint8_t		exeCode[448];
		//uint8_t		exeEndMarker[2];		//55 aa
} FAT_BOOT_RECORD; //total 512 bytes

//Boot Record: 1 sector; FAT1: 1 sector; FAT2: 1 sector; Root Directory: 1 sector
FAT_BOOT_RECORD PROGMEM fatBootData = 
{
		.bootstrap          = {0xeb, 0x3c, 0x90},
		.OEM				= "OpenPCR ",
		.iBytesPerSector 	= 512,
		.iSectorsPerCluster = 1,
		.iReservedSectors	= 1,
		.iFATs				= 2,
		.iRootEntries		= 512,
		.iTotalSectors		= 4352,
		.iMediaDescr		= 0xf0,
		.iSectorsPerFAT		= 17,
		.iSectorsPerTrack	= 0,
		.iHeads				= 0,
		.iHiddenSectors		= 0,
		.iTotalSectorsEx	= 0,
		.iLogicDriveNumber	= 0x00, //0x80,
		.extSignature		= 0x29,
		.serialNumber		= USE_INTERNAL_SERIAL,
		.volumeLabel        = "OPENPCR    ",
		.fatName			= "FAT16   ",
	//	.exeCode			= {},
	//	.exeEndMarker		= {0x55, 0xaa}
};

 typedef PROGMEM struct
{
		uint8_t		filename[8];
		uint8_t		ext[3];
		uint8_t		attribute;
		uint8_t		reserved;
		uint8_t		create_time_ms;
		uint16_t	create_time;
		uint16_t	create_date;
		uint16_t	access_date;
		uint16_t	first_cluster_highorder;
		uint16_t	modified_time;
		uint16_t	modified_date;
		uint16_t	first_cluster_loworder;
		uint32_t	size;
} FAT_ROOT_DIRECTORY; //total 512 bytes

/*date format: bits 0-4: day of month 1-31; bits 5-8: month of year 1-12; bits 9-15: years since 1980 0-127
  time format: bits 0-4: 2 second count 0-29; bits 5-10: minutes 0-59; bits 11-15: hours 0-23  */
FAT_ROOT_DIRECTORY PROGMEM volumeLabel = 
{
		.filename          			= "OPENPCR ",
		.ext						= "   ",
		.attribute 					= 0x08,
		.reserved 					= 0,
		.create_time_ms				= 0,
		.create_time				= 0,
		.create_date				= 0,
		.access_date				= 0,
		.first_cluster_highorder	= 0,
		.modified_time				= 0,
		.modified_date				= 0,
		.first_cluster_loworder		= 0,
		.size						= 0
};

FAT_ROOT_DIRECTORY PROGMEM fileName = 
{
		.filename          			= "STATUS  ",
		.ext						= "TXT",
		.attribute 					= 0,
		.reserved 					= 0,
		.create_time_ms				= 0,
		.create_time				= 0x8800,
		.create_date				= 0x3ea1,
		.access_date				= 0x3ea1,
		.first_cluster_highorder	= 0,
		.modified_time				= 0x8800,
		.modified_date				= 0x3ea1,
		.first_cluster_loworder		= 2,
		.size						= 300
};

#define START_CODE				0xFF
#define PACKET_HEADER_LENGTH	4

typedef enum{
	SEND_CMD		= 0x10,
	STATUS_REQ		= 0x40,
	STATUS_RESP		= 0x80	
}PACKET_TYPE;

#define FILE_SIGNATURE		"s=ACGTC"
#define FILE_SIGNATURE_LEN	7
#define FILE_MAX_LENGTH		252

bool DoReadFlowControl() {
	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearIN();

		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		  return false;
	}

	return true;
}

bool DoWriteFlowControl() {
	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearOUT();

		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		  return false;
	}

	return true;
}

bool SerialWaitUntilReady(){
	//timeout in 50000*60/16M (clock frequency) = 0.1875 secs
	uint16_t timeout = 60;
	
	TCNT1 = 0;
	while(timeout){
		if (Serial_IsCharReceived())
			return true;
		if (TCNT1 >= 50000){ 
			TCNT1 = 0;
			timeout--;
		}
	}

	return false;
}

/*
void Endpoint_Write_Zeros(uint16_t size){
	uint16_t block_index = 0;

	while(block_index < size){
		DoReadFlowControl();
		for(uint16_t blockdiv16_index = 0; blockdiv16_index < 1; blockdiv16_index++) {
			Endpoint_Write_Byte(0x00);
			block_index++;
			if (block_index >= size)
				return;
		}
	}
}
*/

void FlushBlock(uint16_t size)
{
	uint16_t block_index;
	for (block_index=0; block_index<size; block_index++){
		DoWriteFlowControl();
		Endpoint_Read_Byte();
	}
}

bool DataManager_ReadBlocks(uint32_t BlockAddress, uint16_t TotalBlocks)
{
	uint16_t block_index = 0;

	while (TotalBlocks){
		if (BlockAddress == 0){ //BOOT Record
			for (block_index=0;block_index<62; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(pgm_read_byte(((char*)&fatBootData)+block_index));
			}
			for (block_index=0; block_index<448; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(0x00);
			}
			DoReadFlowControl();
			Endpoint_Write_Byte(0x55);
			Endpoint_Write_Byte(0xaa);				
		}
		else if (BlockAddress == 1 || BlockAddress == 18){ //FAT TABLE 1 and 2 (15 blocks)
			//first two sectors are not used
			DoReadFlowControl();
			Endpoint_Write_Byte(0xf0);
			Endpoint_Write_Byte(0xff);
			Endpoint_Write_Byte(0xff);
			Endpoint_Write_Byte(0xff);
			//sector 2 where data start
			Endpoint_Write_Byte(0xff);
			Endpoint_Write_Byte(0xff);
			
			for (block_index=6; block_index<VIRTUAL_MEMORY_BLOCK_SIZE; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(0x00);
			}
		}
		else if (BlockAddress == 35){	//Root Directory
			for (block_index=0;block_index<32; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(pgm_read_byte(((char*)&volumeLabel)+block_index));
			}
			for (block_index=0;block_index<32; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(pgm_read_byte(((char*)&fileName)+block_index));
			}
			for (block_index=64; block_index<VIRTUAL_MEMORY_BLOCK_SIZE; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(0x00);
			}
		}
		else if (BlockAddress == 67){	//STATUS.TXT
			Serial_TxByte(START_CODE);
			Serial_TxByte(PACKET_HEADER_LENGTH);
			Serial_TxByte(0);
			Serial_TxByte(STATUS_REQ);

			uint16_t length = 0;
			uint8_t  packet_type;
			bool success = true;
			while(true){
				while ((success = SerialWaitUntilReady()) && Serial_RxByte() != START_CODE);
				if (!success) break;
				if (!(success = SerialWaitUntilReady())) break;
				length = Serial_RxByte();
				if (!(success = SerialWaitUntilReady())) break;
				length |= (Serial_RxByte() << 8);
				if (length > PACKET_HEADER_LENGTH){
					length -= PACKET_HEADER_LENGTH;
					if (!(success = SerialWaitUntilReady())) break;
					packet_type = Serial_RxByte();
					if (packet_type == STATUS_RESP)
						break;
				}
			}

			block_index = 0;
			if (success){
				for (; block_index<length; block_index++){
					if (!SerialWaitUntilReady()) break;
					DoReadFlowControl();
					Endpoint_Write_Byte(Serial_RxByte());
				}
			}

			for (; block_index<VIRTUAL_MEMORY_BLOCK_SIZE; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(0x00);
			}
		}
		else{
			for (block_index=0; block_index<VIRTUAL_MEMORY_BLOCK_SIZE; block_index++){
				DoReadFlowControl();
				Endpoint_Write_Byte(0x00);
			}
		}
	
		BlockAddress++;
		TotalBlocks--;
	}

	if (!(Endpoint_IsReadWriteAllowed()))
		Endpoint_ClearIN();

	return true;
}

bool DataManager_WriteBlocks(uint32_t BlockAddress, uint16_t TotalBlocks)
{
	uint16_t block_index;
	bool bSignatureFound = false;
	while(TotalBlocks){
		if (BlockAddress > 67 && bSignatureFound == false){ //in data sector
			bSignatureFound = true;
			for (block_index=0; block_index<FILE_SIGNATURE_LEN; block_index++){
				DoWriteFlowControl();
				if (Endpoint_Read_Byte() != FILE_SIGNATURE[block_index]){
					FlushBlock(VIRTUAL_MEMORY_BLOCK_SIZE-block_index-1);
					bSignatureFound = false;
					break;
				}
			}
			if (bSignatureFound){
				uint16_t length = PACKET_HEADER_LENGTH+FILE_MAX_LENGTH;
				Serial_TxByte(START_CODE);
				Serial_TxByte(length & 0xff);
				Serial_TxByte((length & 0xff00) >> 8);
				Serial_TxByte(SEND_CMD);
				for (block_index=0; block_index<FILE_MAX_LENGTH; block_index++){
					DoWriteFlowControl();
					Serial_TxByte(Endpoint_Read_Byte());
				}
				FlushBlock(VIRTUAL_MEMORY_BLOCK_SIZE-FILE_MAX_LENGTH-FILE_SIGNATURE_LEN);
			}
		}
		else{
			FlushBlock(VIRTUAL_MEMORY_BLOCK_SIZE);
		}
		BlockAddress++;
		TotalBlocks--;
	}

	if (!(Endpoint_IsReadWriteAllowed()))
		Endpoint_ClearOUT();

	return true;
}

