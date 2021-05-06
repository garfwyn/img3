/*!	\file 		IMG3_FileSection.h
	\author		Matthew Areno
	\date		March 21, 2011
	\version	1.0

	This header file is used to define the different sections that exist inside of an img3 file.
*/



#ifndef IMG3_FILE_SECTION_H_
#define IMG3_FILE_SECTION_H_

#include <stdint.h>
#include "IMG3_defines.h"

/*!	\def	IMG3_MAGIC
	\brief 	Magic value for img3 files
*/
/*!	\def	IMG3_DATA_MAGIC
	\brief	Magic value for img3 DATA sections
*/
/*!	\def	IMG3_VERS_MAGIC
	\brief	Magic value for img3 VERS sections
*/
/*!	\def	IMG3_SEPO_MAGIC
	\brief	Magic value for img3 SEPO sections
*/
/*!	\def	IMG3_SCEP_MAGIC
	\brief	Magic value for img3 SCEP sections
*/
/*!	\def	IMG3_BORD_MAGIC
	\brief	Magic value for img3 BORD sections
*/
/*!	\def	IMG3_BDID_MAGIC
	\brief	Magic value for img3 BDID sections
*/
/*!	\def	IMG3_SHSH_MAGIC
	\brief	Magic value for img3 SHSH sections
*/
/*!	\def	IMG3_CERT_MAGIC
	\brief	Magic value for img3 CERT sections
*/
/*!	\def	IMG3_KBAG_MAGIC
	\brief	Magic value for img3 KBAG sections
*/
/*!	\def	IMG3_TYPE_MAGIC
	\brief	Magic value for img3 TYPE sections
*/
/*!	\def	IMG3_SDOM_MAGIC
	\brief	Magic value for img3 SDOM sections
*/
/*!	\def	IMG3_PROD_MAGIC
	\brief	Magic value for img3 PROD sections
*/
/*!	\def	IMG3_CHIP_MAGIC
	\brief	Magic value for img3 CHIP sections
*/
/*!	\def	IMG3_ECID_MAGIC
	\brief	Magic value for img3 ECID sections
*/

#define IMG3_MAGIC  	0x496D6733
#define IMG3_DATA_MAGIC 0x44415441
#define IMG3_VERS_MAGIC 0x56455253
#define IMG3_SEPO_MAGIC 0x5345504F
#define IMG3_SCEP_MAGIC 0x53434550
#define IMG3_BORD_MAGIC 0x424F5244
#define IMG3_BDID_MAGIC 0x42444944
#define IMG3_SHSH_MAGIC 0x53485348
#define IMG3_CERT_MAGIC 0x43455254
#define IMG3_KBAG_MAGIC 0x4B424147
#define IMG3_TYPE_MAGIC 0x54595045
#define IMG3_SDOM_MAGIC 0x53444F4D
#define IMG3_PROD_MAGIC 0x50524F44
#define IMG3_CHIP_MAGIC 0x43484950
#define IMG3_ECID_MAGIC 0x45434944

#ifndef MAX_PATH
#define MAX_PATH 2048
#endif

#define MAX_IV_SIZE         16
#define MAX_KEY_SIZE        32

#define IMG3_FILE_SECTION_NUM_ERRORS			0x0005

#define IMG3_FILE_SECTION_ERROR_NONE			0x0000
#define IMG3_FILE_SECTION_ERROR_INVALID_DATA	0x0001
#define IMG3_FILE_SECTION_ERROR_INVALID_FORMAT	0x0002
#define IMG3_FILE_SECTION_ERROR_LENGTHS_DIFFER	0x0003
#define IMG3_FILE_SECTION_ERROR_TAIL_SECTION	0x0004

/*
char fileSection_errorMessage[IMG3_FILE_SECTION_NUM_ERRORS][256] = {
	"No error",
	"No valid img3 section found",
	"File does not have conform to the img3 format",
	"Img3 length and file length differ",
	"Copy of tail section failed",
};
*/

//! IMG3_SectionType enum
/*! This enum defines all currently supported sections within an img3 file. */
typedef enum IMG3_SectionType {
	IMG3_BASE = 0,	/*!< Declaration of base img3 file header. */
	IMG3_CERT,		/*!< Declaration of CERT section. */
	IMG3_DATA,		/*!< Declaration of DATA section. */
	IMG3_KBAG,		/*!< Declaration of KBAG section. */
	IMG3_SEPO,		/*!< Declaration of SEPO section. */
	IMG3_SHSH,		/*!< Declaration of SHSH section. */
	IMG3_TYPE,		/*!< Declaration of TYPE section. */
	IMG3_SDOM,		/*!< Declaration of SDOM section. */
	IMG3_PROD,		/*!< Declaration of PROD section. */
	IMG3_CHIP,		/*!< Declaration of CHIP section. */
	IMG3_ECID,		/*!< Declaration of ECID section. */
	IMG3_VERS,		/*!< Declaration of VERS section. */
	IMG3_SCEP,		/*!< Declaration of SCEP section. */
	IMG3_BORD,		/*!< Declaration of BORD section. */
	IMG3_BDID,		/*!< Declaration of BDID sectino. */
	IMG3_UNKNOWN	/*!< Declaration of an unknown section within an img3 file. */
}IMG3_SectionType;

//!	IMG3_Generic_Header
/*!	A structure representing the generic section header found in img3 files. */

typedef struct IMG3_Generic_Header {
	uint32_t  magic;		/*!< the magic value identifying the section */
	uint32_t  totalLength;	/*!< the total length of the section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the section */
}__attribute__((__packed__)) IMG3_Generic_Header;

//! IMG3_Struct
/*! A structure representing an Img3 section in an img3 file. */

typedef struct IMG3_Struct {
	uint32_t  magic;		/*!< the magic value for an Img3 section */
	uint32_t  totalLength;	/*!< the total length of the Img3 section */
	uint32_t  dataLength;	/*!< the length of the data portion of the Img3 section */
	uint32_t  shshOffset;	/*!< the offset, from the start of the data, to the shsh. */
	uint32_t  name;			/*!< a tag identifying the type of img3 file, i.e. krnl, illb, ibot */
}__attribute__((__packed__)) IMG3_Struct;

//! IMG3_CERT_Struct
/*! A structure representing a CERT section in an img3 file. */

typedef struct IMG3_CERT_Struct {
	uint32_t  magic;		/*!< the magic value for a CERT section */
	uint32_t  totalLength;	/*!< the total length of the CERT section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the CERT section */
	uint8_t   *data;		/*!< a pointer to the data portion of the CERT section */
}__attribute__((__packed__)) CERT_Struct;

//! IMG3_DATA_Struct
/*! A structure representing a DATA section in an img3 file. */

typedef struct IMG3_DATA_Struct {
	uint32_t  magic;		/*!< the magic value for a DATA section */
	uint32_t  totalLength;	/*!< the total length of the DATA section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the DATA section */
	uint8_t   *data;		/*!< a pointer to the data portion of the DATA section */
}__attribute__((__packed__)) DATA_Struct;

//! IMG3_KBAG_Struct
/*! A structure representing a KBAG section in an img3 file. */

typedef struct IMG3_KBAG_Struct {
	uint32_t  magic;		/*!< the magic value for a KBAG section */
	uint32_t  totalLength;	/*!< the total length of the KBAG section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the KBAG section */
	uint32_t  state;		/*!< value defining crypto state, 1 means the iv and key are encrypted with the GID */
	uint32_t  aes_type;		/*!< 0x80 = aes128 / 0xc0 = aes192 / 0x100 = aes256 */
	uint8_t   *iv;			/*!< iv portion of KBAG section */
	uint8_t   *key;			/*!< key portion of the KBAG section */
}__attribute__((__packed__)) KBAG_Struct;

//! IMG3_SEPO_Struct
/*! A structure representing a SEPO section in an img3 file. */

typedef struct IMG3_SEPO_Struct {
	uint32_t  magic;		/*!< the magic value for a SEPO section */
	uint32_t  totalLength;	/*!< the total length of the SEPO section, hearder included */
	uint32_t  dataLength;	/*!< the length of the data portion of the SEPO section */
	uint32_t  unknown1;		/*!< unknown value */
	uint32_t  unknown2;		/*!< unknown value */
	uint32_t  data;			/*!< data value */
}__attribute__((__packed__)) SEPO_Struct;

//! IMG3_SHSH_Struct
/*!	A structure representing a SHSH section in an img3 file. */

typedef struct IMG3_SHSH_Struct {
	uint32_t  magic;		/*!< the magic value for a SHSH section */
	uint32_t  totalLength;	/*!< the total length of the SHSH section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the SHSH section */
	uint8_t   *data;		/*!< a pointer to the data portion of the SHSH section */
}__attribute__((__packed__)) SHSH_Struct;

//! IMG3_TYPE_Struct
/*!	A structure representing a TYPE section in an img3 file. */

typedef struct IMG3_TYPE_Struct {
	uint32_t  magic;		/*!< the magic value for a TYPE section */
	uint32_t  totalLength;	/*!< the total length of the TYPE section, header included */
	uint32_t  dataLength;	/*!< the length of the data portion of the TYPE section */
	uint32_t  name;			/*!< a name identifying the type of img3 file, i.e. krnl, illb, ibot */
	uint32_t  unknown1;		/*!< unknown value (typically zeros) */
	uint32_t  unknown2;		/*!< unknown value (typically zeros) */
	uint32_t  unknown3;		/*!< unknown value (typically zeros) */
	uint32_t  data;			/*!< data value (typically zeros) */
}__attribute__((__packed__)) TYPE_Struct;

//! IMG3_FileSection class
/*!
	The FileSection class is a simple class that maintains three pieces of information about the
	corresponding section: its type, its length, and its data.  The data variable is dynamically
	allocated and contains a copy of the data from the original file.  This data will be released
	automatically by the deconstructor.
*/

class IMG3_FileSection {
private:
	//! Private IMG3_Generic_Header variable.
	/*! This variable maintains the type of the corresponding section.
	*/
	IMG3_Generic_Header header;
	
	//! Private uint8_t * variable.
	/*! This variable maintains a copy of the data portion of the original file.
	*/
	uint8_t	*data;

	//! Private uint32_t variable.
	/*! This variable maintains a pointer to the section header in the original file.
	*/
	uint8_t	*original;

	uint32_t errorCode;

public:
	//! IMG3_FileSection constructor.
	IMG3_FileSection();
	//! IMG3_FileSection destructor.
	virtual ~IMG3_FileSection();

	//! ParseSection function.
	/*! This function parsed examines a given block of data to determine which type of section it
		represents.  It will then set type, data, and dataLength accordingly.
		\param section a pointer to a block of data to be analyzed.
	*/
	int32_t ParseSection(uint8_t *section);

	//! WriteData public function.
	/*! This function may be used to overwrite the data portion of the object.
		\param [in] newData a pointer to the new data that should be written
		\param [in] length the length of the new data
		\return [out] int32_t 0 for success, -1 otherwise
	*/
	int32_t WriteData( uint8_t *newData, uint32_t length );

	uint32_t GetSectionTotalLength() { return header.totalLength; }

	uint32_t GetSectionHeaderLength() { return header.totalLength - header.dataLength; }

	uint32_t GetSectionDataLength() { return header.dataLength; }

	//! GetSectionData function.
	/*! This function simply returns a pointer to the corresponding data.
	*/
	uint8_t * GetSectionData() { return data; }
	
	//! GetSectionType function.
	/*! This function simply returns the type of the corresponding section.
	*/
	IMG3_SectionType GetSectionType();
};

#endif /* IMG3_FILE_SECTION_H_ */
