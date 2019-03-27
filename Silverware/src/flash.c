
#include "project.h"
#include "drv_fmc.h"
#include "defines.h"

#if defined(USE_BEESIGN)
#include "beesign.h"
#include "stick_command.h"
#endif

extern int fmc_erase( void );
extern void fmc_unlock(void);
extern void fmc_lock(void);

extern float accelcal[];
extern float * pids_array[3];

extern float hardcoded_pid_identifier;


#define FMC_HEADER 0x12AA0001

float initial_pid_identifier = -10;
float saved_pid_identifier;


float flash_get_hard_coded_pid_identifier( void) {
	float result = 0;

	for (int i=0;  i<3 ; i++) {
		for (int j=0; j<3 ; j++) {
			result += pids_array[i][j] * (i+1) * (j+1) * 0.932f;
		}
	}
	return result;
}


void flash_hard_coded_pid_identifier( void)
{
 initial_pid_identifier = flash_get_hard_coded_pid_identifier();
}




void flash_save( void) {

    fmc_unlock();
	fmc_erase();
	
	unsigned long addresscount = 0;

    writeword(addresscount++, FMC_HEADER);
   
	fmc_write_float(addresscount++, initial_pid_identifier );
	
	for (int i=0;  i<3 ; i++) {
		for (int j=0; j<3 ; j++) {
            fmc_write_float(addresscount++, pids_array[i][j]);
		}
	}
 

    fmc_write_float(addresscount++, accelcal[0]);
    fmc_write_float(addresscount++, accelcal[1]);
    fmc_write_float(addresscount++, accelcal[2]);

   
#ifdef RX_BAYANG_PROTOCOL_TELEMETRY_AUTOBIND
// autobind info     
extern char rfchannel[4];
extern char rxaddress[5];
extern int telemetry_enabled;
extern int rx_bind_enable;
    
 // save radio bind info  
    if ( rx_bind_enable )
    {
    writeword(50, rxaddress[4]|telemetry_enabled<<8);
    writeword(51, rxaddress[0]|(rxaddress[1]<<8)|(rxaddress[2]<<16)|(rxaddress[3]<<24));
    writeword(52, rfchannel[0]|(rfchannel[1]<<8)|(rfchannel[2]<<16)|(rfchannel[3]<<24));
    }
    else
    {
      // this will leave 255's so it will be picked up as disabled  
    }
#endif  


#ifdef SWITCHABLE_FEATURE_1
extern int flash_feature_1;

//save filter cut info

if (flash_feature_1)
{
	fmc_write_float (53,1);	
}else{
	fmc_write_float (53,0);	
}
#endif

#ifdef SWITCHABLE_FEATURE_2
extern int flash_feature_2;

//save LVC info

if (flash_feature_2)
{
	fmc_write_float (54,1);	
}else{
	fmc_write_float (54,0);	
}
#endif

#ifdef SWITCHABLE_FEATURE_3
extern int flash_feature_3;

//save LVC info

if (flash_feature_3)
{
	fmc_write_float (55,1);	
}else{
	fmc_write_float (55,0);	
}
#endif

#if defined(RX_DSMX_2048) || defined(RX_DSM2_1024)
extern int rx_bind_enable;
if ( rx_bind_enable ){
		fmc_write_float (56,1);
	}else{
		fmc_write_float (56,0);	
	}
#endif


#if defined(USE_BEESIGN)
    unsigned long beesign_temp;
    beesign_temp = bsDevice.blankFlg;
    beesign_temp <<= 8;
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.vtx.channel;
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.vtx.power;
    writeword(57, beesign_temp);
    beesign_temp = bsDevice.osd.voltagePosition;
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.osd.rssiPosition;
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.osd.namePosition;
    writeword(58, beesign_temp);
    beesign_temp = bsDevice.others.name[0];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[1];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[2];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[3];
    writeword(59, beesign_temp);
    beesign_temp = bsDevice.others.name[4];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[5];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[6];
    beesign_temp <<= 8;
    beesign_temp |= bsDevice.others.name[7];
    writeword(60, beesign_temp);
    beesign_temp = rcCmdArm;
    beesign_temp <<= 8;
    beesign_temp |= rcCmdAlti;
    beesign_temp <<= 8;
    beesign_temp |= rcCmdLevel;
    beesign_temp <<= 8;
    beesign_temp |= rcCmdRace;
    writeword(61, beesign_temp);
    beesign_temp = rcCmdHorizon;
    writeword(62, beesign_temp);
#endif
    writeword(255, FMC_HEADER);
    
	fmc_lock();
}



void flash_load( void) {

	unsigned long addresscount = 0;
// check if saved datav is present
    if (FMC_HEADER == fmc_read(addresscount++)&& FMC_HEADER == fmc_read(255))
    {

     saved_pid_identifier = fmc_read_float(addresscount++);
// load pids from flash if pid.c values are still the same       
     if (  saved_pid_identifier == initial_pid_identifier )
     {
         for (int i=0;  i<3 ; i++) {
            for (int j=0; j<3 ; j++) {
                pids_array[i][j] = fmc_read_float(addresscount++);
            }
        }
     }
     else{
         addresscount+=9; 
     }    

    accelcal[0] = fmc_read_float(addresscount++ );
    accelcal[1] = fmc_read_float(addresscount++ );
    accelcal[2] = fmc_read_float(addresscount++ );  

       
 #ifdef RX_BAYANG_PROTOCOL_TELEMETRY_AUTOBIND  
extern char rfchannel[4];
extern char rxaddress[5];
extern int telemetry_enabled;
extern int rx_bind_load;
extern int rx_bind_enable;
     
 // save radio bind info   

    int temp = fmc_read(52);
    int error = 0;
    for ( int i = 0 ; i < 4; i++)
    {
        if ( ((temp>>(i*8))&0xff  ) > 127)
        {
            error = 1;
        }   
    }
    
    if( !error )   
    {
        rx_bind_load = rx_bind_enable = 1; 
        
        rxaddress[4] = fmc_read(50);

        telemetry_enabled = fmc_read(50)>>8;
        int temp = fmc_read(51);
        for ( int i = 0 ; i < 4; i++)
        {
            rxaddress[i] =  temp>>(i*8);        
        }
        
        temp = fmc_read(52);  
        for ( int i = 0 ; i < 4; i++)
        {
            rfchannel[i] =  temp>>(i*8);  
        }
    }
#endif

#ifdef SWITCHABLE_FEATURE_1
	extern int flash_feature_1;
	flash_feature_1 = fmc_read_float(53);
#endif

#ifdef SWITCHABLE_FEATURE_2
	extern int flash_feature_2;
	flash_feature_2 = fmc_read_float(54);
#endif

#ifdef SWITCHABLE_FEATURE_3
	extern int flash_feature_3;
	flash_feature_3 = fmc_read_float(55);
#endif

#if defined(RX_DSMX_2048) || defined(RX_DSM2_1024)
	extern int rx_bind_enable;
	rx_bind_enable = fmc_read_float(56);
#endif

#if defined(USE_BEESIGN)
    unsigned long beesign_temp;
    beesign_temp = fmc_read(57);
    bsDevice.blankFlg = beesign_temp >> 24;
    bsDevice.vtx.channel = beesign_temp >> 8;
    bsDevice.vtx.power = beesign_temp;
    beesign_temp = fmc_read(58);
    bsDevice.osd.voltagePosition = beesign_temp >> 16;
    bsDevice.osd.rssiPosition = beesign_temp >> 8;
    bsDevice.osd.namePosition = beesign_temp;
    beesign_temp = fmc_read(59);
    bsDevice.others.name[0] = beesign_temp >> 24;
    bsDevice.others.name[1] = beesign_temp >> 16;
    bsDevice.others.name[2] = beesign_temp >> 8;
    bsDevice.others.name[3] = beesign_temp;
    beesign_temp = fmc_read(60);
    bsDevice.others.name[4] = beesign_temp >> 24;
    bsDevice.others.name[5] = beesign_temp >> 16;
    bsDevice.others.name[6] = beesign_temp >> 8;
    bsDevice.others.name[7] = beesign_temp;
    beesign_temp = fmc_read(61);
    rcCmdArm = beesign_temp >> 24;
    rcCmdAlti = beesign_temp >> 16;
    rcCmdLevel = beesign_temp >> 8;
    rcCmdRace = beesign_temp;
    beesign_temp = fmc_read(62);
    rcCmdHorizon = beesign_temp;
#endif

    }
    else
    {
        
    }
    
}













