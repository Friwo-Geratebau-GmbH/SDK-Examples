/**
*******************************************************************************
* @file canApi.h
* @brief -
* @author Jonas_Lovermann
* @date 07.05.2021 - 12:38:17
* <hr>
*******************************************************************************
* COPYRIGHT &copy; 2021 FRIWO GmbH
*******************************************************************************
*/

#ifndef CANAPI_H_
#define CANAPI_H_

/**
* @addtogroup canApi
* @{
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* INCLUDES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdint.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC DEFINES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC TYPEDEF */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef unsigned char Bool; /* boolean basetype */
typedef float Float32; /* 32 bit floating-point basetype */
typedef double Float64; /* 64 bit floating-point basetype */
typedef signed short int Int16; /* 16 bit signed integer basetype */
typedef signed long int Int32; /* 32 bit signed integer basetype */
typedef signed char Int8; /* 8 bit signed integer basetype */
typedef unsigned short int UInt16; /* 16 bit unsigned integer basetype */
typedef unsigned long int UInt32; /* 32 bit unsigned integer basetype */
typedef unsigned char UInt8; /* 8 bit unsigned integer basetype */


/* Public function return value definition */

/**
 * @brief CanApi Status Typedef
 * reports status of input/output functions
 */
typedef enum
{
    CAN_OK = ((uint8_t)0x00),
    CAN_BUFFER_FULL = ((uint8_t)0x01),
    CAN_BUFFER_EMPTY = ((uint8_t)0x02),
    CAN_INVALID_VALUE = ((uint8_t)0x03),
    CAN_ERROR = ((uint8_t)0x04)
}canApi_StatusTypeDef;


/* Input/Output message definition*/

/**
 * @brief Represents a can message with payload
 */
typedef struct
{
    uint32_t Identifier; /**< @brief Specifies the extended identifier*/
    uint8_t Priority; /**< @brief priority of the message. Negative value is treated as no message in buffer slot */
    uint8_t IDE; /**< Specifies the type of identifier to be used. 0x00u = standard frame identifier, 0x01u = extended frame identifier*/
    uint8_t RTR; /**< Specifies the type of frame for the message. 0x00u = Data frame, 0x01u = remote transmission request frame */
    uint8_t DLC; /**< @brief length of the actual payload */
    uint8_t Data[8]; /**< @brief buffer for the payload */
}canApi_MessageTypedef;

/* Setup helper argument */

/**
 * @brief Buffer type selection for input and output buffer
 */
typedef enum
{
    RINGBUFFER = ((uint8_t)0x00), /**< @brief Standard ringbuffer FIFO implementation */
    PRIORITYBUFFER_QUEUE = ((uint8_t)0x01), /**< @brief Queue with individual message priorities */
    PRIORITYBUFFER_REPLACE = ((uint8_t)0x02) /**< @brief like prio queue, but replace existing queue members when message with same id added */
}buffer_BufferType;


typedef enum canApi_FilterBank {
	FilterBank01 = 1,
	FilterBank02 = 2,
	FilterBank03 = 3,
	FilterBank04 = 4,
	FilterBank05 = 5,
	FilterBank06 = 6,
	FilterBank07 = 7,
	FilterBank08 = 8,
	FilterBank09 = 9,
	FilterBank10 = 10,
	FilterBank11 = 11,
	FilterBank12 = 12,
	FilterBank13 = 13,
	FilterBank14 = 14,
	FilterBank15 = 15,
	FilterBank16 = 16,
	FilterBank17 = 17,
	FilterBank18 = 18,
	FilterBank19 = 19,
	FilterBank20 = 20,
	FilterBank21 = 21,
	FilterBank22 = 22,
	FilterBank23 = 23,
	FilterBank24 = 24,
	FilterBank25 = 25,
	FilterBank26 = 26
} canApi_FilterBank_Type;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC CONSTANTS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC FUNCTION PROTOTYPES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* user-side CAN buffer access functions */

/* Utility Functions */

/**
 * @brief Initialize buffers and set the buffer type implementation
 * @param receiveBufferType: Buffer type of CAN receive buffer
 * @param transmitBufferType: Buffer type of CAN transmit buffer
 */
void canApi_SetupBuffer(buffer_BufferType receiveBufferType, buffer_BufferType transmitBufferType);

/**
 * @brief Clear the content of the transmit buffer
 */
void canApi_ClearTransmitBuffer(void);

/**
 * @brief Clear the content of the receive buffer
 */
void canApi_ClearReceiveBuffer(void);

/**
 * @brief Puts a message into the buffer to send it using the CAN peripheral.
 * @param message: message to send
 * @return Status of the transmit buffer system
 */
canApi_StatusTypeDef canApi_SendMessage(const canApi_MessageTypedef *message);

/**
 * @brief Get a message from the CAN receive buffer system.
 * @param message: Target pointer to store the received message
 * @retval Status of the receive buffer system
 */
canApi_StatusTypeDef canApi_ReceiveMessage(canApi_MessageTypedef *message);

/**
 * @brief User code implementation of CAN API module, called every 1ms.
 */
void canApi_UserPeriodicCallBack(void);

/**
 * @brief User init implementation of CAN API module, called by CAN hardware init
 */
void canApi_UserInitCallBack(void);


/**
 * @brief This function deactivates an active setting on a specified filter bank
 * 
 * @param FilterBank Number of the filter bank which should be deactivated
 */
void canApi_FilterDeactivateFilterBank(canApi_FilterBank_Type FilterBank);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for one standard identifier in mask mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame) 
 * @param Mask_1 Masking of the relavant and irrelevant bits of the specified first standard identifier
 * @param RTR_Mask_1 Masking of the remote transmission request of the first standard identifier if relevant or not
 */
void canApi_FilterSetOneStdIdMaskMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1, uint16_t Mask_1, uint8_t RTR_Mask_1);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for two standard identifiers in mask mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame) 
 * @param Mask_1 Masking of the relavant and irrelevant bits of the specified first standard identifier
 * @param RTR_Mask_1 Masking of the remote transmission request of the first standard identifier if relevant or not
 * @param StdId_2 Second standard identifier which should be filtered
 * @param RTR_StdId_2 Remote Transmission Request for the second standard identifier (set 1 if StdId_2 is remote frame) 
 * @param Mask_2 Masking of the relavant and irrelevant bits of the specified second standard identifier
 * @param RTR_Mask_2 Masking of the remote transmission request of the second standard identifier if relevant or not
 */
void canApi_FilterSetTwoStdIdMaskMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1, uint16_t Mask_1, uint8_t RTR_Mask_1, uint16_t StdId_2, uint8_t RTR_StdId_2, uint16_t Mask_2, uint8_t RTR_Mask_2);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for one standard identifier in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame) 
 */
void canApi_FilterSetOneStdIdListMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for two standard identifiers in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame)
 * @param StdId_2 Second standard identifier which should be filtered
 * @param RTR_StdId_2 Remote Transmission Request for the second standard identifier (set 1 if StdId_2 is remote frame)
 */
void canApi_FilterSetTwoStdIdListMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1,uint16_t StdId_2,uint8_t RTR_StdId_2);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for three standard identifiers in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame)
 * @param StdId_2 Second standard identifier which should be filtered
 * @param RTR_StdId_2 Remote Transmission Request for the second standard identifier (set 1 if StdId_2 is remote frame)
 * @param StdId_3 Third standard identifier which should be filtered
 * @param RTR_StdId_3 Remote Transmission Request for the third standard identifier (set 1 if StdId_3 is remote frame)
 */
void canApi_FilterSetThreeStdIdListMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1,uint16_t StdId_2,uint8_t RTR_StdId_2,uint16_t StdId_3,uint8_t RTR_StdId_3);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for four standard identifiers in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame)
 * @param StdId_2 Second standard identifier which should be filtered
 * @param RTR_StdId_2 Remote Transmission Request for the second standard identifier (set 1 if StdId_2 is remote frame)
 * @param StdId_3 Third standard identifier which should be filtered
 * @param RTR_StdId_3 Remote Transmission Request for the third standard identifier (set 1 if StdId_3 is remote frame)
 * @param StdId_4 Fourth standard identifier which should be filtered
 * @param RTR_StdId_4 Remote Transmission Request for the fourth standard identifier (set 1 if StdId_4 is remote frame)
 */
void canApi_FilterSetFourStdIdListMode(canApi_FilterBank_Type FilterBank,uint16_t StdId_1,uint8_t RTR_StdId_1,uint16_t StdId_2,uint8_t RTR_StdId_2,uint16_t StdId_3,uint8_t RTR_StdId_3,uint16_t StdId_4,uint8_t RTR_StdId_4);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for one extended identifier in mask mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param ExtId_1 First extended identifier which should be filtered
 * @param RTR_ExtId_1 Remote Transmission Request for the first extended identifier (set 1 if ExtId_1 is remote frame)
 * @param Mask_1 Masking of the relavant and irrelevant bits of the specified first extended identifier
 * @param RTR_Mask_1 Masking of the remote transmission request of the first extended identifier if relevant or not
 */
void canApi_FilterSetOneExtIdMaskMode(canApi_FilterBank_Type FilterBank,uint32_t ExtId_1,uint8_t RTR_ExtId_1, uint32_t Mask_1,uint8_t RTR_Mask_1);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for one extended identifier in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param ExtId_1 First extended identifier which should be filtered
 * @param RTR_ExtId_1 Remote Transmission Request for the first extended identifier (set 1 if ExtId_1 is remote frame)
 */
void canApi_FilterSetOneExtIdListMode(canApi_FilterBank_Type FilterBank, uint32_t ExtId_1,uint8_t RTR_ExtId_1);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for two extended identifier in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param ExtId_1 First extended identifier which should be filtered
 * @param RTR_ExtId_1 Remote Transmission Request for the first extended identifier (set 1 if ExtId_1 is remote frame)
 * @param ExtId_2 Second extended identifier which should be filtered
 * @param RTR_ExtId_2 Remote Transmission Request for the second extended identifier (set 1 if ExtId_2 is remote frame)
 */
void canApi_FilterSetTwoExtIdListMode(canApi_FilterBank_Type FilterBank, uint32_t ExtId_1,uint8_t RTR_ExtId_1, uint32_t ExtId_2,uint8_t RTR_ExtId_2);

/**
 * @brief This function sets a specified filter bank of the receive CAN filter for one extended identifier and one standard identifier in list mode.
 * 
 * @param FilterBank Number of the filter bank which should be set
 * @param StdId_1 First standard identifier which should be filtered
 * @param RTR_StdId_1 Remote Transmission Request for the first standard identifier (set 1 if StdId_1 is remote frame)
 * @param ExtId_1 First extended identifier which should be filtered
 * @param RTR_ExtId_1 Remote Transmission Request for the first extended identifier (set 1 if ExtId_1 is remote frame)
 */
void canApi_FilterSetOneStdIdOneExtIdListMode(canApi_FilterBank_Type FilterBank, uint16_t StdId_1,uint8_t RTR_StdId_1,uint32_t ExtId_1,uint8_t RTR_ExtId_1);


/*----------------------------------------------------------------------------*\
  GET FUNCTIONS (TX Signals)
\*----------------------------------------------------------------------------*/

/* CAN peripheral status signals */

/**
 * @brief Get BusOff status flag of CAN peripheral
 * @return 1 if bus in off state, else 0
 */
Int16 canApi_Get_BSW_IO_F_CAN_BSW_BusOff(void);

/**
 * @brief Get BusPassive status flag of CAN peripheral
 * @return 1 if bus in passive state, else 0
 */
Int16 canApi_Get_BSW_IO_F_CAN_BSW_Passive(void);

/**
 * @brief Get BusWarning status flag of CAN peripheral
 * @return 1 if bus in warning state, else 0
 */
Int16 canApi_Get_BSW_IO_F_CAN_BSW_Warning(void);

/* Info signals */
Float32 canApi_Get_INFO_ODO_Total_Kilometers(void);
Float32 canApi_Get_INFO_ODO_Trip_Kilometers(void);
Float32 canApi_Get_INFO_Motor_Current_Iq(void);
Float32 canApi_Get_INFO_Motor_Current_Id(void);
Float32 canApi_Get_INFO_DC_Current(void);
Float32 canApi_Get_INFO_Voltage_DC_Link(void);
Float32 canApi_Get_INFO_Rotor_Speed(void);
Float32 canApi_Get_INFO_Motor_Current(void);
Float32 canApi_Get_INFO_Vehicle_Speed(void);
Float32 canApi_Get_INFO_Remaining_Distance(void);
Float32 canApi_Get_INFO_Consumption_Ave_Trip(void);
Float32 canApi_Get_INFO_Ah_Pos(void);
Float32 canApi_Get_INFO_Ah_Neg(void);
Float32 canApi_Get_INFO_Rel_Torque_Setpoint(void);
Float32 canApi_Get_INFO_Rel_Torque_Max(void);
Float32 canApi_Get_INFO_Rel_Torque_Mapping(void);

/* Component temperature signals */
Float32 canApi_Get_TEMP_FET_Max(void);
Float32 canApi_Get_TEMP_Motor(void);
Float32 canApi_Get_TEMP_MCU(void);
Float32 canApi_Get_TEMP_Combined_Max_Rel(void);

/* Errorcodes */
UInt32 canApi_Get_ERR_Errorcode(void);
UInt32 canApi_Get_ERR_MEM_Trace_0_Errorcode(void);

/* State Manager signals */
Float32 canApi_Get_SM_OUT_SYS_Trq_Control(void);
Float32 canApi_Get_SM_PE_Mode_Req_Int(void);
Float32 canApi_Get_SM_BMS_Control_State(void);

/* Rotor Offset Calibration signals */
Float32 canApi_Get_ROC_Result(void);

/* Application signals */
Float32 canApi_Get_APP_Disp_Ride_Mode(void);
UInt32 canApi_Get_APP_Boost_Info(void);
Float32 canApi_Get_APP_Boost_Avail_Rel(void);
Float32 canApi_Get_APP_Boost_Avail_As(void);

/* Torque control signals */
Float32 canApi_Get_TRQ_LIM_Derating_Temp_MCU(void);
Float32 canApi_Get_TRQ_LIM_Derating_Max_Positive_Current(void);
Float32 canApi_Get_TRQ_LIM_Derating_Max_Negative_Current(void);
Float32 canApi_Get_TRQ_LIM_Derating_DC_Link_Voltage_Max(void);
Float32 canApi_Get_TRQ_LIM_Derating_DC_Link_Voltage_Min(void);
Float32 canApi_Get_TRQ_LIM_Derating_Rotor_Speed(void);
Float32 canApi_Get_TRQ_LIM_Derating_Temp_FET(void);
Float32 canApi_Get_TRQ_LIM_Derating_Temp_Motor(void);
Float32 canApi_Get_TRQ_LIM_Derating_Active(void);
Float32 canApi_Get_TRQ_DES_Driver_Reverse_Gear(void);

/* Hardware variant specific information */
UInt32 canApi_Get_PROD_M_BSW_Ver_Release(void);
UInt32 canApi_Get_PROD_M_BSW_Ver_Revision(void);
UInt32 canApi_Get_PROD_C_HW_Prod_Info_1(void);
UInt32 canApi_Get_PROD_M_HW_ID1(void);
UInt32 canApi_Get_PROD_M_HW_ID2(void);

/* Firmware specific information */
UInt32 canApi_Get_BSW_C_BSW_ET_Dataset_ID1(void);
UInt32 canApi_Get_BSW_C_BSW_ET_Dataset_ID2(void);
UInt32 canApi_Get_BSW_C_BSW_ET_Dataset_ID3(void);
UInt32 canApi_Get_BSW_Immo_Challenge_Lower(void);
UInt32 canApi_Get_BSW_Immo_Challenge_Higher(void);
UInt32 canApi_Get_BSW_BMS_Unlock_Code_Higher(void);
UInt32 canApi_Get_BSW_BMS_Unlock_Code_Lower(void);

/* Battery related information */
Float32 canApi_Get_SOC_State_of_Charge(void);

/*----------------------------------------------------------------------------*\
  SET FUNCTIONS (RX Signals)
\*----------------------------------------------------------------------------*/

/* External control signals */
void canApi_Set_CAN_EXT_Alive_Counter(UInt8);
void canApi_Set_CAN_EXT_Alive_Counter_Timeout(UInt8);
void canApi_Set_CAN_EXT_State_Request(Float32);
void canApi_Set_CAN_EXT_State_Request_Timeout(UInt8);
void canApi_Set_CAN_EXT_Ride_Mode(Float32);
void canApi_Set_CAN_EXT_Ride_Mode_Timeout(UInt8);
void canApi_Set_CAN_EXT_ROC_Start(Float32);
void canApi_Set_CAN_EXT_ROC_Start_Timeout(UInt8);
void canApi_Set_CAN_EXT_Boost_Enable(Float32);
void canApi_Set_CAN_EXT_Boost_Enable_Timeout(UInt8);
void canApi_Set_CAN_EXT_Reverse_Gear(Float32);
void canApi_Set_CAN_EXT_Reverse_Gear_Timeout(UInt8);
void canApi_Set_CAN_EXT_Torque_Request(Float32);
void canApi_Set_CAN_EXT_Torque_Request_Timeout(UInt8);
void canApi_Set_CAN_EXT_Rotor_Speed_Max(Float32);
void canApi_Set_CAN_EXT_Rotor_Speed_Max_Timeout(UInt8);
void canApi_Set_CAN_EXT_Skip_Signal_Checks(Float32);
void canApi_Set_CAN_EXT_Skip_Signal_Checks_Timeout(UInt8);

/* Immobilizer control signals */
void canApi_Set_CAN_Immo_Unlock_Request_Lower(UInt32);
void canApi_Set_CAN_Immo_Unlock_Request_Higher(UInt32);
void canApi_Set_CAN_Immo_Unlock_Request_Timeout(UInt8);

/* Battery Management System (BMS) control signals */
void canApi_Set_CAN_BMS_Pack_Voltage(Float32);
void canApi_Set_CAN_BMS_Pack_Voltage_Timeout(UInt8);
void canApi_Set_CAN_BMS_Pack_Current(Float32);
void canApi_Set_CAN_BMS_Pack_Current_Timeout(UInt8);
void canApi_Set_CAN_BMS_Errorcode(UInt32);
void canApi_Set_CAN_BMS_Errorcode_Timeout(UInt8);
void canApi_Set_CAN_BMS_Charge_Plug_Detection(Float32);
void canApi_Set_CAN_BMS_Charge_Plug_Detection_Timeout(UInt8);
void canApi_Set_CAN_BMS_State(Float32);
void canApi_Set_CAN_BMS_State_Timeout(UInt8);
void canApi_Set_CAN_BMS_SOC(Float32);
void canApi_Set_CAN_BMS_SOC_Timeout(UInt8);
void canApi_Set_CAN_BMS_State_of_Health(Float32);
void canApi_Set_CAN_BMS_State_of_Health_Timeout(UInt8);
void canApi_Set_CAN_BMS_Remaining_Capacity(Float32);
void canApi_Set_CAN_BMS_Remaining_Capacity_Timeout(UInt8);
void canApi_Set_CAN_BMS_Fullcharge_Capacity(Float32);
void canApi_Set_CAN_BMS_Fullcharge_Capacity_Timeout(UInt8);
void canApi_Set_CAN_BMS_TEMP_Powerstage1(Float32);
void canApi_Set_CAN_BMS_TEMP_Powerstage1_Timeout(UInt8);
void canApi_Set_CAN_BMS_TEMP_Powerstage2(Float32);
void canApi_Set_CAN_BMS_TEMP_Powerstage2_Timeout(UInt8);
void canApi_Set_CAN_BMS_TEMP_MCU(Float32);
void canApi_Set_CAN_BMS_TEMP_MCU_Timeout(UInt8);
void canApi_Set_CAN_BMS_TEMP_Cell1(Float32);
void canApi_Set_CAN_BMS_TEMP_Cell1_Timeout(UInt8);
void canApi_Set_CAN_BMS_TEMP_Cell2(Float32);
void canApi_Set_CAN_BMS_TEMP_Cell2_Timeout(UInt8);
void canApi_Set_CAN_BMS_Max_Charge(Float32);
void canApi_Set_CAN_BMS_Max_Charge_Timeout(UInt8);
void canApi_Set_CAN_BMS_Max_Discharge(Float32);
void canApi_Set_CAN_BMS_Max_Discharge_Timeout(UInt8);
void canApi_Set_CAN_BMS_Max_Voltage(Float32);
void canApi_Set_CAN_BMS_Max_Voltage_Timeout(UInt8);
void canApi_Set_CAN_BMS_Min_Voltage(Float32);
void canApi_Set_CAN_BMS_Min_Voltage_Timeout(UInt8);
void canApi_Set_CAN_BMS_Warning_Status(Float32);
void canApi_Set_CAN_BMS_Warning_Status_Timeout(UInt8);
void canApi_Set_CAN_BMS_Pending_HV_Shutdown(Float32);
void canApi_Set_CAN_BMS_Pending_HV_Shutdown_Timeout(UInt8);
void canApi_Set_CAN_BMS_Pending_Bordnet_Shutdown(Float32);
void canApi_Set_CAN_BMS_Pending_Bordnet_Shutdown_Timeout(UInt8);
void canApi_Set_CAN_BMS_PushButton_ShortPress_Detected(Float32);
void canApi_Set_CAN_BMS_PushButton_ShortPress_Detected_Timeout(UInt8);
void canApi_Set_CAN_BMS_PushButton_LongPress_Detected(Float32);
void canApi_Set_CAN_BMS_PushButton_LongPress_Detected_Timeout(UInt8);
void canApi_Set_CAN_BMS_PushButton_SuperLongPress_Detected(Float32);
void canApi_Set_CAN_BMS_PushButton_SuperLongPress_Detected_Timeout(UInt8);
void canApi_Set_CAN_BMS_PushButton_SuperLongPress_Ongoing(Float32);
void canApi_Set_CAN_BMS_PushButton_SuperLongPress_Ongoing_Timeout(UInt8);

/* Display signals */
void canApi_Set_CAN_Disp_Reset_Trip(Float32);
void canApi_Set_CAN_Disp_Reset_Trip_Timeout(UInt8);

/* Dynamometer control signals */
void canApi_Set_CAN_Dyno_Torque(Float32);
void canApi_Set_CAN_Dyno_Torque_Timeout(UInt8);
void canApi_Set_CAN_Dyno_DC_Current(Float32);
void canApi_Set_CAN_Dyno_DC_Current_Timeout(UInt8);
void canApi_Set_CAN_Dyno_DC_Voltage(Float32);
void canApi_Set_CAN_Dyno_DC_Voltage_Timeout(UInt8);
void canApi_Set_CAN_Dyno_Elec_Power_Input(Float32);
void canApi_Set_CAN_Dyno_Elec_Power_Input_Timeout(UInt8);

/* Timeout flag set functions for individual use */
void canApi_Set_CAN_Custom_Timeout_Bit27(UInt8);
void canApi_Set_CAN_Custom_Timeout_Bit28(UInt8);
void canApi_Set_CAN_Custom_Timeout_Bit29(UInt8);
void canApi_Set_CAN_Custom_Timeout_Bit30(UInt8);
void canApi_Set_CAN_Custom_Timeout_Bit31(UInt8);


/** @} */ 

#endif /* CANAPI_H_ */
