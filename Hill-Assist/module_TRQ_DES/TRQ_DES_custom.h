/**************************************************************************************************\
 *** 
 *** 				FRIWO SDK: Header file for custom module TRQ_DES
 *** 
\**************************************************************************************************/

#ifndef TRQ_DES_CUSTOM_H
#define TRQ_DES_CUSTOM_H

/*----------------------------------------------------------------------------*\
  DEFINES (OPT)
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
  INCLUDES
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
  DEFINES
\*----------------------------------------------------------------------------*/
#define SDK_Modul_Interfaces volatile
/*----------------------------------------------------------------------------*\
  TYPEDEFS
\*----------------------------------------------------------------------------*/

typedef unsigned char Bool; /* boolean basetype */
typedef float Float32; /* 32 bit floating-point basetype */
typedef double Float64; /* 64 bit floating-point basetype */
typedef signed short int Int16; /* 16 bit signed integer basetype */
typedef signed long int Int32; /* 32 bit signed integer basetype */
typedef signed char Int8; /* 8 bit signed integer basetype */
typedef unsigned short int UInt16; /* 16 bit unsigned integer basetype */
typedef unsigned long int UInt32; /* 32 bit unsigned integer basetype */
typedef unsigned char UInt8; /* 8 bit unsigned integer basetype */

/*----------------------------------------------------------------------------*\
  ENUMS
\*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*\
  VARIABLES
\*----------------------------------------------------------------------------*/

/******************************************************************************\
   General SDK module interface variables | Width: 32
\******************************************************************************/

/* Inputs */
extern SDK_Modul_Interfaces Float32 AIN1_Throttle /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 AIN2_Throttle /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 APP_Brake_Signal_Channel /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 APP_Reverse_Gear_Signal_Channel /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 APP_Throttle_Signal_Channel /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 CAN_EXT_Reverse_Gear /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 CAN_EXT_Torque_Request /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 DIN_DIN1_Signal /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 DIN_DIN2_Signal /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 PWMI_Throttle /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
extern SDK_Modul_Interfaces Float32 MO_Rotor_Speed /* 
   Description: Mechanical rotor-speed [1/s] */;
extern SDK_Modul_Interfaces Float32 SM_Trq_Control_status /* 
   Description: Status of torque-control;StateList;0=Torque-control deactivated;1=Torque-control act
   ive */;
extern SDK_Modul_Interfaces Float32 APP_Disp_Ride_Mode /* 
   Description: Selected ride-mode */;  
   
   
/* Outputs */
extern SDK_Modul_Interfaces Float32 TRQ_DES_Driver_Brake /* 
   Description: Brake signal used for desired driver torque calculation [%] */;
extern SDK_Modul_Interfaces Float32 TRQ_DES_Driver_Reverse_Gear /* 
   Description: Selected driving direction;StateList;0 = Forward gear selected;1 = Reverse gear sele
   cted */;
extern SDK_Modul_Interfaces Float32 TRQ_DES_Driver_Throttle /* 
   Description: Accelerator-pedal signal used for desired driver torque calculation [%] */;
extern SDK_Modul_Interfaces Float32 TRQ_DES_Trq_Req_Rel /* 
   Description: Desired driver torque, combined from all input-signal sources [%] */;
   
/*----------------------------------------------------------------------------*\
  PARAMETERIZED MACROS
\*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*\
  FUNCTION PROTOTYPES
\*----------------------------------------------------------------------------*/
void TRQ_DES_custom(void);

#endif/*TRQ_DES_CUSTOM_H */
/*----------------------------------------------------------------------------*\
  END OF FILE
\*----------------------------------------------------------------------------*/

