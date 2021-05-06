/*
 * IMG3_LzssInterface.cpp
 *
 *  Created on: Mar 22, 2011
 *      Author: mareno
 */

#include <stdio.h>
#include <arpa/inet.h>
#include "IMG3_LzssInterface.h"
#include "IMG3_defines.h"

/*! \fn 	IMG3_LzssInterface()
	\brief	Constructor for IMG3_LzssInterface class
*/

IMG3_LzssInterface::IMG3_LzssInterface() {
}

/*! \fn		~IMG3_LzssInterface()
	\brief	Deconstructor for IMG3_LzssInterface class
*/

IMG3_LzssInterface::~IMG3_LzssInterface() {
}

/*! \fn		int32_t LzssDecompress( uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize )
	\brief	Publically available LZSS decompression routine
	\param	inbuff pointer to the buffer containing the data to be decompressed
	\param	insize length of the inbuff in bytes
	\param	outbuff address of a pointer to a buffer to store the decompressed data; buffer will be automatically allocated by the function
	\param	outsize address of a size_t variable in which to store the length of outbuff
*/

int32_t IMG3_LzssInterface::LzssDecompress(uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize)
{
	IMG3_LzssInterface_CompressionHeader *header;
	uint32_t checksum;

	CLASS_VALIDATE_PARAMETER( inbuff, -1 );
	CLASS_VALIDATE_PARAMETER( outbuff, -1 );
	CLASS_VALIDATE_PARAMETER( outsize, -1 );

	if (insize < sizeof(IMG3_LzssInterface_CompressionHeader)) {
		errorCode = IMG3_LZSS_ERROR_INPUT_TOO_SMALL;
		PRINT_CLASS_ERROR( "input size too small to hold lzss header" );
		return -1;
	}

	header = (IMG3_LzssInterface_CompressionHeader *)inbuff;

	if (IsFileCompressed(inbuff) == -1) {
		errorCode = IMG3_LZSS_ERROR_FILE_IS_NOT_COMPRESSED;
		PRINT_CLASS_ERROR( "file is not compressed" );
		return -1;
	}

	*outsize = ntohl(header->length_uncompressed);
	*outbuff = new uint8_t[*outsize];
	if (*outbuff == NULL) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	Decompress((uint8_t *)*outbuff, (uint8_t *)(header+1), ntohl(header->length_compressed));

	checksum = lzadler32(*outbuff, *outsize);
	if (ntohl(header->checksum) != checksum) {
#ifdef IMG3_DEBUG
		errorCode = IMG3_LZSS_ERROR_CHECKSUM_MISMATCH;
		PRINT_CLASS_ERROR( "header checksum does not match calculated checksum" );
#endif
	}

	return 0;
}

/*! \fn		int32_t LzssCompress( uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize )
	\brief	Publically available LZSS compression routine
	\param	inbuff pointer to the buffer containing the data to be compressed
	\param	insize length of the inbuff in bytes
	\param	outbuff address of a pointer to a buffer to store the compressed data; buffer will be automatically allocated by the function
	\param	outsize address of a size_t variable in which to store the length of outbuff
*/

int32_t IMG3_LzssInterface::LzssCompress(uint8_t *inbuff, size_t insize, uint8_t **outbuff, size_t *outsize)
{
	IMG3_LzssInterface_CompressionHeader *header;
	uintptr_t outend;
	uint8_t *curPtr;
	uint32_t bytesCompressed = 0, bytesToCompress = insize;

	CLASS_VALIDATE_PARAMETER( inbuff, -1 );
	CLASS_VALIDATE_PARAMETER( outbuff, -1 );
	CLASS_VALIDATE_PARAMETER( outsize, -1 );

	if ( insize < sizeof(IMG3_LzssInterface_CompressionHeader) ) {
		errorCode = IMG3_LZSS_ERROR_INPUT_TOO_SMALL;
		PRINT_CLASS_ERROR( "input size too small to hold lzss header" );
		return -1;
	}

	*outsize = insize * 2 + sizeof(IMG3_LzssInterface_CompressionHeader);
	*outbuff = new uint8_t[*outsize];
	if (outbuff == NULL) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	header = (IMG3_LzssInterface_CompressionHeader *)*outbuff;

	outend = (uintptr_t)Compress((uint8_t *)(header + 1), *outsize, inbuff, insize);
	if (outend == 0) {
		errorCode = IMG3_LZSS_ERROR_COMPRESSION_FAILED;
		PRINT_CLASS_ERROR( "compression failed" );
		return -1;
	}	

	*outsize = outend - (uintptr_t)*outbuff;
	if (*outsize % 16)
		*outsize += 16 - (*outsize % 16);

	header->signature = htonl(IMG3_LZSSINTERFACE_COMP_SIGNATURE);
	header->compression_type = htonl(IMG3_LZSSINTERFACE_LZSS_SIGNATURE);
	header->checksum = htonl(lzadler32(inbuff, insize));
	header->length_uncompressed = htonl(insize);
	header->length_compressed = htonl(outend - (uintptr_t)(header + 1));
	memset(header->padding,0,sizeof(header->padding));

	return 0;
}

/*! \fn		int32_t IsFileCompressed( uint8_t *inbuff )
	\brief	Publically available routine for determining if data is compressed
	\param	inbuff pointer to the buffer containing possible compressed data
*/

int32_t IMG3_LzssInterface::IsFileCompressed(uint8_t *inbuff)
{
	IMG3_LzssInterface_CompressionHeader *header;

	CLASS_VALIDATE_PARAMETER(inbuff, -1);

	header = (IMG3_LzssInterface_CompressionHeader *)inbuff;

	header = (IMG3_LzssInterface_CompressionHeader *)inbuff;

	if (ntohl(header->signature) != IMG3_LZSSINTERFACE_COMP_SIGNATURE && ntohl(header->compression_type) != IMG3_LZSSINTERFACE_LZSS_SIGNATURE) {
		errorCode = IMG3_LZSS_ERROR_FILE_NOT_A_LZSS_FILE;
		PRINT_CLASS_ERROR( "file not a valid lzss archive" );
		return -1;
	}
	return 0;
}

/*!	\fn		uint32_t lzadler32( uint8_t *buf, int32_t len )
	\brief	Private function for computing the adler32 checksum of the provided data
	\param	buf pointer to the data on which to perform the checksum
	\param  len size of buf buffer in bytes
*/

uint32_t IMG3_LzssInterface::lzadler32(uint8_t *buf, int32_t len)
{
	unsigned long s1 = 1;
	unsigned long s2 = 0;
	int k;

	for ( k = 0; k < len; k++ )
	{
		s1 = (s1 + buf[k]) % 65521;
		s2 = (s2 + s1) % 65521;
	}
	
	return (s2 << 16) | s1;
}

/*!	\fn	int Decompress( uint8_t *dst, uint8_t *src, uint32_t srclen
	\brief	Private decompression method for performing LZSS decompression
	\param	dst pointer to a buffer to store the result of the decompression
	\param	src pointer to the buffer containing the data to be decompressed
	\param	srclen size of src buffer in bytes
*/

uint32_t IMG3_LzssInterface::Decompress(uint8_t *dst, uint8_t *src, uint32_t srclen)
{
	/* ring buffer of size N, with extra F-1 bytes to aid string comparison */
	uint8_t text_buf[IMG3_LZSSINTERFACE_N + IMG3_LZSSINTERFACE_F - 1];
	uint8_t *dststart = dst;
	uint8_t *srcend = src + srclen;
	int i, j, k, r, c;
	unsigned int flags;

	dst = dststart;
	srcend = src + srclen;
	for (i = 0; i < IMG3_LZSSINTERFACE_N - IMG3_LZSSINTERFACE_F; i++)
		text_buf[i] = ' ';
	r = IMG3_LZSSINTERFACE_N - IMG3_LZSSINTERFACE_F;
	flags = 0;
	for ( ; ; ) {
		if (((flags >>= 1) & 0x100) == 0) {
			if (src < srcend) c = *src++; else break;
			flags = c | 0xFF00;  /* uses higher byte cleverly */
		}  /* to count eight */
		if (flags & 1) {
			if (src < srcend) c = *src++; else break;
			*dst++ = c;
			text_buf[r++] = c;
			r &= (IMG3_LZSSINTERFACE_N - 1);
		} else {
			if (src < srcend) i = *src++; else break;
			if (src < srcend) j = *src++; else break;
			i |= ((j & 0xF0) << 4);
			j = (j & 0x0F) + IMG3_LZSSINTERFACE_THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (IMG3_LZSSINTERFACE_N - 1)];
				*dst++ = c;
				text_buf[r++] = c;
				r &= (IMG3_LZSSINTERFACE_N - 1);
			}
		}
	}

	return dst - dststart;
}

/*! \fn		uint8_t * Compress( uint8_t *dst, uint32_t dstlen, uint8_t *src, uint32_t srclen )
	\brief	Private compression method for performing LZSS compression
	\param	dst pointer to a buffer to store the result of the compression
	\param	dstlen size in bytes of the dst buffer
	\param	src pointer to the buffer containing the data to be compressed
	\param 	srclen size in bytes of the src buffer
*/

uint8_t * IMG3_LzssInterface::Compress(uint8_t *dst, uint32_t dstlen, uint8_t *src, uint32_t srclen)
{
	/* Encoding state, mostly tree but some current match stuff */
	struct encode_state *sp;

	int i, c, len, r, s, last_match_length, code_buf_ptr;
	uint8_t code_buf[17], mask;
	uint8_t *srcend = src + srclen;
	uint8_t *dstend = dst + dstlen;

	/* initialize trees */
	sp = new struct encode_state;
	InitState(sp);

	/*
	 * code_buf[1..16] saves eight units of code, and code_buf[0] works
	 * as eight flags, "1" representing that the unit is an unencoded
	 * letter (1 byte), "" a position-and-length pair (2 bytes).
	 * Thus, eight units require at most 16 bytes of code.
	 */
	code_buf[0] = 0;
	code_buf_ptr = mask = 1;

	/* Clear the buffer with any character that will appear often. */
	s = 0; r = IMG3_LZSSINTERFACE_N - IMG3_LZSSINTERFACE_F;

	/* Read F bytes into the last F bytes of the buffer */
	for (len = 0; len < IMG3_LZSSINTERFACE_F && src < srcend; len++)
		sp->text_buf[r + len] = *src++;
	if (!len) {
		delete(sp);
		return NULL; /* text of size zero */
	}
	/*
	 * Insert the F strings, each of which begins with one or more
	 * 'space' characters.  Note the order in which these strings are
	 * inserted.  This way, degenerate trees will be less likely to occur.
	 */
	for (i = 1; i <= IMG3_LZSSINTERFACE_F; i++)
		InsertNode(sp, r - i);

	/*
	 * Finally, insert the whole string just read.
	 * The global variables match_length and match_position are set.
	 */
	InsertNode(sp, r);
	do {
		/* match_length may be spuriously long near the end of text. */
		if (sp->match_length > len)
			sp->match_length = len;
		if (sp->match_length <= IMG3_LZSSINTERFACE_THRESHOLD) {
			sp->match_length = 1;  /* Not long enough match.  Send one byte. */
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = sp->text_buf[r]; /* Send uncoded. */
		} else {
			/* Send position and length pair.  Note match_length > THRESHOLD. */
			code_buf[code_buf_ptr++] = (uint8_t) sp->match_position;
			code_buf[code_buf_ptr++] = (uint8_t)
                						( ((sp->match_position >> 4) & 0xF0)
                								|  (sp->match_length - (IMG3_LZSSINTERFACE_THRESHOLD +1)) );
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			/* Send at most 8 units of code together */
			for (i = 0; i < code_buf_ptr; i++)
				if (dst < dstend)
					*dst++ = code_buf[i];
				else {
					delete(sp);
					return NULL;
				}
			code_buf[0] = 0;
			code_buf_ptr = mask = 1;
		}
		last_match_length = sp->match_length;
		for (i = 0; i < last_match_length && src < srcend; i++) {
			DeleteNode(sp, s);  /* Delete old strings and */
			c = *src++;
			sp->text_buf[s] = c; /* read new bytes */

			/*
			 * If the position is near the end of buffer, extend the buffer
			 * to make string comparison easier.
			 */
			if (s < IMG3_LZSSINTERFACE_F - 1)
				sp->text_buf[s + IMG3_LZSSINTERFACE_N] = c;

			/* Since this is a ring buffer, increment the position modulo N. */
			s = (s + 1) & (IMG3_LZSSINTERFACE_N - 1);
			r = (r + 1) & (IMG3_LZSSINTERFACE_N - 1);

			/* Register the string in text_buf[ r..r+F-1] */
			InsertNode(sp, r);
		}
		while (i++ < last_match_length) {
			DeleteNode(sp, s);

			/* After the end of text, no need to read. */
			s = (s + 1) & (IMG3_LZSSINTERFACE_N - 1);
			r = (r + 1) & (IMG3_LZSSINTERFACE_N - 1);
			/* but buffer may not be empty. */
			if (--len)
				InsertNode(sp , r);
		}
	} while (len > 0);  /* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {  /* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++)
			if (dst < dstend)
				*dst++ = code_buf[i];
			else {
				delete(sp);
				return NULL;
			}
	}

	delete(sp);
	return dst;
}

/*!	\fn		void InitState( struct encode_state *sp )
	\brief	Private method for restoring compression information to its initial state
	\param	sp pointer to the class instance's encode_state structure
*/

void IMG3_LzssInterface::InitState(struct encode_state *sp)
{
	int  i;

    bzero(sp, sizeof(*sp));

    for (i = 0; i < IMG3_LZSSINTERFACE_N - IMG3_LZSSINTERFACE_F; i++)
        sp->text_buf[i] = ' ';
    for (i = IMG3_LZSSINTERFACE_N + 1; i <= IMG3_LZSSINTERFACE_N + 256; i++)
        sp->rchild[i] = IMG3_LZSSINTERFACE_NIL;
    for (i = 0; i < IMG3_LZSSINTERFACE_N; i++)
        sp->parent[i] = IMG3_LZSSINTERFACE_NIL;
}

/*! \fn		void InsertNode( struct encode_state *sp, int r )
	\brief	Private method for inserting a node into the binary search tree used for compression
	\param	sp pointer to the class instance's encode_state structure
	\int	r value of the node to be inserted
*/

void IMG3_LzssInterface::InsertNode(struct encode_state *sp, int r)
{
	int  i, p, cmp;
    u_int8_t  *key;

    cmp = 1;
    key = &sp->text_buf[r];
    p = IMG3_LZSSINTERFACE_N + 1 + key[0];
    sp->rchild[r] = sp->lchild[r] = IMG3_LZSSINTERFACE_NIL;
    sp->match_length = 0;
    for ( ; ; ) {
        if (cmp >= 0) {
            if (sp->rchild[p] != IMG3_LZSSINTERFACE_NIL)
                p = sp->rchild[p];
            else {
                sp->rchild[p] = r; 
                sp->parent[r] = p;
                return;
            }
        } else {
            if (sp->lchild[p] != IMG3_LZSSINTERFACE_NIL)
                p = sp->lchild[p];
            else {
                sp->lchild[p] = r;
                sp->parent[r] = p;
                return;
            }
        }
        for (i = 1; i < IMG3_LZSSINTERFACE_F; i++) {
            if ((cmp = key[i] - sp->text_buf[p + i]) != 0)
                break;
        }
        if (i > sp->match_length) {
            sp->match_position = p;
            if ((sp->match_length = i) >= IMG3_LZSSINTERFACE_F)
                break;
        }
    }
    sp->parent[r] = sp->parent[p];
    sp->lchild[r] = sp->lchild[p];
    sp->rchild[r] = sp->rchild[p];
    sp->parent[sp->lchild[p]] = r;
    sp->parent[sp->rchild[p]] = r;
    if (sp->rchild[sp->parent[p]] == p)
        sp->rchild[sp->parent[p]] = r;
    else
        sp->lchild[sp->parent[p]] = r;
    sp->parent[p] = IMG3_LZSSINTERFACE_NIL;  /* remove p */
}

/*!	\fn		void DeleteNode( struct encode_state *sp, int p )
	\brief	Private method for deleting a node from the binary search tree used for compression
	\param	sp pointer to the class instance's encode_state structure
	\param	p value of the node to be deleted
*/

void IMG3_LzssInterface::DeleteNode(struct encode_state *sp, int p)
{
	int  q;
    
    if (sp->parent[p] == IMG3_LZSSINTERFACE_NIL)
        return;  /* not in tree */
    if (sp->rchild[p] == IMG3_LZSSINTERFACE_NIL)
        q = sp->lchild[p];
    else if (sp->lchild[p] == IMG3_LZSSINTERFACE_NIL)
        q = sp->rchild[p];
    else {
        q = sp->lchild[p];
        if (sp->rchild[q] != IMG3_LZSSINTERFACE_NIL) {
            do {
                q = sp->rchild[q];
            } while (sp->rchild[q] != IMG3_LZSSINTERFACE_NIL);
            sp->rchild[sp->parent[q]] = sp->lchild[q];
            sp->parent[sp->lchild[q]] = sp->parent[q];
            sp->lchild[q] = sp->lchild[p];
            sp->parent[sp->lchild[p]] = q;
        }
        sp->rchild[q] = sp->rchild[p];
        sp->parent[sp->rchild[p]] = q;
    }
    sp->parent[q] = sp->parent[p];
    if (sp->rchild[sp->parent[p]] == p)
        sp->rchild[sp->parent[p]] = q;
    else
        sp->lchild[sp->parent[p]] = q;
    sp->parent[p] = IMG3_LZSSINTERFACE_NIL;
}
