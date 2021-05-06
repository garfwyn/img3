/*
 * IMG3_OpensslInterface.cpp
 *
 *  Created on: Mar 18, 2011
 *      Author: Matthew
 */

#include <unistd.h>
#include <sys/wait.h>
#include "IMG3_OpensslInterface.h"
#include "IMG3_defines.h"

IMG3_OpensslInterface::IMG3_OpensslInterface()
{
	key = NULL;
	iv = NULL;
}

IMG3_OpensslInterface::IMG3_OpensslInterface(char *newKey, char *newIV)
{
	key = NULL;
	iv = NULL;

	SetKeyAndIV(newKey,newIV);
}

IMG3_OpensslInterface::~IMG3_OpensslInterface() {
	if (key != NULL)
		delete(key);
	if (iv != NULL)
		delete(iv);
}

void IMG3_OpensslInterface::HexToBytes(const char* hex, unsigned char* buffer, uint32_t bytes)
{
	size_t i;

	for(i = 0; i < bytes; i++) {
		uint32_t byte;
		sscanf(hex, "%02x", &byte);
		buffer[i] = byte;
		hex += 2;
	}
}

int32_t IMG3_OpensslInterface::SetKeyAndIV(char *newKey, char *newIV)
{
	uint32_t bits;
	
	ASSERT_RET(newKey,-1);
	ASSERT_RET(newIV,-1);

	if (key != NULL)
		delete(key);
	if (iv != NULL)
		delete(iv);

	keySize = strlen(newKey) >> 1;
	bits = keySize * 8;
	if (bits != 128 && bits != 192 && bits != 256) {
		fprintf(stderr,"%s: Invalid key provided: keySize = %d.\n", __FUNCTION__, keySize);
		return -1;
	}
	ivSize = strlen(newIV) >> 1;
	bits = ivSize * 8;
	if (bits != 128) {
		fprintf(stderr,"%s: Invalid IV provided: ivSize = %d.\n", __FUNCTION__, ivSize);
		return -1;
	}


	key = new uint8_t[keySize];
	if (!key) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	iv = new uint8_t[ivSize];
	if (!iv) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	HexToBytes(newKey, key, keySize);
	HexToBytes(newIV, iv, ivSize);

	return 0;
}

int32_t	IMG3_OpensslInterface::DecryptData(uint8_t *input, uint32_t length, uint8_t **output)
{
	int32_t bytesWritten;
	EVP_CIPHER_CTX dec;

	ASSERT_RET(input, -1);
	ASSERT_RET(length, -1);

	if ( *output == NULL ) {
		*output = new uint8_t[length];
		if ( output == NULL ) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		memset( *output, 0, length );
	}

	EVP_CIPHER_CTX_init( &dec );
	EVP_CIPHER_CTX_set_padding( &dec, 0 );
	EVP_DecryptInit_ex( &dec, EVP_aes_256_cbc(), NULL, key, iv );

	EVP_DecryptUpdate( &dec, *output, &bytesWritten, input, length );
	EVP_DecryptFinal_ex( &dec, *output+bytesWritten, &bytesWritten );

	return 0;
}

int32_t IMG3_OpensslInterface::EncryptData(uint8_t *input, uint32_t length, uint8_t **output)
{
	int32_t bytesWritten;
	EVP_CIPHER_CTX enc;

	ASSERT_RET(input, -1);
	ASSERT_RET(length, -1);

	if ( *output == NULL ) {
		*output = new uint8_t[length];
		if ( output == NULL ) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		memset( *output, 0, length );
	}

	EVP_CIPHER_CTX_init( &enc );
	EVP_CIPHER_CTX_set_padding( &enc, 0 );
	EVP_EncryptInit_ex( &enc, EVP_aes_256_cbc(), NULL, key, iv );

	EVP_EncryptUpdate( &enc, *output, &bytesWritten, input, length );
	EVP_EncryptFinal_ex( &enc, *output + bytesWritten, &bytesWritten );

	return bytesWritten;
}
