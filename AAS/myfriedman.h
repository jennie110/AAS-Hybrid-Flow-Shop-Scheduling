//
// MATLAB Compiler: 7.0 (R2018b)
// Date: Thu Feb  9 11:15:46 2023
// Arguments:
// "-B""macro_default""-W""cpplib:myfriedman""-T""link:lib""myfriedman.m""-C"
//

#ifndef __myfriedman_h
#define __myfriedman_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_myfriedman_C_API 
#define LIB_myfriedman_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_myfriedman_C_API 
bool MW_CALL_CONV myfriedmanInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_myfriedman_C_API 
bool MW_CALL_CONV myfriedmanInitialize(void);

extern LIB_myfriedman_C_API 
void MW_CALL_CONV myfriedmanTerminate(void);

extern LIB_myfriedman_C_API 
void MW_CALL_CONV myfriedmanPrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_myfriedman_C_API 
bool MW_CALL_CONV mlxMyfriedman(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#ifdef __cplusplus
}
#endif


/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__MINGW64__)

#ifdef EXPORTING_myfriedman
#define PUBLIC_myfriedman_CPP_API __declspec(dllexport)
#else
#define PUBLIC_myfriedman_CPP_API __declspec(dllimport)
#endif

#define LIB_myfriedman_CPP_API PUBLIC_myfriedman_CPP_API

#else

#if !defined(LIB_myfriedman_CPP_API)
#if defined(LIB_myfriedman_C_API)
#define LIB_myfriedman_CPP_API LIB_myfriedman_C_API
#else
#define LIB_myfriedman_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_myfriedman_CPP_API void MW_CALL_CONV myfriedman(int nargout, mwArray& c, const mwArray& x);

/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */
#endif

#endif
