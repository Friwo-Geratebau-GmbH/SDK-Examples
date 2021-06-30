/**
*******************************************************************************
* @file TRQ_DES_custom.c
* @brief FRIWO SDK Source file for custom module TRQDES
* @author Sven Hoechemer
* @date 14.06.2021 - 10:08:33
* <hr>
*******************************************************************************
* COPYRIGHT &copy; 2021 FRIWO GmbH
*******************************************************************************
*/

#ifndef TRQ_DES_CUSTOM_C_
#define TRQ_DES_CUSTOM_C_

/**
* @addtogroup TRQDES_custom
* @{
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* INCLUDES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TRQ_DES_custom.h"
#include "trqdesApi.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE DEFINES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE TYPEDEF */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** 
 * @brief Define different states for hill-assist state machine as enumeration.
 */
typedef enum 
{ 
	STATE_INITIAL, /**< @brief Initial state after start-up; decides whether to accelerate in assisted or normal mode depending on throttle/brake input and rotor speed. */
	STATE_THROTTLE_PRIO, /**< @brief Priorizes throttle over break input for a certain time (counter) in order to generate enough torque for acceleration on hill. */
	STATE_NORMAL_ACCELERATION /**< @brief Add up both throttle and brake inputs to calculate desired torque. State is left, if both inputs equal zero and rotor speed is below threshold. */
}hillAssistState_TypeDef;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE CONSTANTS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**	
 * Define variables to be displayed in FRIWO EnableTool Application.
 * Section EMERGE_DISP_RAM: Application data which will be read from Flash.
 * All data types which can be chosen:  Int8, Int16, Int32, UInt8, UInt16, UInt32, Bool and Float32.
 */
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces Float32 TRQ_DES_Throttle_Input; /*
	Description: Throttle signal value after selection of input channel [%] */
	
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces Float32 TRQ_DES_Brake_Input; /*
	Description: Brake signal value after selection of input channel [%] */
   
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces Float32 TRQ_DES_ReverseGear_Input; /*
	Description: Shows if reverse gear is selected after selection of input channel;  0 = forward gear selected; 1 = reverse gear selected */
	
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces Float32 TRQ_DES_TorqueRequest; /*
	Description: Shows the desired torque request returned to trqdesApi [%]; */
   
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces UInt8 TRQ_DES_TorqueRequest_UpperLim; /*
	Description: Shows if desired torque has reached the upper bound of allowed operational range */
   
__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces UInt8 TRQ_DES_TorqueRequest_LowerLim; /*
	Description: Shows if desired torque has reached the lower bound of allowed operational range */

__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces UInt8 TRQ_DES_HillAssist_State; /*
	Description: Shows the actual state of hill-assist algorithm; 0 = Initial state; 1 = Throttle priorization; 
	2 = Accelerate without priorization; */

__attribute__((section("EMERGE_DISP_RAM")))
MEDKit_Modul_Interfaces UInt16 TRQ_DES_HillAssist_ValCounter; /*
	Description: Shows the actual value of hill-assist counter to priorize throttle */


/** 
 * Define variables to be calibrated with FRIWO EnableTool Application.
 * Section EMERGE_NV_RAM_PAGE1:  Standart application data which will be stored in flash when writing a snapshot.
 * All data types which can be chosen:  Int8, Int16, Int32, UInt8, UInt16, UInt32, Bool and Float32.
 */
 __attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces UInt8 TRQ_DES_C_ReverseGear_TestInput = 0u; /* 
	Description: Test parameter for manual input of reverse gear signal [-]; Limits: 0...1 */
	
__attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces UInt16 TRQ_DES_C_ThrottlePriorization_Time = 10000u; /* 
	Description: Parameter for time during which throttle will be priorized when both brake and throttle
	pedal are used in parallel [ms]; Limits: 0...65535 */
   
__attribute__((section("EMERGE_NV_RAM_PAGE1")))
MEDKit_Modul_Interfaces Float32 TRQ_DES_C_ThrottlePriorization_MaxRotorSpeed = 2.F; /* 
	Description: Parameter for maximum rotor speed to priorize throttle when both brake and throttle
	pedal are used in parallel [1/s]; Limits: -1...2000 */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE FUNCTION PROTOTYPES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Saturation function to saturate intermediate results and return parameters */
Float32 sigSaturation(Float32, Float32, Float32);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE VARIABLES */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PRIVATE FUNCTIONS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** 
 * @brief Saturate input signal by the transfered lower and upper limits.
 * @param sigLowerLimit: Lower limit of input signal used for saturation.
 * @param sigUpperLimit: Upper limit of input signal used for saturation.
 * @param sigInput: Input signal to be saturated.
 * @return saturated value of input.
 */
Float32 sigSaturation(Float32 sigLowerLimit, Float32 sigUpperLimit, Float32 sigInput){
	Float32 sigOutput = 0.F;
	
	if (sigInput > sigUpperLimit) {
		sigOutput = sigUpperLimit;
	}
	else {
		if (sigInput < sigLowerLimit) {
			sigOutput = sigLowerLimit;
		}
		else {
			sigOutput = sigInput;
		}
	}
	return sigOutput;
}	

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* PUBLIC FUNCTIONS */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** 
 * @brief Public function for desired torque calculation which can be customized.
 *
 * This function is the first module of torque calculation and strategy, which is called in basic firmware.
 * In this example, the function realizes a hill-assist functionality which can be used in eScooter applications.
 * Necessary input values such as analog input signals from throttle and brake pedal are received from trqdesApi through 
 * Get functions. Additionaly, there are four output signals which have to be set since they are cross connected to
 * other modules in basic firmware.
 * @see trqdesApi.h file for more details about available Get and Set functions.
 */
void TRQ_DES_custom(void){
	/* Define private variables to store return values of get-functions from trqdesApi. */
	Float32 AnalogInput1_Signal;
	Float32 AnalogInput2_Signal;
	Float32 Motor_RotorSpeed;
	Float32 TorqueControl_Status;

	/* Define state and counter variables which show the current state of hill-assist state machine. */
	static hillAssistState_TypeDef stateHillAssist = STATE_INITIAL;
	static UInt16 ctrHillAssist = 0u;
	
	/* Get desired signals from trqdesApi and store them in locally defined variables. */
	AnalogInput1_Signal = trqdesApi_Get_AIN1_Throttle();
	AnalogInput2_Signal = trqdesApi_Get_AIN2_Throttle();
	Motor_RotorSpeed = trqdesApi_Get_INFO_Rotor_Speed();
	TorqueControl_Status = trqdesApi_Get_SM_OUT_SYS_Trq_Control();
	
	/* Saturate the input signals to the desired range */
	TRQ_DES_Throttle_Input = sigSaturation(0.F, 100.F, AnalogInput1_Signal);
   	TRQ_DES_Brake_Input = sigSaturation(0.F, 100.F, AnalogInput2_Signal);
	TRQ_DES_ReverseGear_Input = sigSaturation(0.F, 1.F, (Float32)TRQ_DES_C_ReverseGear_TestInput);
	TorqueControl_Status = sigSaturation(0.F, 1.F, TorqueControl_Status);
	
	/* Make sure that system is ready and torque control is active by checking the flag Status_TorqueControl */
	if(TorqueControl_Status == 1.F) {
		/* Implement the state-machine for the hill-assist using the three states STATE_INITIAL, STATE_THROTTLE_PRIO and STATE_NORMAL_ACCELERATION */
		switch(stateHillAssist) {
			case STATE_INITIAL: {
				/* When accelerating while brake is pulled and rotor speed is below threshold, reload hill-assist counter and jump to STATE_THROTTLE_PRIO */
				if (TRQ_DES_Throttle_Input > 0.F && TRQ_DES_Brake_Input > 0.F && abs(Motor_RotorSpeed) <= TRQ_DES_C_ThrottlePriorization_MaxRotorSpeed) {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input;
					stateHillAssist = STATE_THROTTLE_PRIO;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
					ctrHillAssist = TRQ_DES_C_ThrottlePriorization_Time;
				}
				/* When accelerating without holding brake there is no priorization of throttle input and jump to STATE_NORMAL_ACCELERATION */
				else if (TRQ_DES_Throttle_Input > 0.F && TRQ_DES_Brake_Input <= 0.F) {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
					stateHillAssist = STATE_NORMAL_ACCELERATION;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
				}
				/* When there is no acceleration stay in STATE_INITIAL */
				else {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
				}
				break;
			}
			case STATE_THROTTLE_PRIO: {
				/* If still accelerating when counter reaches zero jump to STATE_NORMAL_ACCELERATION  */
				ctrHillAssist--;
				if (TRQ_DES_Throttle_Input > 0.F && ctrHillAssist == 0u) {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
					stateHillAssist = STATE_NORMAL_ACCELERATION;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
				}
				/* If throttle is released while counter still running reset counter and jump back to STATE_INITIAL */
				else if (TRQ_DES_Throttle_Input <= 0.F && ctrHillAssist != 0u) {
					ctrHillAssist = 0u;
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
					stateHillAssist = STATE_INITIAL;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
				}
				/* As long as counter has not reached zero priorize throttle over brake input */
				else {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input;
				}
				break;
			}
			case STATE_NORMAL_ACCELERATION: {
				/* If both pedals throttle and brake are released and rotor speed is below threshold, go back to STATE_INITIAL to reset hill-assist state machine */
				if (TRQ_DES_Throttle_Input <= 0.F && TRQ_DES_Brake_Input <= 0.F && abs(Motor_RotorSpeed) <= TRQ_DES_C_ThrottlePriorization_MaxRotorSpeed) {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
					stateHillAssist = STATE_INITIAL;
					TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;
				}
				/* Normal acceleration without priorization after hill-assist counter has reached zero */
				else {
					TRQ_DES_TorqueRequest = TRQ_DES_Throttle_Input - TRQ_DES_Brake_Input;
				}
				break;
			}
		}
	}
	else {
		stateHillAssist = STATE_INITIAL;
		ctrHillAssist = 1u;
		TRQ_DES_TorqueRequest = 0.F;
	}
	
	/* Show current state and counter value of hill-assist state machine */
	TRQ_DES_HillAssist_ValCounter = ctrHillAssist;
	TRQ_DES_HillAssist_State = (UInt8)stateHillAssist;

	/* Saturation of relative torque request from -100...100% with limit indication */
	if (TRQ_DES_TorqueRequest > 100.F) {
		TRQ_DES_TorqueRequest_UpperLim = 1u;
		TRQ_DES_TorqueRequest = 100.F;
	}
	else {
		if (TRQ_DES_TorqueRequest < -100.F) {
		TRQ_DES_TorqueRequest_LowerLim = 1u;
		TRQ_DES_TorqueRequest = -100.F;
		}
	}
	
	/**
	 * Return calculated set values as module outports to trqdesApi. 
	 * Besides desired torque TRQ_DES_TorqueRequest the following cross connections for state management and system startup must be considered and set as well:
	 * TRQ_DES_Driver_Throttle, TRQ_DES_Driver_Brake and TRQ_DES_Reverse_Gear
	*/
	trqdesApi_Set_TRQ_DES_Driver_Throttle(TRQ_DES_Throttle_Input);
	trqdesApi_Set_TRQ_DES_Driver_Brake(TRQ_DES_Brake_Input);
	trqdesApi_Set_TRQ_DES_Driver_Reverse_Gear(TRQ_DES_ReverseGear_Input);
	trqdesApi_Set_TRQ_DES_Trq_Req_Rel(TRQ_DES_TorqueRequest);
	
}

/** @} */

#endif /* TRQ_DES_CUSTOM_C_ */
