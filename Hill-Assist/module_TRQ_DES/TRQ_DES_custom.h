/**
*******************************************************************************
* @file TRQ_DES_custom.h
* @brief FRIWO SDK Header file for custom module TRQDES
* @author Sven Hoechemer
* @date 15.06.2021 - 9:49:34
* <hr>
*******************************************************************************
* COPYRIGHT &copy; 2021 FRIWO GmbH
*******************************************************************************
*/

#ifndef TRQ_DES_CUSTOM_H_
#define TRQ_DES_CUSTOM_H_

/**
* @addtogroup TRQ_DES_custom
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

/** 
 * @brief Public function for desired torque calculation which can be customized.
 */
void TRQ_DES_custom(void);


/** @} */ 

#endif /* TRQ_DES_CUSTOM_H_ */
