/*! \file IMG3_LzssInterface.h
 	\author Matthew Areno
	\data March 21, 2011
	\version 1.0
 
 	This is an C++ class for interfacing with the LZSS compression methods developed by Haruhiko Okumura.
 */

#ifndef IMG3_LZSSINTERFACE_H_
#define IMG3_LZSSINTERFACE_H_

#include <stdio.h>
#include <stdint.h>

#define IMG3_LZSSINTERFACE_BASE 65521L
#define IMG3_LZSSINTERFACE_NMAX 5521

#define IMG3_LZSSINTERFACE_N           4096
#define IMG3_LZSSINTERFACE_F           18
#define IMG3_LZSSINTERFACE_THRESHOLD   2
#define IMG3_LZSSINTERFACE_NIL         IMG3_LZSSINTERFACE_N

#define IMG3_LZSSINTERFACE_COMP_SIGNATURE 0x636F6D70
#define IMG3_LZSSINTERFACE_LZSS_SIGNATURE 0x6C7A7373

#define IMG3_LZSS_ERROR_NONE					0x0000
#define IMG3_LZSS_ERROR_FILE_IS_NOT_COMPRESSED	0x0001
#define	IMG3_LZSS_ERROR_FILE_NOT_A_LZSS_FILE	0x0002
#define IMG3_LZSS_ERROR_COMPRESSION_FAILED		0x0003
#define IMG3_LZSS_ERROR_CHECKSUM_MISMATCH		0x0004
#define IMG3_LZSS_ERROR_INPUT_TOO_SMALL			0x0005

//! IMG3_LzssInterface_CompressionHeader
/*! A structure representing the LZSS compression header. */

typedef struct IMG3_LzssInterface_CompressionHeader {
    uint32_t signature;				/*!< The LZSS signature; should always be 0x6C7A7373 */
    uint32_t compression_type;		/*!< The compression type marker; should always be 0x636F6D70 */
    uint32_t checksum;				/*!< The Alder32 checksum for the LZSS archive */
    uint32_t length_uncompressed;	/*!< The length of the file once it's decompressed */
    uint32_t length_compressed;		/*!< The length of the file compressed */
    uint8_t  padding[ 0x16c ];		/*!< 0x16C bytes of zero padding before data */
} __attribute__((__packed__)) IMG3_LzssInterface_CompressionHeader;

//! encode_state
/*! A structure representing the encode state used for the LZSS encoding state machine. */

struct encode_state {
	int lchild[ IMG3_LZSSINTERFACE_N+1 ]; 	/*!< Left child in binary search tree */
	int rchild[ IMG3_LZSSINTERFACE_N+257 ]; /*!< Right child in binary search tree */
	int parent[ IMG3_LZSSINTERFACE_N+1 ]; 	/*!< Parent in binary search trees. */
    uint8_t text_buf[ IMG3_LZSSINTERFACE_N + IMG3_LZSSINTERFACE_F - 1 ]; /*!< Ring buffer of size N, with extra F-1 bytes to aid string comparison */
    int match_position;	/*!< Match position of longest match */
	int match_length;	/*!< Match length of longest match */
};

//! IMG3_LzssInteface class
/*!	
	The IMG3_LzssInterface class is used to provide wrappers around the opensource LZSS code
	that was found online.  The code mostly worked, but didn't handle everything exactly right.
	Whether that's because Apple is doing it wrong/differently, or because the original author
	was wrong, is anyone's guess.  However, this code does correctly decompress and compress
	Apple Firmware data.
*/

class IMG3_LzssInterface {
private:
	//! Private int32_t variable 
	/*! This variable represents any error encoutered during compression or decompression. */
	int32_t errorCode;

	//! Private function
	/*! This function is used to initialize the state machine used in the compression and
		decompression routines. */
	void InitState( struct encode_state *sp );											

	//! Private function
	/*/ Insert node into the encoding binary search tree. */
	void InsertNode( struct encode_state *sp, int r );

	//! Private function
	/*! Delete node from the encoding binary search tree. */
	void DeleteNode( struct encode_state *sp, int p );
	
	//! Private function
	/*! Generate adler32 checksum for the given data. */
	uint32_t lzadler32( uint8_t *buf, int32_t len );
	
	//! Private function
	/*! Decompress data using LZSS decompression. */
	uint32_t Decompress( uint8_t *dst, uint8_t *src, uint32_t srclen );
	
	//! Private function
	/*! Compress data using LZSS compression. */
	uint8_t * Compress( uint8_t *dst, uint32_t dstlen, uint8_t *src, uint32_t srclen );

public:
	//! IMG3_LzssInterface constructor.
	IMG3_LzssInterface();

	//! IMG3_LzssInterface destructor.
	virtual ~IMG3_LzssInterface();

	//! LzssDecompress public function.
	/*! This function may be called to decompress a block of LZSS compressed data.
		\param inbuff a pointer to the LZSS compressed data
		\param insize the length of the LZSS compressed data
		\param outbuff a double pointer that will be allocated automatically to store the 
					decompressed data
		\param outsize a pointer to a variable that will be used to store the length of the 
					decompressed data
	*/
	int32_t LzssDecompress( uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize );
	
	//! LzssCompress public function.
	/*! This function may be called to compress a block of data using LZSS compression.
		\param inbuff a pointer to the data to be compressed
		\param insize the length of the data to be compressed
		\param outbuff a double pointer that will be allocated automatically to store the 
					compressed data
		\param outsize a pointer to a variable that will be used to store the length of the
					compressed data
	*/
	int32_t LzssCompress( uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize );
	
	//! IsFileCompressed public function.
	/*! This function may be called to determine if a given block of data is compressed with 
		LZSS compression.  The function returns zero if it is a valid LZSS compressed block; 
		otherwise, it returns -1.
		\param inbuff a pointer to the block of data to evaluate
	*/
	int32_t IsFileCompressed( uint8_t *inbuff );
	
	//! GetError public function.
	/*! This function simply returns the last known error. */
	int32_t GetError( void ) { return errorCode; }
};

#endif /* IMG3_LZSSINTERFACE_H_ */
