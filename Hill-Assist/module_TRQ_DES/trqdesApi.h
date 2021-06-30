/**
*******************************************************************************
* @file canApi.h
* @brief -
* @author Sven Hoechemer
* @date 16.06.2021 - 10:13:17
* <hr>
*******************************************************************************
* COPYRIGHT &copy; 2021 FRIWO GmbH
*******************************************************************************
*/

#ifndef TRQDESAPI_H_
#define TRQDESAPI_H_

/**
* @addtogroup trqdesApi
* @{
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* INCLUDES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC CONSTANTS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC FUNCTION PROTOTYPES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Function prototypes to GET variables from other modules */
Float32 trqdesApi_Get_APP_Brake_Signal_Channel(void);
Float32 trqdesApi_Get_APP_Reverse_Gear_Signal_Channel(void);
Float32 trqdesApi_Get_APP_Throttle_Signal_Channel(void);
Float32 trqdesApi_Get_AIN1_Throttle(void);
Float32 trqdesApi_Get_AIN2_Throttle(void);
Float32 trqdesApi_Get_CAN_EXT_Reverse_Gear(void);
Float32 trqdesApi_Get_CAN_EXT_Torque_Request(void);
Float32 trqdesApi_Get_DIN_DIN1_Signal(void);
Float32 trqdesApi_Get_DIN_DIN2_Signal(void);
Float32 trqdesApi_Get_PWMI_Throttle(void);
Float32 trqdesApi_Get_INFO_Rotor_Speed(void);
Float32 trqdesApi_Get_APP_Disp_Ride_Mode(void);
Float32 trqdesApi_Get_SM_OUT_SYS_Trq_Control(void);
Float32 trqdesApi_Get_IHS_Vibration_Detected(void);

/* Function prototypes to SET variables for other modules */
void trqdesApi_Set_TRQ_DES_Driver_Throttle(Float32);
void trqdesApi_Set_TRQ_DES_Driver_Brake(Float32);
void trqdesApi_Set_TRQ_DES_Driver_Reverse_Gear(Float32);
void trqdesApi_Set_TRQ_DES_Trq_Req_Rel(Float32);

/** @} */ 

#endif /* TRQDESAPI_H_ */
