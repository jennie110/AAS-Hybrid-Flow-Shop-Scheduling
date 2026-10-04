//
// MATLAB Compiler: 7.0 (R2018b)
// Date: Wed May  5 17:15:24 2021
// Arguments:
// "-B""macro_default""-W""cpplib:ranksum""-T""link:lib""myranksum.m""-C"
//

#ifndef __ranksum_h
#define __ranksum_h 1

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
#ifndef LIB_ranksum_C_API 
#define LIB_ranksum_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_ranksum_C_API 
bool MW_CALL_CONV ranksumInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_ranksum_C_API 
bool MW_CALL_CONV ranksumInitialize(void);

extern LIB_ranksum_C_API 
void MW_CALL_CONV ranksumTerminate(void);

extern LIB_ranksum_C_API 
void MW_CALL_CONV ranksumPrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_ranksum_C_API 
bool MW_CALL_CONV mlxMyranksum(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#ifdef __cplusplus
}
#endif


/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__MINGW64__)

#ifdef EXPORTING_ranksum
#define PUBLIC_ranksum_CPP_API __declspec(dllexport)
#else
#define PUBLIC_ranksum_CPP_API __declspec(dllimport)
#endif

#define LIB_ranksum_CPP_API PUBLIC_ranksum_CPP_API

#else

#if !defined(LIB_ranksum_CPP_API)
#if defined(LIB_ranksum_C_API)
#define LIB_ranksum_CPP_API LIB_ranksum_C_API
#else
#define LIB_ranksum_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_ranksum_CPP_API void MW_CALL_CONV myranksum(int nargout, mwArray& p, mwArray& h, const mwArray& x, const mwArray& y);

/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */
#endif

#endif
