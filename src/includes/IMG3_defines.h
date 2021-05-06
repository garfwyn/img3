#ifndef __IMG3_DEFINES_H
#define __IMG3_DEFINES_H

#include <string.h>
#include <stdio.h>
#include <errno.h>

#define IMG3_ERROR_NONE					0x0000000
#define IMG3_ERROR_INVALID_PARAMETER	0x0001000

//#define IMG3_DEBUG

#ifdef IMG3_DEBUG
#undef IMG3_DEBUG
#endif

#define PRINT_CLASS_ERROR( X )		fprintf( stderr, "%s:%s:%u\r\nClass error 0x%08x: %s.\r\n", __FILE__, __FUNCTION__, __LINE__, errorCode, X )
#define PRINT_SYSTEM_ERROR()		fprintf( stderr,"%s:%s:%u\r\nSystem error %#08x: %s.\n", __FILE__, __FUNCTION__, __LINE__, errno, strerror( errno ) )
#define PRINT_PROGRAM_ERROR( X )	fprintf( stderr, "%s:%s:%u:%s.\n", __FILE__, __FUNCTION__, __LINE__, X )

#define ASSERT(X);					{ \
										if ( !X ) { \
											fprintf( stderr, "Assertion: file %s, function %s, line %d.\n", __FILE__, __FUNCTION__, __LINE__ ); \
											return; \
										} \
									}

#define ASSERT_RET(X,Y);			{ \
										if ( !X ) { \
											fprintf( stderr, "Assertion: file %s, function %s, line %d.\n", __FILE__, __FUNCTION__, __LINE__ ); \
											return Y; \
										} \
									}
#define CLASS_ASSERT_RET( X, Y, Z )	{ \
										if ( !X ) { \
											errorCode = Y; \
											fprintf( stderr, "Assertion: file %s, function %s, line %d.\n", __FILE__, __FUNCTION__, __LINE__ ); \
											return Z; \
										} \
									}

#define CLASS_VALIDATE_PARAMETER( X, Y )	{ \
												if ( !X ) { \
													errorCode = IMG3_ERROR_INVALID_PARAMETER; \
													fprintf( stderr, "Invalid parameter: file %s, function %s, line %d.\n", __FILE__, __FUNCTION__, __LINE__ ); \
													return Y; \
												} \
											}

#endif //__IMG3_DEFINES_H
