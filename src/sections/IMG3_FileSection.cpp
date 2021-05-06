/*!	\file		IMG3_Section.cpp
	\author		Matthew Areno
	\date		March 21, 2011
	\version	1.0
*/

#include <stdio.h>
#include <stdlib.h>
#include "IMG3_FileSection.h"

//! IMG3_FileSection constructor
/*! This function provides the basic initialization for an IMG3_FileSection object.  The header variable is zeroed out, and the
	data and original pointers are set to NULL.
*/

IMG3_FileSection::IMG3_FileSection() {
	memset( &header, 0, sizeof( header ) );
	data = NULL;
	original = NULL;
}

//!	~IMG3_FileSection destructor
/*! This function provides the basic destruction functionality for an IMG3_FileSection object.  The only member to be concerned
	about is the data pointer.  If the section was used, then this variable will possibly point to an allocated section of 
	memory.  If so, deallocate it prior to destroying this object.
*/

IMG3_FileSection::~IMG3_FileSection() {
	if ( data != NULL )
		free( data );
}

//! IMG3_FileSection::ParseSection function
/*!	This function is used to parse a specific block of data and allocated a FileSection object to represent the section.  Depending
	upon the section found, it will be handled differently.  An Img3 section has no data element and contains offset values that
	are based upon the overall length of the entire file.  All other section contain a simple header and a data field.  The size of 
	header can be found by subtracting the dataLength field from the totalLength field.  The data variable will be dynamically 
	allocated and will store a copy of the original data portion of the corresponding section.  This is because if patching is used,
	we need to be able to modify the data and write out a new file.  We also maintain a pointer to the original header in case there
	is anything else we might need at a later time.
	\param section a uint8_t pointer that points to the block of data to be parsed
	\return int32_t 0 for success, -1 otherwise
*/

int32_t IMG3_FileSection::ParseSection(uint8_t *section)
{
	IMG3_Generic_Header *sectionHeader;
	uint32_t dataOffset;

	ASSERT_RET( section, -1 );

	// Every section should start with the generic header
	sectionHeader = ( IMG3_Generic_Header * ) section;	

	// From there, we can look at the magic value to determine what type of section we're looking at
	switch( sectionHeader->magic ) {
	case IMG3_MAGIC:
		// If it's the main Img3 header, there is no data section.
		header.magic = sectionHeader->magic;
		header.totalLength = sectionHeader->totalLength;
		header.dataLength = sectionHeader->dataLength;
		original = section;
		data = NULL;
		break;
	case IMG3_DATA_MAGIC:
		// Otherwise, there may be a data section and we need to check.
		header.magic = sectionHeader->magic;
		header.totalLength = sectionHeader->totalLength;
		header.dataLength = sectionHeader->dataLength;
		original = section;
		// If the dataLength field is not zero, allocate enought room to hold the data and then copy it over.
		if ( header.dataLength != 0 ) {
			data = ( uint8_t * ) malloc ( header.dataLength );
			if ( data == NULL ) {
				goto PARSE_SECTION_ERROR;
			}
			dataOffset = 12;
			memcpy( data, section + dataOffset, header.dataLength );
		// Otherwise, just set it to NULL and move on
		} else {
			data = NULL;
		}
		break;
	case IMG3_VERS_MAGIC:
	case IMG3_SEPO_MAGIC:
	case IMG3_SCEP_MAGIC:
	case IMG3_BORD_MAGIC:
	case IMG3_BDID_MAGIC:
	case IMG3_SHSH_MAGIC:
	case IMG3_CERT_MAGIC:
	case IMG3_KBAG_MAGIC:
	case IMG3_TYPE_MAGIC:
	case IMG3_SDOM_MAGIC:
	case IMG3_PROD_MAGIC:
	case IMG3_CHIP_MAGIC:
	case IMG3_ECID_MAGIC:
		// Otherwise, there may be a data section and we need to check.
		header.magic = sectionHeader->magic;
		header.totalLength = sectionHeader->totalLength;
		header.dataLength = sectionHeader->dataLength;
		original = section;
		// If the dataLength field is not zero, allocate enought room to hold the data and then copy it over.
		if ( header.dataLength != 0 ) {
			data = ( uint8_t * ) malloc ( header.dataLength );
			if ( data == NULL ) {
				goto PARSE_SECTION_ERROR;
			}
			dataOffset = sectionHeader->totalLength - sectionHeader->dataLength;
			memcpy( data, section + dataOffset, header.dataLength );
		// Otherwise, just set it to NULL and move on
		} else {
			data = NULL;
		}
		break;
	default:
		// If the section is not supported, print an error message and return.
		fprintf( stderr,"%s: Unsupported section type: %#08x.\n", __FUNCTION__, sectionHeader->magic );
		data = NULL;
		return -1;
	}
	// Once the section is successfully parsed, return SUCCESS
	return 0;
	
	// If a system error occurred, print it and return ERROR
PARSE_SECTION_ERROR:
	PRINT_SYSTEM_ERROR();
	return -1;
}

int32_t	IMG3_FileSection::WriteData( uint8_t *newData, uint32_t length  )
{
	ASSERT_RET( newData, -1 );
	ASSERT_RET( length, -1 );

	if( data == NULL ) {
		fprintf( stderr, "Data section for FileSection object doesn't appear to be used, but is being overwritten!\n" );
	} else {
		free( data );
	}

	data = ( uint8_t * ) malloc( length );
	if( data == NULL ) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	memcpy( data, newData, length );
	
	return 0;
}

IMG3_SectionType IMG3_FileSection::GetSectionType()
{
	switch( header.magic ) {
	case IMG3_MAGIC:
		return IMG3_BASE;
	case IMG3_DATA_MAGIC:
		return IMG3_DATA;
	case IMG3_VERS_MAGIC:
		return IMG3_VERS;
	case IMG3_SEPO_MAGIC:
		return IMG3_SEPO;
	case IMG3_SCEP_MAGIC:
		return IMG3_SCEP;
	case IMG3_BORD_MAGIC:
		return IMG3_BORD;
	case IMG3_BDID_MAGIC:
		return IMG3_BDID;
	case IMG3_SHSH_MAGIC:
		return IMG3_SHSH;
	case IMG3_CERT_MAGIC:
		return IMG3_CERT;
	case IMG3_KBAG_MAGIC:
		return IMG3_KBAG;
	case IMG3_TYPE_MAGIC:
		return IMG3_TYPE;
	case IMG3_SDOM_MAGIC:
		return IMG3_SDOM;
	case IMG3_PROD_MAGIC:
		return IMG3_PROD;
	case IMG3_CHIP_MAGIC:
		return IMG3_CHIP;
	case IMG3_ECID_MAGIC:
		return IMG3_ECID;
	default:
		return IMG3_UNKNOWN;
	}
}

