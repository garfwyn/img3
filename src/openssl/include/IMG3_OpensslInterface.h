/*
 * IMG3_OpensslInterface.h
 *
 *  Created on: Mar 18, 2011
 *      Author: Matthew
 */

#ifndef IMG3_OPENSSLINTERFACE_H_
#define IMG3_OPENSSLINTERFACE_H_

#include <stdint.h>
#include <openssl/evp.h>

class IMG3_OpensslInterface {
private:
	uint8_t *key;
	uint8_t *iv;
	uint32_t keySize;
	uint32_t ivSize;

	void HexToBytes(const char *hex, uint8_t *buffer, uint32_t bytes);

public:
	IMG3_OpensslInterface();
	IMG3_OpensslInterface(char *newKey, char *newIV);
	virtual ~IMG3_OpensslInterface();

	int32_t SetKeyAndIV(char *newKey, char *newIV);
	int32_t DecryptData(uint8_t *input, uint32_t length, uint8_t **output);
	int32_t EncryptData(uint8_t *input, uint32_t length, uint8_t **output);
};

#endif /* IMG3_OPENSSLINTERFACE_H_ */
