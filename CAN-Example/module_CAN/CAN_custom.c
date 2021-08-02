/**
*******************************************************************************
* @file CAN_custom.c
* @brief FRIWO SDK Source file for custom module CAN
* @author FRIWO
* @date 26.05.2021 - 07:41:33
* <hr>
*******************************************************************************
* COPYRIGHT &copy; 2021 FRIWO GmbH
*******************************************************************************
*/

/**
* @addtogroup CAN_custom
* @{
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* INCLUDES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include "CAN_custom.h"
#include "canApi.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE DEFINES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @brief number of known CAN messages to receive */
#define COMMANDS_AVAILABLE ((uint8_t)(sizeof(msgManagment_array) / sizeof(msgManagement_TypeDef)))

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE TYPEDEF */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @brief define pointer to function for message timeout callback */
typedef void (*FptrOnTimeout)(void);

/** @brief define pointer to function for message receive callback */
typedef void (*FptrOnReceive)(const canApi_MessageTypedef *message);

/**
 * @brief Typedef to map received messages to timeout settings and callback functions.
 * This is part of the helper functions to manage message receival and timeout management.
 */
typedef struct
{
	uint32_t CanIdentifier; /**< @brief Identifier of the received message */
	uint8_t IDE; /**< @brief 0x00u = standard frame identifier, 0x01u = extended frame identifier*/
	int16_t TimeoutCounter; /**< @brief Current timeout counter value, set to negative value to disable timeout */
	int16_t TimeoutReloadValue; /**< @brief Reload counter value. Timeout counter is reset to this value on message receive, set to negative value to disable timeout*/
	FptrOnTimeout TimeoutFunction; /**< @brief pointer to function which is called on message timeout detection */
	FptrOnReceive ReceiveFunction; /**< @brief pointer to function which is called on message receive */
}msgManagement_TypeDef;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE CONSTANTS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

__attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces UInt32 CAN_C_Switch_KilometerToMiles = 0; /* 
	Description: Select between Kilometer and Miles for CAN output;StateList;0=Kilometer;1=Miles; Limits: 0...1 */

__attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces UInt32 CAN_C_SwitchDataInfo_ID_207 = 0; /* 
	Description: CAN-bus Display Data[-];StateList;0 = Boost; 1 = SetpointTorque; 2 = MaxTorque; 3 = MappingTorque; Limits: 0...3 */

__attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces UInt32 CAN_C_SwitchDataInfo_ID_306 = 0; /* 
	Description: CAN-bus Display Data[-];StateList;0 = BateryVoltage; 1 = RemainigDistance; Limits: 0...1 */
	
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces UInt32 CAN_M_ReceivedTestData = 0; /* 
	Description: CAN-bus Test data, received value via CAN in demo code */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE FUNCTION PROTOTYPES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* helper function to manage receive and timeout callbacks */
static msgManagement_TypeDef* GetMessageManagement(const canApi_MessageTypedef *message);
static void HandleMessageTimeouts(void);

/* callback functions for received messages and their timeouts */
static void MessageTimeout0x111(void);
static void MessageReceive0x111(const canApi_MessageTypedef *message);
static void MessageTimeout0x1B6(void);
static void MessageReceive0x1B6(const canApi_MessageTypedef *message);
static void MessageTimeout0x171(void);
static void MessageReceive0x171(const canApi_MessageTypedef *message);
static void MessageTimeout0x172(void);
static void MessageReceive0x172(const canApi_MessageTypedef *message);
static void MessageTimeout0x176(void);
static void MessageReceive0x176(const canApi_MessageTypedef *message);
static void MessageTimeout0x178(void);
static void MessageReceive0x178(const canApi_MessageTypedef *message);
static void MessageTimeout0x310(void);
static void MessageReceive0x310(const canApi_MessageTypedef *message);
static void MessageTimeout0x521(void);
static void MessageReceive0x521(const canApi_MessageTypedef *message);
static void MessageTimeout0x50C(void);
static void MessageReceive0x50C(const canApi_MessageTypedef *message);

static void MessageTimeoutDemo(void);
static void MessageReceiveDemo(const canApi_MessageTypedef *message);


/* functions to send individual predefined messages */
static void MessageSend0x1BF(void); /* PE_Act_05 */
static void MessageSend0x1BD(void); /* MC_Temperature_01 */
static void MessageSend0x1BC(void); /* MC_Errorflags_01 */
static void MessageSend0x2B9(void); /* MC_State_01 */
static void MessageSend0x1BA(void); /* MC_Current_01 */
static void MessageSend0x601(void); /* MC_Prod_Data_01 */
static void MessageSend0x602(void); /* MC_Prod_Data_02 */
static void MessageSend0x603(void); /* MC_Prod_Data_03 */
static void MessageSend0x604(void); /* MC_Prod_Data_04 */
static void MessageSend0x1F0(void); /* MC_APP_01*/
static void MessageSend0x1F1(void); /* MC_APP_02*/
static void MessageSend0x1F2(void); /* MC_APP_03*/
static void MessageSend0x1F4(void); /* MC_APP_04*/
static void MessageSend0x90(void);  /* ICS_Info_01 */
static void MessageSend0x206(void); /* Odo */
static void MessageSend0x207(void); /* Display_01 */
static void MessageSend0x305(void); /* Display_02 */
static void MessageSend0x306(void); /* Display_03 */
static void MessageSend0x209(void); /* Error */
static void MessageSend0x1B5(void); /* Challenge for Immo Unlocking*/
static void MessageSend0x1B7(void); /* Unlock Code sent to GRID-BMS if needed by GRID */
static void MessageSend0x160(void); /* BMS Ctrl 01 */

static void MessageSendFictionalDisplay(void); /* new message for our example */



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		
/**
 * @brief array of commands with their corresponing execution functions
 * All received messages and their timeouts and callbacks must be defined here.
 */
static msgManagement_TypeDef msgManagment_array[] =
{
	/*{Identifier, IDE, TimeoutCounter, TimeoutReloadValue, TimeoutCallback, ReceiveCallback}*/ 
	{0x111, 0, 200, 200,  MessageTimeout0x111, MessageReceive0x111}, /* Message EXT_Torque_Control_01 */
	{0x1B6, 0, 200, 200,  MessageTimeout0x1B6, MessageReceive0x1B6}, /* Message EXT_Immo_Control_01 */
	{0x171, 0, 200, 200,  MessageTimeout0x171, MessageReceive0x171}, /* Message BMS_Info_01 */
	{0x172, 0, 2500, 2500,  MessageTimeout0x172, MessageReceive0x172}, /* Message BMS_Info_02 */
	{0x176, 0, 2500, 2500,  MessageTimeout0x176, MessageReceive0x176}, /* Message BMS_Info_06 */
	{0x178, 0, 2500, 2500,  MessageTimeout0x178, MessageReceive0x178}, /* Message BMS_Info_08 */
	{0x310, 0, 200, 200,  MessageTimeout0x310, MessageReceive0x310}, /* Message Dyno_Act_01 */
	{0x521, 0, 200, 200,  MessageTimeout0x521, MessageReceive0x521}, /* Message ISA_Scale_F1_Current_Sensor */
	{0x50C, 0, 200, 200,  MessageTimeout0x50C, MessageReceive0x50C}, /* Message CAN Display Reset Message */
	{0x600, 0, 500, 500, MessageTimeoutDemo, MessageReceiveDemo}, /* Demo message for display in EnableTool */
};


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE FUNCTIONS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* helper function to manage receive and timeout callbacks */

/**
 * @brief Get the entry with timeout management and callbacks for given message
 * @param message: Pointer to received message
 * @return pointer to entry. Is 0 if no entry available
 */
static msgManagement_TypeDef* GetMessageManagement(const canApi_MessageTypedef *message)
{
	uint8_t i;
	msgManagement_TypeDef* retval = (msgManagement_TypeDef*)0;
	
	for (i = 0; i < COMMANDS_AVAILABLE; i++)
	{
			if (msgManagment_array[i].CanIdentifier == message->Identifier
				&& msgManagment_array[i].IDE == message->IDE)
			{
					retval = &msgManagment_array[i];
			}
	}
	return retval;
}

/**
 * @brief Decrement all message timeout counter values.
 * If a counter reaches 0, the message timout callback function is called
 */
static void HandleMessageTimeouts(void)
{
	uint8_t i;
	
	for (i = 0; i < COMMANDS_AVAILABLE; i++)
	{
		if (msgManagment_array[i].TimeoutCounter > 0)
		{
			msgManagment_array[i].TimeoutCounter--;
		}
		else if (msgManagment_array[i].TimeoutCounter == 0)
		{
			/* call timeout function if counter reaches 0 */
			if (msgManagment_array[i].TimeoutFunction != 0)
			{
				msgManagment_array[i].TimeoutFunction();
			}
			/* set to -1 to avoid calling the timeout callback every millisecond */
			msgManagment_array[i].TimeoutCounter = -1;
		}
		else
		{
			/* do nothing */
		}
	}
}

/* Callbacks to handle receival and timeout management of individual CAN messages. */
/* See our CAN database file (.dbc) for details about our reference implementation */

static void MessageTimeout0x111(void)
{
	canApi_Set_CAN_EXT_Alive_Counter_Timeout(1);
	canApi_Set_CAN_EXT_State_Request_Timeout(1);
	canApi_Set_CAN_EXT_Ride_Mode_Timeout(1);
	canApi_Set_CAN_EXT_ROC_Start_Timeout(1);
	canApi_Set_CAN_EXT_Boost_Enable_Timeout(1);
	canApi_Set_CAN_EXT_Reverse_Gear_Timeout(1);
	canApi_Set_CAN_EXT_Skip_Signal_Checks_Timeout(1);
	canApi_Set_CAN_EXT_Torque_Request_Timeout(1);
	canApi_Set_CAN_EXT_Rotor_Speed_Max_Timeout(1);
}
static void MessageReceive0x111(const canApi_MessageTypedef *message)
{	
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_EXT_Alive_Counter_Timeout(0);
		canApi_Set_CAN_EXT_State_Request_Timeout(0);
		canApi_Set_CAN_EXT_Ride_Mode_Timeout(0);
		canApi_Set_CAN_EXT_ROC_Start_Timeout(0);
		canApi_Set_CAN_EXT_Boost_Enable_Timeout(0);
		canApi_Set_CAN_EXT_Reverse_Gear_Timeout(0);
		canApi_Set_CAN_EXT_Skip_Signal_Checks_Timeout(0);
		canApi_Set_CAN_EXT_Torque_Request_Timeout(0);
		canApi_Set_CAN_EXT_Rotor_Speed_Max_Timeout(0);
		
		canApi_Set_CAN_EXT_Alive_Counter((UInt8)(message->Data[0] & 0x0f));
		canApi_Set_CAN_EXT_State_Request((message->Data[1] & 0x01)>0);
		canApi_Set_CAN_EXT_Ride_Mode((message->Data[2] & 0x03));
		canApi_Set_CAN_EXT_ROC_Start((message->Data[2] & 0x04)>0);
		canApi_Set_CAN_EXT_Boost_Enable((message->Data[2] & 0x08)>0);
		canApi_Set_CAN_EXT_Reverse_Gear((message->Data[2] & 0x10)>0);
		canApi_Set_CAN_EXT_Skip_Signal_Checks((message->Data[2] & 0x20)>0);
		canApi_Set_CAN_EXT_Torque_Request((Float32)((Int16)(message->Data[5]<<8) + (UInt16)message->Data[4])/256);
		canApi_Set_CAN_EXT_Rotor_Speed_Max((Float32)((Int16)(message->Data[7]<<8) + (UInt16)message->Data[6])/32);
	}
}

static void MessageTimeout0x1B6(void)
{
	canApi_Set_CAN_Immo_Unlock_Request_Timeout(1);
}
static void MessageReceive0x1B6(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_Immo_Unlock_Request_Timeout(0);
		canApi_Set_CAN_Immo_Unlock_Request_Higher((UInt32)(message->Data[4]<<0) + (UInt32)(message->Data[5]<<8) + (UInt32)(message->Data[6]<<16) + (UInt32)(message->Data[7]<<24));
		canApi_Set_CAN_Immo_Unlock_Request_Lower((UInt32)(message->Data[0]<<0) + (UInt32)(message->Data[1]<<8) + (UInt32)(message->Data[2]<<16) + (UInt32)(message->Data[3]<<24));
	}
}

static void MessageTimeout0x171(void)
{
	canApi_Set_CAN_BMS_Pack_Voltage_Timeout(1);
	canApi_Set_CAN_BMS_Pack_Current_Timeout(1);
	canApi_Set_CAN_BMS_Errorcode_Timeout(1);
	canApi_Set_CAN_BMS_Charge_Plug_Detection_Timeout(1);
}
static void MessageReceive0x171(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_BMS_Pack_Voltage_Timeout(0);
		canApi_Set_CAN_BMS_Pack_Current_Timeout(0);
		canApi_Set_CAN_BMS_Errorcode_Timeout(0);
		canApi_Set_CAN_BMS_Charge_Plug_Detection_Timeout(0);
		
		canApi_Set_CAN_BMS_Pack_Voltage((Float32)((Int16)(message->Data[1]<<8) + (UInt16)message->Data[0])/128);
		canApi_Set_CAN_BMS_Pack_Current((Float32)((Int16)(message->Data[3]<<8) + (UInt16)message->Data[2])/32);
		canApi_Set_CAN_BMS_Errorcode((UInt32)(message->Data[4]<<0) + (UInt32)(message->Data[5]<<8) + (UInt32)(message->Data[6]<<16) + ((UInt32)(message->Data[7]<<24)&0x3F));
		canApi_Set_CAN_BMS_Charge_Plug_Detection((message->Data[7] & 0xC0)>0);
	}
}
static void MessageTimeout0x172(void)
{
	canApi_Set_CAN_BMS_State_Timeout(1);
	canApi_Set_CAN_BMS_SOC_Timeout(1);
	canApi_Set_CAN_BMS_State_of_Health_Timeout(1);
	canApi_Set_CAN_BMS_Remaining_Capacity_Timeout(1);
	canApi_Set_CAN_BMS_Fullcharge_Capacity_Timeout(1);
}
static void MessageReceive0x172(const canApi_MessageTypedef *message)
{
	Float32 RemainingCapacity = 0;
	Float32 FullchargeCapacity = 0;
	
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_BMS_State_Timeout(0);
		canApi_Set_CAN_BMS_SOC_Timeout(0);
		canApi_Set_CAN_BMS_State_of_Health_Timeout(0);
		canApi_Set_CAN_BMS_Remaining_Capacity_Timeout(0);
		canApi_Set_CAN_BMS_Fullcharge_Capacity_Timeout(0);
		
		canApi_Set_CAN_BMS_State((message->Data[0]<<0) + (message->Data[1]<<8));
		canApi_Set_CAN_BMS_SOC(message->Data[2]);
		canApi_Set_CAN_BMS_State_of_Health(message->Data[3]);
		FullchargeCapacity = (message->Data[6]<<0) + (message->Data[7]<<8);
		RemainingCapacity = (message->Data[4]<<0) + (message->Data[5]<<8);
		
		/* Adaption Grid-Scaling Ah -> mAh */
		if (FullchargeCapacity < 10000)
		{
			/* Grid Scaling */
			canApi_Set_CAN_BMS_Remaining_Capacity(RemainingCapacity * 1000);
			canApi_Set_CAN_BMS_Fullcharge_Capacity(FullchargeCapacity * 1000);
		}
		else
		{
			canApi_Set_CAN_BMS_Remaining_Capacity(RemainingCapacity);
			canApi_Set_CAN_BMS_Fullcharge_Capacity(FullchargeCapacity);
		}
	}
}
static void MessageTimeout0x176(void)
{
	canApi_Set_CAN_BMS_TEMP_Powerstage1_Timeout(1);
	canApi_Set_CAN_BMS_TEMP_Powerstage2_Timeout(1);
	canApi_Set_CAN_BMS_TEMP_MCU_Timeout(1);
	canApi_Set_CAN_BMS_TEMP_Cell1_Timeout(1);
	canApi_Set_CAN_BMS_TEMP_Cell2_Timeout(1);	
}
static void MessageReceive0x176(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_BMS_TEMP_Powerstage1_Timeout(0);
		canApi_Set_CAN_BMS_TEMP_Powerstage2_Timeout(0);
		canApi_Set_CAN_BMS_TEMP_MCU_Timeout(0);
		canApi_Set_CAN_BMS_TEMP_Cell1_Timeout(0);
		canApi_Set_CAN_BMS_TEMP_Cell2_Timeout(0);
		
		canApi_Set_CAN_BMS_TEMP_Powerstage1((Float32)((Int16)(message->Data[1]<<8) + (UInt16)message->Data[0]));
		canApi_Set_CAN_BMS_TEMP_Powerstage2((Float32)((Int16)(message->Data[3]<<8) + (UInt16)message->Data[2]));
		canApi_Set_CAN_BMS_TEMP_MCU((Float32)((Int16)(message->Data[5]<<8) + (UInt16)message->Data[4]));
		canApi_Set_CAN_BMS_TEMP_Cell1((Float32)((Int16)message->Data[6]));
		canApi_Set_CAN_BMS_TEMP_Cell2((Float32)((Int16)message->Data[7]));
	}
}
static void MessageTimeout0x178(void)
{
	canApi_Set_CAN_BMS_Max_Charge_Timeout(1);
	canApi_Set_CAN_BMS_Max_Discharge_Timeout(1);
	canApi_Set_CAN_BMS_Max_Voltage_Timeout(1);
	canApi_Set_CAN_BMS_Min_Voltage_Timeout(1);
	canApi_Set_CAN_BMS_Warning_Status_Timeout(1);
	canApi_Set_CAN_BMS_Pending_HV_Shutdown_Timeout(1);
	canApi_Set_CAN_BMS_Pending_Bordnet_Shutdown_Timeout(1);
	canApi_Set_CAN_BMS_PushButton_ShortPress_Detected_Timeout(1);
	canApi_Set_CAN_BMS_PushButton_LongPress_Detected_Timeout(1);
	canApi_Set_CAN_BMS_PushButton_SuperLongPress_Detected_Timeout(1);
	canApi_Set_CAN_BMS_PushButton_SuperLongPress_Ongoing_Timeout(1);
}
static void MessageReceive0x178(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_BMS_Max_Charge_Timeout(0);
		canApi_Set_CAN_BMS_Max_Discharge_Timeout(0);
		canApi_Set_CAN_BMS_Max_Voltage_Timeout(0);
		canApi_Set_CAN_BMS_Min_Voltage_Timeout(0);
		canApi_Set_CAN_BMS_Warning_Status_Timeout(0);
		canApi_Set_CAN_BMS_Pending_HV_Shutdown_Timeout(0);
		canApi_Set_CAN_BMS_Pending_Bordnet_Shutdown_Timeout(0);
		canApi_Set_CAN_BMS_PushButton_ShortPress_Detected_Timeout(0);
		canApi_Set_CAN_BMS_PushButton_LongPress_Detected_Timeout(0);
		canApi_Set_CAN_BMS_PushButton_SuperLongPress_Detected_Timeout(0);
		canApi_Set_CAN_BMS_PushButton_SuperLongPress_Ongoing_Timeout(0);
		
		canApi_Set_CAN_BMS_Max_Charge(((message->Data[2]<<8) + (message->Data[1]<<0))/64);
		canApi_Set_CAN_BMS_Max_Discharge(((message->Data[4]<<8) + (message->Data[3]<<0))/64);
		canApi_Set_CAN_BMS_Max_Voltage(message->Data[5] << 0);
		canApi_Set_CAN_BMS_Min_Voltage(message->Data[6] << 0);
		canApi_Set_CAN_BMS_Warning_Status((message->Data[7] & 0x01) >> 0);
		canApi_Set_CAN_BMS_Pending_HV_Shutdown((message->Data[7] & 0x02) >> 1);
		canApi_Set_CAN_BMS_Pending_Bordnet_Shutdown((message->Data[7] & 0x04) >> 2);
		canApi_Set_CAN_BMS_PushButton_ShortPress_Detected((message->Data[7] & 0x08) >> 3);
		canApi_Set_CAN_BMS_PushButton_LongPress_Detected((message->Data[7] & 0x10) >> 4);
		canApi_Set_CAN_BMS_PushButton_SuperLongPress_Detected((message->Data[7] & 0x20) >> 5);
		canApi_Set_CAN_BMS_PushButton_SuperLongPress_Ongoing((message->Data[7] & 0x40) >> 6);
	}
}
static void MessageTimeout0x310(void)
{
	canApi_Set_CAN_Dyno_Torque_Timeout(1);
}
static void MessageReceive0x310(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_Dyno_Torque_Timeout(0);
		canApi_Set_CAN_Dyno_Torque((Float32)((Int16)(message->Data[1]<<8) + (UInt16)(message->Data[0]<<0))/32);
	}
}
static void MessageTimeout0x521(void)
{
	canApi_Set_CAN_Dyno_DC_Current_Timeout(1);
	canApi_Set_CAN_Dyno_DC_Voltage_Timeout(1);
	canApi_Set_CAN_Dyno_Elec_Power_Input_Timeout(1);
}
static void MessageReceive0x521(const canApi_MessageTypedef *message)
{
	if (message->DLC == 8u)
	{
		canApi_Set_CAN_Dyno_DC_Current_Timeout(0);
		canApi_Set_CAN_Dyno_DC_Voltage_Timeout(0);
		canApi_Set_CAN_Dyno_Elec_Power_Input_Timeout(0);
		
		canApi_Set_CAN_Dyno_DC_Current((Float32)((Int16) ((Int16)(message->Data[0]<<8) + (UInt16)(message->Data[1]))) *0.02f);
		canApi_Set_CAN_Dyno_DC_Voltage(((Float32)((Int16) ((Int16)(message->Data[2]<<8) + (UInt16)(message->Data[3]))) *0.036f));
		canApi_Set_CAN_Dyno_Elec_Power_Input(((Float32)((Int16) ((Int16)(message->Data[4]<<8) + (UInt16)(message->Data[5]))) *0.01f));
	}
}
static void MessageTimeout0x50C(void)
{
	canApi_Set_CAN_Disp_Reset_Trip_Timeout(1);
}
static void MessageReceive0x50C(const canApi_MessageTypedef *message)
{
	if (message->DLC == 1u)
	{
		canApi_Set_CAN_Disp_Reset_Trip_Timeout(0);
		canApi_Set_CAN_Disp_Reset_Trip(message->Data[0]>0);
	}
}

static void MessageTimeoutDemo(void)
{
	CAN_M_ReceivedTestData = 0;
}
static void MessageReceiveDemo(const canApi_MessageTypedef *message)
{
	uint32_t tmp = 0;
	if (message->DLC == 4u)
	{
		/* copy the data from the CAN frame to the display variable */
		tmp |= message->Data[0];
		tmp |= message->Data[1] << 8;
		tmp |= message->Data[2] << 16;
		tmp |= message->Data[3] << 24;
	}
	CAN_M_ReceivedTestData = tmp;
}


/* PE_Act_05 */
static void MessageSend0x1BF(void)
{
	UInt32 temp_odo_m = 0;
	UInt32 temp_trip_m = 0;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1BF;
	message.Priority = 1;
	message.RTR = 0;
	
	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		temp_odo_m = canApi_Get_INFO_ODO_Total_Kilometers()*621.3711f;
		temp_trip_m = canApi_Get_INFO_ODO_Trip_Kilometers()*621.3711f;
	}
	else
	{
		temp_odo_m = (UInt32)(canApi_Get_INFO_ODO_Total_Kilometers()*1000);
		temp_trip_m = (UInt32)(canApi_Get_INFO_ODO_Trip_Kilometers()*1000);
	}
	
	message.Data[0] = (UInt8)(temp_odo_m);
	message.Data[1] = (UInt8)(temp_odo_m >> 8 );
	message.Data[2] = (UInt8)(temp_odo_m >> 16);
	message.Data[3] = (UInt8)(temp_odo_m >> 24);
	message.Data[4] = (UInt8)(temp_trip_m);
	message.Data[5] = (UInt8)(temp_trip_m >> 8);
	message.Data[6] = (UInt8)(temp_trip_m >> 16);
	message.Data[7] = (UInt8)(temp_trip_m >> 24);
	
	canApi_SendMessage(&message);
}

 /* MC_Temperature_01 */
static void MessageSend0x1BD(void)
{
	canApi_MessageTypedef message;
	message.DLC = 6;
	message.IDE = 0;
	message.Identifier = 0x1BD;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)((UInt32)(canApi_Get_TEMP_FET_Max()*16) >>4);
	message.Data[1] = (UInt8)((UInt32)(canApi_Get_TEMP_FET_Max()*16) >> 12 );
	message.Data[2] = (UInt8)((UInt32)(canApi_Get_TEMP_Motor()*16) >>4);
	message.Data[3] = (UInt8)((UInt32)(canApi_Get_TEMP_Motor()*16) >> 12);
	message.Data[4] = (UInt8)((UInt32)(canApi_Get_TEMP_MCU()*16) >>4);
	message.Data[5] = (UInt8)((UInt32)(canApi_Get_TEMP_MCU()*16) >> 12);
	
	canApi_SendMessage(&message);
}

/* MC_Errorflags_01 */
static void MessageSend0x1BC(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1BC;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(((UInt32)canApi_Get_ERR_Errorcode()));
	message.Data[1] = (UInt8)(((UInt32)canApi_Get_ERR_Errorcode())>>8);
	message.Data[2] = (UInt8)(((UInt32)canApi_Get_ERR_Errorcode())>>16);
	message.Data[3] = (UInt8)(((UInt32)canApi_Get_ERR_Errorcode())>>24);
	message.Data[4] = (UInt8)((UInt32)canApi_Get_ERR_MEM_Trace_0_Errorcode());
	message.Data[5] = (UInt8)(((UInt32)canApi_Get_ERR_MEM_Trace_0_Errorcode())>>8);
	message.Data[6] = (UInt8)(((UInt32)canApi_Get_ERR_MEM_Trace_0_Errorcode())>>16);
	message.Data[7] = (UInt8)(((UInt32)canApi_Get_ERR_MEM_Trace_0_Errorcode())>>24);
	
	canApi_SendMessage(&message);
}

 /* MC_State_01 */
static void MessageSend0x2B9(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x2B9;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)((Int32)(canApi_Get_INFO_Rotor_Speed()/0.025f));
	message.Data[1] = (UInt8)(((Int32)(canApi_Get_INFO_Rotor_Speed()/0.025f)) >> 8 );

	message.Data[2] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current()/0.01f)) >> 0);
	message.Data[3] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current()/0.01f)) >> 8 );

	message.Data[4] = 0;
	message.Data[4] |= (UInt8)((UInt32)canApi_Get_SM_OUT_SYS_Trq_Control()) & 0x01;
	message.Data[4] |= (UInt8)((UInt32)(canApi_Get_ERR_Errorcode()>0) << 1) & 0x02; 
	message.Data[4] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Active() << 2) & 0x04; 
	message.Data[4] |= (UInt8)((UInt32)(canApi_Get_ROC_Result()==1) << 3) & 0x08; 
	message.Data[4] |= (UInt8)((UInt32)(canApi_Get_ROC_Result()==2) << 4) & 0x10; 
	message.Data[4] |= (UInt8)((UInt32)(canApi_Get_SM_PE_Mode_Req_Int()>0) << 5) & 0x20; 
	message.Data[4] |= (UInt8)((UInt32)canApi_Get_APP_Disp_Ride_Mode() << 6) & 0xC0; 

	message.Data[5] = 0;

	message.Data[6] = 0;
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Temp_MCU()) & 0x01;
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Max_Positive_Current() << 1) & 0x02; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Max_Negative_Current() << 2) & 0x04; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_DC_Link_Voltage_Max() << 3) & 0x08; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_DC_Link_Voltage_Min() << 4) & 0x10; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Temp_Motor() << 5) & 0x20; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Temp_FET() << 6) & 0x40; 
	message.Data[6] |= (UInt8)((UInt32)canApi_Get_TRQ_LIM_Derating_Rotor_Speed() << 7) & 0x80; 
	message.Data[7] = 0;
	
	canApi_SendMessage(&message);
}

/* MC_Current_01 */
static void MessageSend0x1BA(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1BA;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current_Iq()/0.01f)) >> 0);
	message.Data[1] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current_Iq()/0.01f)) >> 8 );
	message.Data[2] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current_Id()/0.01f)) >> 0);
	message.Data[3] = (UInt8)(((Int32)(canApi_Get_INFO_Motor_Current_Id()/0.01f)) >> 8);
	message.Data[4] = (UInt8)(((Int32)(canApi_Get_INFO_DC_Current()/0.01f)) >> 0);
	message.Data[5] = (UInt8)(((Int32)(canApi_Get_INFO_DC_Current()/0.01f)) >> 8 );
	message.Data[6] = (UInt8)(((UInt32)(canApi_Get_INFO_Voltage_DC_Link()/0.01f)) >> 0);
	message.Data[7] = (UInt8)(((UInt32)(canApi_Get_INFO_Voltage_DC_Link()/0.01f)) >> 8);
	
	canApi_SendMessage(&message);
}

 /* MC_Prod_Data_01 */
static void MessageSend0x601(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x601;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Release());
	message.Data[1] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Release()>>8);
	message.Data[2] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Release()>>16);
	message.Data[3] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Release()>>24);
	message.Data[4] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Revision());
	message.Data[5] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Revision()>>8);
	message.Data[6] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Revision()>>16);
	message.Data[7] = (UInt8)(canApi_Get_PROD_M_BSW_Ver_Revision()>>24);
	
	canApi_SendMessage(&message);
}
 /* MC_Prod_Data_02 */
static void MessageSend0x602(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x602;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID1());
	message.Data[1] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID1()>>8);
	message.Data[2] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID2());
	message.Data[3] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID2()>>8);
	message.Data[4] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID2()>>16);
	message.Data[5] = (UInt8)(canApi_Get_BSW_C_BSW_ET_Dataset_ID2()>>24);
	message.Data[6] = (UInt8)((canApi_Get_BSW_C_BSW_ET_Dataset_ID3()/1000));
	message.Data[7] = (UInt8)((canApi_Get_BSW_C_BSW_ET_Dataset_ID3()/1000)>>8);
	
	canApi_SendMessage(&message);
}
/* MC_Prod_Data_03 */
static void MessageSend0x603(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x603;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(UInt8)(canApi_Get_PROD_C_HW_Prod_Info_1());
	message.Data[1] = (UInt8)(UInt8)(canApi_Get_PROD_C_HW_Prod_Info_1()>>8);
	message.Data[2] = (UInt8)(UInt8)(canApi_Get_PROD_C_HW_Prod_Info_1()>>16);
	message.Data[3] = (UInt8)(UInt8)(canApi_Get_PROD_C_HW_Prod_Info_1()>>24);
	message.Data[4] = (UInt8)0;
	message.Data[5] = (UInt8)0;
	message.Data[6] = (UInt8)0;
	message.Data[7] = (UInt8)0;
	
	canApi_SendMessage(&message);
}
/* MC_Prod_Data_04 */
static void MessageSend0x604(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x604;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(canApi_Get_PROD_M_HW_ID1());
	message.Data[1] = (UInt8)(canApi_Get_PROD_M_HW_ID1()>>8);
	message.Data[2] = (UInt8)(canApi_Get_PROD_M_HW_ID1()>>16);
	message.Data[3] = (UInt8)(canApi_Get_PROD_M_HW_ID1()>>24);
	message.Data[4] = (UInt8)(canApi_Get_PROD_M_HW_ID2());
	message.Data[5] = (UInt8)(canApi_Get_PROD_M_HW_ID2()>>8);
	message.Data[6] = (UInt8)(canApi_Get_PROD_M_HW_ID2()>>16);
	message.Data[7] = (UInt8)(canApi_Get_PROD_M_HW_ID2()>>24);
	
	canApi_SendMessage(&message);
}
 /* MC_APP_01*/
static void MessageSend0x1F0(void)
{
	UInt32 temp_odo_trip;
	Int32  temp_speed;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1F0;
	message.Priority = 1;
	message.RTR = 0;
	
	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		temp_speed = canApi_Get_INFO_Vehicle_Speed()*62.13711F;
		temp_odo_trip = canApi_Get_INFO_ODO_Trip_Kilometers()*621.3711F;
	}
	else
	{
		temp_speed = canApi_Get_INFO_Vehicle_Speed()*100;
		temp_odo_trip = canApi_Get_INFO_ODO_Trip_Kilometers()*1000;
	}
	
	message.Data[0] = (UInt8)(temp_speed);
	message.Data[1] = (UInt8)(temp_speed >> 8);
	message.Data[2] = (UInt8)canApi_Get_APP_Disp_Ride_Mode();
	message.Data[3] = 0;
	message.Data[4] = (UInt8)(temp_odo_trip);
	message.Data[5] = (UInt8)(temp_odo_trip >> 8);
	message.Data[6] = (UInt8)(temp_odo_trip >> 16);
	message.Data[7] = (UInt8)(temp_odo_trip >> 24);
	
	canApi_SendMessage(&message);
}
 /* MC_APP_02*/
static void MessageSend0x1F1(void)
{
	UInt32 temp_odo_total;
	UInt32 temp_odo_trip;
	UInt16 temp_rem_distance;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1F1;
	message.Priority = 1;
	message.RTR = 0;
	
	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		temp_rem_distance = canApi_Get_INFO_Remaining_Distance()*0.6213711F;
		temp_odo_total = canApi_Get_INFO_ODO_Total_Kilometers()*0.6213711F;
		temp_odo_trip = canApi_Get_INFO_ODO_Trip_Kilometers()*0.6213711F;
	}
	else
	{
		temp_rem_distance = canApi_Get_INFO_Remaining_Distance();
		temp_odo_total = canApi_Get_INFO_ODO_Total_Kilometers();
		temp_odo_trip = canApi_Get_INFO_ODO_Trip_Kilometers();
	}
	
	message.Data[0] = (UInt8)(temp_rem_distance);
	message.Data[1] = (UInt8)(temp_rem_distance >> 8);
	message.Data[2] = (UInt8)canApi_Get_SOC_State_of_Charge();
	message.Data[3] = (UInt8)(temp_odo_total);
	message.Data[4] = (UInt8)(temp_odo_total >> 8);
	message.Data[5] = (UInt8)(temp_odo_total >> 16);
	message.Data[6] = (UInt8)(temp_odo_trip);
	message.Data[7] = (UInt8)(temp_odo_trip >> 8);
	
	canApi_SendMessage(&message);
}

 /* MC_APP_03*/
static void MessageSend0x1F2(void)
{
	
	UInt32 temp_consumption = 0;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1F2;
	message.Priority = 1;
	message.RTR = 0;
	
	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		temp_consumption = canApi_Get_INFO_Consumption_Ave_Trip()*160.9344F;
	}
	else
	{
		temp_consumption = canApi_Get_INFO_Consumption_Ave_Trip()*100;
	}
	
	message.Data[0] = (UInt8)(temp_consumption);
	message.Data[1] = (UInt8)(temp_consumption >> 8);
	message.Data[2] = (UInt8)((UInt32)(canApi_Get_INFO_Ah_Pos()*10));
	message.Data[3] = (UInt8)((UInt32)((canApi_Get_INFO_Ah_Pos()*10)) >> 8);
	message.Data[4] = (UInt8)((UInt32)((canApi_Get_INFO_Ah_Pos()*10)) >> 16);
	message.Data[5] = (UInt8)((UInt32)(canApi_Get_INFO_Ah_Neg()*10));
	message.Data[6] = (UInt8)((UInt32)((canApi_Get_INFO_Ah_Neg()*10)) >> 8);
	message.Data[7] = (UInt8)((UInt32)((canApi_Get_INFO_Ah_Neg()*10)) >> 16);
	
	canApi_SendMessage(&message);
}
/* MC_APP_04*/
static void MessageSend0x1F4(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1F4;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(canApi_Get_APP_Boost_Info());
	message.Data[1] = (UInt8)(canApi_Get_APP_Boost_Avail_Rel());
	message.Data[2] = (UInt8)((UInt32)canApi_Get_APP_Boost_Avail_As());
	message.Data[3] = (UInt8)((UInt32)canApi_Get_APP_Boost_Avail_As()>>8);
	message.Data[4] = (UInt8)(0);
	message.Data[5] = (UInt8)(0);
	message.Data[6] = (UInt8)(0);
	message.Data[7] = (UInt8)(0);
	
	canApi_SendMessage(&message);
}
  /* ICS_Info_01 */
static void MessageSend0x90(void)
{
	static UInt16 ICS_Counter=0;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x90;
	message.Priority = 1;
	message.RTR = 0;
	
	ICS_Counter = (ICS_Counter+1)%16;
	message.Data[0] = (UInt8) ICS_Counter;
	message.Data[1] = (UInt8)((Int32)(canApi_Get_INFO_DC_Current()*-32));
	message.Data[2] = (UInt8)(((Int32)(canApi_Get_INFO_DC_Current()*-32))>>8);
	message.Data[3] = (UInt8)(((Int32)(canApi_Get_INFO_Voltage_DC_Link()*64)));
	message.Data[4] = (UInt8)((UInt8)(((Int32)(canApi_Get_INFO_Voltage_DC_Link()*64))>>8) & 0x3F);

	message.Data[5] = (UInt8)(1); // ICS_Sensor_Status 1 (ICS Ready)
	message.Data[5] |= (UInt8)(1<<5); // ICS_Sensor_Type 2 (Emerge 3000)
	message.Data[6] = (UInt8)(30);
	message.Data[7] = (UInt8)(30);
	
	canApi_SendMessage(&message);
}
/* Odo */
static void MessageSend0x206(void)
{
	
	UInt32 total_km = canApi_Get_INFO_ODO_Total_Kilometers();
	UInt32 temp_odo_km = 0;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x206;
	message.Priority = 1;
	message.RTR = 0;

	if(CAN_C_Switch_KilometerToMiles == 1)
	{	
		if (canApi_Get_INFO_ODO_Total_Kilometers() >= 65536)
		{
			temp_odo_km = (UInt32)(((UInt32)(total_km*0.6213711F))%65536);
		}
		else
		{
			temp_odo_km = (UInt32)(((UInt32)(total_km*0.6213711F)));
		}
	}
	else
	{
		if (total_km >= 65536)
		{
			temp_odo_km = (UInt32)(((UInt32)total_km)%65536);
		}
		else
		{
			temp_odo_km = (UInt32)(((UInt32)total_km));
		}
	}
	
	message.Data[0] = (UInt8)0;
	message.Data[1] = (UInt8)0;
	message.Data[2] = (UInt8)temp_odo_km;
	message.Data[3] = (UInt8)(temp_odo_km>>8);
	message.Data[4] = (UInt8)0;
	message.Data[5] = (UInt8)0;
	message.Data[6] = (0);
	message.Data[7] = (0);
	
	canApi_SendMessage(&message);
}
/* Display_01 */
static void MessageSend0x207(void)
{
	UInt32 temp_odo_total;
	UInt32 temp_gear;
	Int32  temp_vref;
	UInt8 temp_Boost_Bar_Info = 0;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x207;
	message.Priority = 1;
	message.RTR = 0;
	
	if(canApi_Get_SM_OUT_SYS_Trq_Control())
	{ 
		if (canApi_Get_TRQ_DES_Driver_Reverse_Gear())
		{
			temp_gear = 0xB;
		}
		else
		{
			temp_gear = canApi_Get_APP_Disp_Ride_Mode() + 1;
		}
	}
	else
	{
		temp_gear = 0xA; // Dashed Line
	}

	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		if (canApi_Get_INFO_Vehicle_Speed()>=0)
		{
			temp_vref = canApi_Get_INFO_Vehicle_Speed()*6.213711F;
		}
		else
		{
			temp_vref = canApi_Get_INFO_Vehicle_Speed()*-6.213711F;
		}
		temp_odo_total = canApi_Get_INFO_ODO_Trip_Kilometers()*6.213711F;
	}
	else
	{
		if (canApi_Get_INFO_Vehicle_Speed()>=0)
		{
			temp_vref = canApi_Get_INFO_Vehicle_Speed()*10;
		}
		else
		{
			temp_vref = canApi_Get_INFO_Vehicle_Speed()*-10;
		}
		temp_odo_total = canApi_Get_INFO_ODO_Trip_Kilometers()*10.0F;
		switch((UInt32)CAN_C_SwitchDataInfo_ID_207)
		{
			case 0:
				temp_Boost_Bar_Info = (UInt8)canApi_Get_APP_Boost_Avail_Rel();
				break;
			
			case 1:
				temp_Boost_Bar_Info = (UInt8)canApi_Get_INFO_Rel_Torque_Setpoint();
				break;
			
			case 2:
				temp_Boost_Bar_Info = (UInt8)canApi_Get_INFO_Rel_Torque_Max();
				break;
			
			case 3:
				temp_Boost_Bar_Info = (UInt8)canApi_Get_INFO_Rel_Torque_Mapping();
				break;
			
			default:
				temp_Boost_Bar_Info = (UInt8)canApi_Get_APP_Boost_Avail_Rel();
				break;
			
		}
	}
	message.Data[0] = (UInt8)0;
	message.Data[1] = (UInt8)8;
	message.Data[2] = (UInt8)temp_gear;

	message.Data[3] = (UInt8)(((((UInt8)CAN_C_Switch_KilometerToMiles)<<7) & 0x80) | (temp_Boost_Bar_Info & 0x7F));
	message.Data[4] = (UInt8)(temp_vref);
	message.Data[5] = (UInt8)(temp_vref >> 8);
	message.Data[6] = (UInt8)(temp_odo_total>>0);
	message.Data[7] = (UInt8)(temp_odo_total>>8);
	
	canApi_SendMessage(&message);
}
/* Display_02 */
static void MessageSend0x305(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x305;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)((Int32)(canApi_Get_TEMP_Combined_Max_Rel()/0.1f));
	message.Data[1] = (UInt8)(((Int32)(canApi_Get_TEMP_Combined_Max_Rel()/0.1f)) >> 8 );
	message.Data[2] = (UInt8)0;
	message.Data[3] = (UInt8)0;
	message.Data[4] = (UInt8)0;
	message.Data[5] = (UInt8)0;
	message.Data[6] = (UInt8)0;
	message.Data[7] = (UInt8)0;
	
	canApi_SendMessage(&message);
}
/* Display_03 */
static void MessageSend0x306(void)
{
	Int32  temp_remaining_dis;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x306;
	message.Priority = 1;
	message.RTR = 0;
	
	if(CAN_C_Switch_KilometerToMiles == 1)
	{
		temp_remaining_dis = canApi_Get_INFO_Remaining_Distance()*62.13711F;
	}
	else
	{
		temp_remaining_dis = canApi_Get_INFO_Remaining_Distance()*100.0F;
	}
	
	message.Data[0] = (UInt8)0;
	message.Data[1] = (UInt8)0;
	message.Data[2] = (UInt8)canApi_Get_SOC_State_of_Charge();
	message.Data[3] = (UInt8)0;
	message.Data[4] = (UInt8)0;
	if(CAN_C_SwitchDataInfo_ID_306 == 0)
	{
		message.Data[5] = (UInt8)((Int32)(canApi_Get_INFO_Voltage_DC_Link()*100.0F) >> 0);
		message.Data[6] = (UInt8)((Int32)(canApi_Get_INFO_Voltage_DC_Link()*100.0F) >> 8);
	}
	else
	{
	 message.Data[5] = (UInt8)(temp_remaining_dis >> 0);
	 message.Data[6] = (UInt8)(temp_remaining_dis >> 8);
	}
	message.Data[7] = (UInt8)0;
	
	canApi_SendMessage(&message);
}
/* Error */
static void MessageSend0x209(void)
{
	UInt8 temp_Error_code;
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x209;
	message.Priority = 1;
	message.RTR = 0;
	
	switch (canApi_Get_ERR_Errorcode())
	{
	case 1:
		temp_Error_code = 1;
		break;
		
	case 2:
		temp_Error_code = 2;
		break;

	case 4:
		temp_Error_code = 3;
		break;

	case 8:
		temp_Error_code = 4;
		break;

	case 16:
		temp_Error_code = 5;
		break;

	case 32:
		temp_Error_code = 6;
		break;

	case 64:
		temp_Error_code = 7;
		break;

	case 128:
		temp_Error_code = 8;
		break;

	case 256:
		temp_Error_code = 9;
		break;

	case 512:
		temp_Error_code = 10;
		break;

	case 1024:
		temp_Error_code = 11;
		break;

	case 2048:
		temp_Error_code = 12;
		break;

	case 4096:
		temp_Error_code = 13;
		break;

	case 8192:
		temp_Error_code = 14;
		break;

	case 16384:
		temp_Error_code = 15;
		break;

	case 32768:
		temp_Error_code = 16;
		break;

	case 65536:
		temp_Error_code = 17;
		break;

	case 131072:
		temp_Error_code = 18;
		break;

	case 262144:
		temp_Error_code = 19;
		break;

	case 524288:
		temp_Error_code = 20;
		break;

	case 1048576:
		temp_Error_code = 21;
		break;

	case 2097152:
		temp_Error_code = 22;
		break;

	case 4194304:
		temp_Error_code = 23;
		break;

	case 8388608:
		temp_Error_code = 24;
		break;

	case 16777216:
		temp_Error_code = 25;
		break;

	case 33554432:
		temp_Error_code = 26;
		break;

	case 67108864:
		temp_Error_code = 27;
		break;

	case 134217728:
		temp_Error_code = 28;
		break;

	case 268435456:
		temp_Error_code = 29;
		break;

	case 536870912:
		temp_Error_code = 30;
		break;

	case 1073741824:
		temp_Error_code = 31;
		break;

	default:
		temp_Error_code = 0;
		break;

	}
	message.Data[0] = (UInt8)(temp_Error_code);
	message.Data[1] = (UInt8)0;
	message.Data[2] = (UInt8)0;
	message.Data[3] = (UInt8)0;
	message.Data[4] = (UInt8)0;
	message.Data[5] = (UInt8)0;
	message.Data[6] = (UInt8)0;
	message.Data[7] = (UInt8)0;
	
	canApi_SendMessage(&message);
}
/* Challenge for Immo Unlocking*/
static void MessageSend0x1B5(void)
{
	UInt32 BSW_Immo_Challenge_Lower = canApi_Get_BSW_Immo_Challenge_Lower();
	UInt32 BSW_Immo_Challenge_Higher = canApi_Get_BSW_Immo_Challenge_Higher();
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1B5;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(BSW_Immo_Challenge_Lower>>0);
	message.Data[1] = (UInt8)(BSW_Immo_Challenge_Lower>>8);
	message.Data[2] = (UInt8)(BSW_Immo_Challenge_Lower>>16);
	message.Data[3] = (UInt8)(BSW_Immo_Challenge_Lower>>24);
	message.Data[4] = (UInt8)(BSW_Immo_Challenge_Higher>>0);
	message.Data[5] = (UInt8)(BSW_Immo_Challenge_Higher>>8);
	message.Data[6] = (UInt8)(BSW_Immo_Challenge_Higher>>16);
	message.Data[7] = (UInt8)(BSW_Immo_Challenge_Higher>>24);
	
	canApi_SendMessage(&message);
}
/* Unlock Code sent to GRID-BMS if needed by GRID */
static void MessageSend0x1B7(void)
{
	UInt32 BSW_BMS_Unlock_Code_Lower = canApi_Get_BSW_BMS_Unlock_Code_Lower();
	UInt32 BSW_BMS_Unlock_Code_Higher = canApi_Get_BSW_BMS_Unlock_Code_Higher();
	
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x1B7;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(BSW_BMS_Unlock_Code_Lower>>0);
	message.Data[1] = (UInt8)(BSW_BMS_Unlock_Code_Lower>>8);
	message.Data[2] = (UInt8)(BSW_BMS_Unlock_Code_Lower>>16);
	message.Data[3] = (UInt8)(BSW_BMS_Unlock_Code_Lower>>24);
	message.Data[4] = (UInt8)(BSW_BMS_Unlock_Code_Higher>>0);
	message.Data[5] = (UInt8)(BSW_BMS_Unlock_Code_Higher>>8);
	message.Data[6] = (UInt8)(BSW_BMS_Unlock_Code_Higher>>16);
	message.Data[7] = (UInt8)(BSW_BMS_Unlock_Code_Higher>>24);
	
	canApi_SendMessage(&message);
}
/* BMS Ctrl 01 */
static void MessageSend0x160(void)
{
	canApi_MessageTypedef message;
	message.DLC = 8;
	message.IDE = 0;
	message.Identifier = 0x160;
	message.Priority = 1;
	message.RTR = 0;
	
	message.Data[0] = (UInt8)(canApi_Get_SM_BMS_Control_State()); // BMS_State Discharge
	message.Data[1] = (UInt8)0;
	message.Data[2] = (UInt8)0;
	message.Data[3] = (UInt8)0;
	message.Data[4] = (UInt8)0;
	message.Data[5] = (UInt8)0;
	message.Data[6] = (UInt8)0;
	message.Data[7] = (UInt8)0;
	
	canApi_SendMessage(&message);
}

static void MessageSendFictionalDisplay(void)
{
	UInt32 temp_trip_m = 0;
	Int32  temp_speed;
	
	canApi_MessageTypedef message;
	message.DLC = 8; /* 8 bytes of data in the CAN frame */
	message.IDE = 1; /* use extended identifier */
	message.Identifier = 0x1FFFFF00; /* CAN identifier */
	message.Priority = 1;
	message.RTR = 0;
	
	temp_trip_m = canApi_Get_INFO_ODO_Trip_Kilometers();
	temp_speed = canApi_Get_INFO_Vehicle_Speed();
	
	message.Data[0] = (UInt8)(temp_trip_m); /* first four byte contain current milage */
	message.Data[1] = (UInt8)(temp_trip_m >> 8);
	message.Data[2] = (UInt8)(temp_trip_m >> 16);
	message.Data[3] = (UInt8)(temp_trip_m >> 24);
	message.Data[4] = (UInt8)(temp_speed); /* last four byte contain current speed */
	message.Data[5] = (UInt8)(temp_speed >> 8);
	message.Data[6] = (UInt8)(temp_speed >> 16);
	message.Data[7] = (UInt8)(temp_speed >> 24);
	
	canApi_SendMessage(&message);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC FUNCTIONS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief Initialize the user implementation layer. Setup the API buffer structure.
 * This function is called once when the firmware initialized the CAN peripheral.
 */
void canApi_UserInitCallBack(void)
{
	/* Init the buffer structure. We use a ringbuffer implementation in this example */
	canApi_SetupBuffer(RINGBUFFER, RINGBUFFER);
	canApi_ClearTransmitBuffer();
	canApi_ClearReceiveBuffer();

	/* Set filter */	
	canApi_FilterSetFourStdIdListMode(FilterBank01, 0x111, 0, 0x1B6, 0, 0x171, 0, 0x172, 0);
	canApi_FilterSetFourStdIdListMode(FilterBank02, 0x176, 0, 0x178, 0, 0x310, 0, 0x521, 0);
	canApi_FilterSetTwoStdIdListMode(FilterBank03, 0x50C, 0, 0x0, 0);
	
	canApi_FilterSetOneStdIdListMode(FilterBank04,0x600,0);
}

/**
 * @brief Callback called every 1ms.
 * The user can check the CAN input buffer for received messages and can put messages to sent into the output buffer.
 */
void canApi_UserPeriodicCallBack(void)
{
	canApi_MessageTypedef message;
	
	static uint16_t timeslot = 0; /* used to control tx message intervals */
	
	/* get all messages from the input buffer */
	while(canApi_ReceiveMessage(&message) == CAN_OK)
	{
		/* check if we have a callback for the received message */
		msgManagement_TypeDef* msgManagement = GetMessageManagement(&message);
		
		if (msgManagement != 0)
		{
			/* reset timeout counter to reload value */
			msgManagement->TimeoutCounter = msgManagement->TimeoutReloadValue;
			
			/* call the callback function if defined */
			if (msgManagement->ReceiveFunction != 0)
			{
				msgManagement->ReceiveFunction(&message);
			}
		}
	}
	
	/* check if a registered message has a timeout and call the corresponding callback */
	HandleMessageTimeouts();
	
	
	/* send periodic messages */
	
	if (timeslot % 10 == 0)
	{
		/* send messages in 10 millisecond interval */
		MessageSend0x160(); /* BMS Ctrl 01 */
		MessageSend0x90();  /* ICS_Info_01 */
		MessageSend0x1BA(); /* MC_Current_01 */
		MessageSend0x1BC(); /* MC_Errorflags_01 */
		MessageSend0x2B9(); /* MC_State_01 */
	}
	
	if (timeslot % 100 == 0)
	{
		/* send messages in 100 millisecond interval */
		MessageSend0x1B5(); /* Challenge for Immo Unlocking*/
		MessageSend0x1B7(); /* Unlock Code sent to GRID-BMS if needed by GRID */
		MessageSend0x1BF(); /* PE_Act_05 */
		MessageSend0x1F0(); /* MC_APP_01*/
		MessageSend0x1F4(); /* MC_APP_04*/
		MessageSend0x206(); /* Odo */
		MessageSend0x207(); /* Display_01 */
		MessageSend0x209(); /* Error */
		MessageSend0x305(); /* Display_02 */
		MessageSend0x306(); /* Display_03 */
	}
	
	if (timeslot % 1000 == 0)
	{
		/* send messages in 1000 millisecond interval */
		MessageSend0x1BD(); /* MC_Temperature_01 */
		MessageSend0x1F1(); /* MC_APP_02*/
		MessageSend0x1F2(); /* MC_APP_03*/
		MessageSend0x601(); /* MC_Prod_Data_01 */
		MessageSend0x602(); /* MC_Prod_Data_02 */
		MessageSend0x603(); /* MC_Prod_Data_03 */
		MessageSend0x604(); /* MC_Prod_Data_04 */
		
		MessageSendFictionalDisplay(); /* Send the data to our fictional display */
	}
	
	timeslot++;
	
	return;
}




/** @} */
