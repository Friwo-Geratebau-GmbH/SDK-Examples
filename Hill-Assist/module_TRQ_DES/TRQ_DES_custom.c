/**************************************************************************************************\
 *** 
 *** 				FRIWO SDK: Source file for custom module TRQ_DES
 *** 
\**************************************************************************************************/

/*----------------------------------------------------------------------------*\
  Module inports:
  	APP_Brake_Signal_Channel		| Float32 | Limits: 0...7	 		| Description: In APP-module selected input signal which will be used as the brake input; StateList: 0=not selected / off;1=AIN1;2=AIN2;3=PWM@DIN2;4=CAN-Bus;5=USB;6=DIN1;7=DIN2
	APP_Reverse_Gear_Signal_Channel	| Float32 | Limits: 0...4	 	   	| Description: In APP-module selected input signal which will be used as the reverse gear select input; StateList: 0=not selected / off;1 = DIN1;2 = DIN2;3 = CAN-bus;4 = USB
	APP_Throttle_Signal_Channel		| Float32 | Limits: 0...5	 		| Description: In APP-module selected input signal which will be used as the throttle input; StateList: 0=not selected / off;1 = AIN1; 2 = AIN2;3 = PWM@DIN2;4 = CAN-Bus;5 = USB
	AIN1_Throttle					| Float32 | Limits: 0...100 		| Description: Output signal [%]; 0 = no throttle; 100 = throttle fully engaged; -100 = max. braking torque (or reverse)
	AIN2_Throttle					| Float32 | Limits: 0...100			| Description: Output signal [%]; 0 = no throttle; 100 = throttle fully engaged
	CAN_EXT_Reverse_Gear			| Float32 | Limits: 0...100    		| Description: Reverse gear selected via CAN
	CAN_EXT_Torque_Request			| Float32 | Limits: 0...100   		| Description: Relative torque request via CAN [%];
	DIN_DIN1_Signal					| Float32 | Limits: 0...1	 	   	| Description: Digital signal value of DIN1 [%]
	DIN_DIN2_Signal					| Float32 | Limits: 0...1	 	   	| Description: Digital signal value of DIN2 [%]
	PWMI_Throttle 					| Float32 | Limits: 0...100 	   	| Description: Output signal [%]; 0 = no throttle; 100 = throttle fully engaged
	MO_Rotor_Speed					| Float32 | Limits: -2400...2400	| Description: Mechanical motor rotor-speed [1/s]
	APP_Disp_Ride_Mode				| Float32 | Limits: 0...3			| Description: Selected ride-mode [-]
	SM_Trq_Control_status			| Float32 | Limits: 0...1			| Description: Status of torque-control; StateList: 0=Torque-control deactivated; 1=Torque-control active

\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
  Module outports:
	TRQ_DES_Driver_Throttle			| Float32 | Limits: 0...100 		| Description: Throttle signal used for desired driver torque calculation [%]
	TRQ_DES_Driver_Brake			| Float32 | Limits: 0...100 		| Description: Brake signal used for desired driver torque calculation [%]
	TRQ_DES_Driver_Reverse_Gear		| Float32 | Limits: 0...1 			| Description: Selected driving direction;StateList;0 = Forward gear selected;1 = Reverse gear selected [-]
	TRQ_DES_Trq_Req_Rel				| Float32 | Limits: -100...100		| Description: Desired driver torque, combined from all input-signal sources [%]	
\*----------------------------------------------------------------------------*/

#ifndef TRQ_DES_CUSTOM_C
#define TRQ_DES_CUSTOM_C

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TRQ_DES_custom.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Define variables to be displayed in FRIWO EnableTool 					 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Section EMERGE_DISP_RAM:  Application data which will be read from Flash */
/* All data types which can be chosen:  Int8, Int16, Int32, UInt8, UInt16, UInt32, Bool and Float32 */

__attribute__((section("EMERGE_DISP_RAM")))
volatile Float32 TRQ_DES_Throttle_Input
/* Description: Throttle input value after selection of input channel [%]; */;
	
__attribute__((section("EMERGE_DISP_RAM")))
volatile Float32 TRQ_DES_Brake_Input
/* Description: Brake input value after selection of input channel [%]; */;
   
__attribute__((section("EMERGE_DISP_RAM")))
volatile Float32 TRQ_DES_Reverse_Gear
/* Description: Shows if reverse gear is selected;  0 = forward gear selected; 1 = reverse gear selected */;
   
__attribute__((section("EMERGE_DISP_RAM")))
volatile Float32 TRQ_DES_Flag_Upper_Lim
/* Description: Shows if desired torque has reached the upper bound of allowed operational range */;
   
__attribute__((section("EMERGE_DISP_RAM")))
volatile Float32 TRQ_DES_Flag_Lower_Lim
/* Description: Shows if desired torque has reached the lower bound of allowed operational range; */;

__attribute__((section("EMERGE_DISP_RAM")))
volatile UInt32 TRQ_DES_Hill_Assist_Disp_State
/* Description: Shows the actual state of hill-assist algorithm; 0 = Hill-Assist not active; 1 = Entry state; 
2 = Prepare for Throttle-Priorization; 3 = Accelerate with Brake on; 4 = Accelerate with Brake released */;

__attribute__((section("EMERGE_DISP_RAM")))
volatile UInt32 TRQ_DES_Hill_Assist_Disp_Counter
/* Description: Shows the actual value of hill-assist counter to priorize throttle; */;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Define variables to be calibrated with FRIWO EnableTool 					 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Section EMERGE_NV_RAM_PAGE1:  Standart application data which will be stored in Flash when writing a snapshot*/
/* All data types which can be chosen:  Int8, Int16, Int32, UInt8, UInt16, UInt32, Bool and Float32  */

__attribute__((section("EMERGE_NV_RAM_PAGE1")))
volatile Float32 TRQ_DES_C_Test_Torque_Request_tmpl = 0.F /* 
	Description: Template parameter for test torque request via custom variable [%]; Limits: -100...100 */;
   
__attribute__((section("EMERGE_NV_RAM_PAGE1")))
volatile UInt32 TRQ_DES_C_Test_Reverse_Gear_tmpl = 0 /* 
	Description: Template parameter to set reverse gear via custom variable [%]; Limits: 0...1 */;
	
__attribute__((section("EMERGE_NV_RAM_PAGE1")))
volatile UInt32 TRQ_DES_C_ThrottleBrakeComb_Cut_Time_tmpl = 10000 /* 
	Description: Template parameter for time in milliseconds after which the accelerator-pedal signal will be 
	cut-off on parallel use of throttle and brake [ms]; Limits: 0...4294967295 */;
   
__attribute__((section("EMERGE_NV_RAM_PAGE1")))
volatile Float32 TRQ_DES_C_ThrottleBrakeComb_Max_Speed_tmpl = 2.F /* 
	Description: Template parameter to define a rotor speed threshold at which the accelerator-pedal signal 
	will always be cut-off by using the brake in parallel [1/s]; Limits: -1...2000 */;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Define local functions													 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Float32 Saturation(Float32 lower_lim, Float32 upper_lim, Float32 sig_in){
	Float32 sig_out = 0.F;
	
	if (sig_in > upper_lim) {
		sig_out = upper_lim;
	}
	else {
		if (sig_in < lower_lim) {
			sig_out = lower_lim;
		}
		else {
			sig_out = sig_in;
		}
	}
	return sig_out;
}	

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Define TRQ_DES_custom module function									 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void TRQ_DES_custom(void){

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Define local variables 													 */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static int SR_ff_Q = 0;
static int SR_ff_Qn = 1;
static int SR_ff_S = 0;
static int SR_ff_R = 0;

struct state_machine {
	int state[10];
};

static struct state_machine TRQ_DES_Hill_Assist;
static UInt32 TRQ_DES_Hill_Assist_ctr = 0;

/*----------------------------------------------------------------------------*\
  User-defined functionalities
\*----------------------------------------------------------------------------*/

	/* Select input channel for throttle signal */
   switch ((Int32) APP_Throttle_Signal_Channel) {
      case 0: {
         TRQ_DES_Throttle_Input = 0.F;
         break;
      }
      case 1: {
         TRQ_DES_Throttle_Input = AIN1_Throttle;
         break;
      }
      case 2: {
         TRQ_DES_Throttle_Input = AIN2_Throttle;
         break;
      }
      case 3: {
         TRQ_DES_Throttle_Input = PWMI_Throttle;
         break;
      }
      case 4: {		  
         /* Saturation */
  	  	TRQ_DES_Throttle_Input = Saturation(0.F, 100.F, CAN_EXT_Torque_Request);
         break;
      }
      default: {         
         /* Saturation */
	  	TRQ_DES_Throttle_Input = Saturation(0.F, 100.F, TRQ_DES_C_Test_Torque_Request_tmpl);
         break;
      }
   }
   	
   /* Saturation */
	TRQ_DES_Throttle_Input = Saturation(0.F, 100.F, TRQ_DES_Throttle_Input);
   
   /* Select input channel for brake signal */
   switch ((Int32) APP_Brake_Signal_Channel) {
      case 0: {
         TRQ_DES_Brake_Input = 0.F;
         break;
      }
      case 1: {
         TRQ_DES_Brake_Input = AIN1_Throttle;
         break;
      }
      case 2: {
         TRQ_DES_Brake_Input = AIN2_Throttle;
         break;
      }
      case 3: {
         TRQ_DES_Brake_Input = PWMI_Throttle;
         break;
      }
      case 4: {		  
         /* Saturation */
		TRQ_DES_Brake_Input = Saturation(-100.F, 0.F, CAN_EXT_Torque_Request);
		TRQ_DES_Brake_Input = -TRQ_DES_Brake_Input;
        break;
      }
      case 5: {         
         /* Saturation */
     	TRQ_DES_Brake_Input = Saturation(-100.F, 0.F, TRQ_DES_C_Test_Torque_Request_tmpl);
		TRQ_DES_Brake_Input = -TRQ_DES_Brake_Input;
		  
         break;
      }
	  case 6: {
		  TRQ_DES_Brake_Input = 100.F * DIN_DIN1_Signal;
	  }
	  default: {
		  TRQ_DES_Brake_Input = 100.F * DIN_DIN2_Signal;
	  }
   }
   
   /* Saturation */
	TRQ_DES_Brake_Input = Saturation(0.F, 100.F, TRQ_DES_Brake_Input);   
	
	/* Select input channel for reverse gear */
   switch ((Int32) APP_Reverse_Gear_Signal_Channel) {
      case 0: {
         TRQ_DES_Reverse_Gear = 0.F;
         break;
      }
      case 1: {
		  TRQ_DES_Reverse_Gear = 100.F * DIN_DIN1_Signal;
         break;
      }
      case 2: {
		  TRQ_DES_Reverse_Gear = 100.F * DIN_DIN2_Signal;
         break;
      }
      case 3: {
         TRQ_DES_Reverse_Gear = CAN_EXT_Reverse_Gear;
         break;
      }
      default: {		  
         TRQ_DES_Reverse_Gear = TRQ_DES_C_Test_Reverse_Gear_tmpl;
      }
	}
	
	/* Saturation */
	TRQ_DES_Reverse_Gear = Saturation(0.F, 1.F, TRQ_DES_Reverse_Gear);
	
	/* Cross connections of "raw" input values for state management and system startup */
	TRQ_DES_Driver_Throttle = TRQ_DES_Throttle_Input;
	TRQ_DES_Driver_Brake = TRQ_DES_Brake_Input;
	TRQ_DES_Driver_Reverse_Gear = TRQ_DES_Reverse_Gear;
   
   /* Invert brake signal for desired torque calculation */
	TRQ_DES_Brake_Input = -TRQ_DES_Brake_Input;
	
	/* Check if motor speed is below speed threshold and throttle input is zero to activate hill assist */
	if(abs(MO_Rotor_Speed) > TRQ_DES_C_ThrottleBrakeComb_Max_Speed_tmpl){
		SR_ff_S = 1;
	}
	else{
		SR_ff_S = 0;
	}
	
	if(TRQ_DES_Throttle_Input <= 0 && !SR_ff_S){
		SR_ff_R = 1;
	}
	else{
		SR_ff_R = 0;
	}
	
	/* S-R flip flop to activate hill assist */
	if (SR_ff_S && (!(SR_ff_R))) {
      SR_ff_Qn = 0;
      SR_ff_Q = 1;
	}
	else {
      if ((!(SR_ff_S)) && SR_ff_R) {
         SR_ff_Qn = 1;
         SR_ff_Q = 0;
      }
      else {
         if (SR_ff_S && SR_ff_R) {
            SR_ff_Qn = 1;
            SR_ff_Q = 0;
         }
         else {
            SR_ff_Q = !(SR_ff_Qn);
         }
      }
	}

	if(SR_ff_Qn){
			/* Hill Assist */
			if (TRQ_DES_Hill_Assist.state[0]) {
				TRQ_DES_Hill_Assist_Disp_State = 1;
			  if (TRQ_DES_Throttle_Input <= 0.F) {
				 TRQ_DES_Hill_Assist.state[0] = 0;
				 TRQ_DES_Hill_Assist.state[1] = 1;
				 TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
			  }
			  else {
				 TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
			  }
			}
			else {
			  if (TRQ_DES_Hill_Assist.state[1]) {
				 /* Prepare for Throttle-Priorization */
				 TRQ_DES_Hill_Assist_Disp_State = 2;
				 if (TRQ_DES_Throttle_Input > 0.F) {
					TRQ_DES_Hill_Assist.state[1] = 0;
					TRQ_DES_Hill_Assist_ctr = 0;
					TRQ_DES_Hill_Assist.state[2] = 1;
					TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input;
				 }
				 else {
					TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
				 }
			  }
			  else {
				 if (TRQ_DES_Hill_Assist.state[2]) {
					/* Accelerate with Brake on */
					TRQ_DES_Hill_Assist_Disp_State = 3;
					TRQ_DES_Hill_Assist_ctr++;
					if (TRQ_DES_Brake_Input == 0.F) {
					   TRQ_DES_Hill_Assist.state[2] = 0;
  					   TRQ_DES_Hill_Assist_ctr = 0;
					   TRQ_DES_Hill_Assist.state[3] = 1;
					   TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
					}
					else {
					   if (TRQ_DES_Hill_Assist_ctr >= TRQ_DES_C_ThrottleBrakeComb_Cut_Time_tmpl) {
							TRQ_DES_Hill_Assist.state[2] = 0;
							TRQ_DES_Hill_Assist_ctr = 0;
							TRQ_DES_Hill_Assist.state[3] = 1;
							TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
					   }
					   else {
						  TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input;
					   }
					}
					TRQ_DES_Hill_Assist_Disp_Counter = TRQ_DES_Hill_Assist_ctr;
				 }
				 else {
					if (TRQ_DES_Hill_Assist.state[3]) {
						/* Accelerate with Brake released */
						TRQ_DES_Hill_Assist_Disp_State = 4;
					   if ((TRQ_DES_Throttle_Input <= 0.F) && (TRQ_DES_Brake_Input <= 0.F)) {
							TRQ_DES_Hill_Assist.state[3] = 0;
							TRQ_DES_Hill_Assist.state[0] = 1;
							TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
					   }
					   else {
							TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
					   }
					}
					else {
						/* Entry state */
						TRQ_DES_Hill_Assist_Disp_State = 1;
						TRQ_DES_Hill_Assist.state[0] = 1;
						TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
					}
				 }
			  }
			}
	}
	else{
		TRQ_DES_Hill_Assist_ctr = 0;
		TRQ_DES_Hill_Assist_Disp_State = 0;
		TRQ_DES_Trq_Req_Rel = TRQ_DES_Throttle_Input + TRQ_DES_Brake_Input;
	}
	
	/* Saturation */
	if (TRQ_DES_Trq_Req_Rel > 100.F) {
		TRQ_DES_Flag_Upper_Lim = 1;
		TRQ_DES_Trq_Req_Rel = 100.F;
	}
	else {
		if (TRQ_DES_Trq_Req_Rel < -100.F) {
		TRQ_DES_Flag_Lower_Lim = 1;
		TRQ_DES_Trq_Req_Rel = -100.F;
		}
	}	
}
#endif/*TRQ_DES_CUSTOM_C */
/*----------------------------------------------------------------------------*\
  END OF FILE
\*----------------------------------------------------------------------------*/