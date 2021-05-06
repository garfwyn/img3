/*!	\file		IMG3_FileInterface.cpp
	\author		Matthew Areno
	\date		March 21, 2011
	\version	1.0
*/

#include "IMG3_FileInterface.h"
#include "IMG3_defines.h"

//! IMG3_FileInteface constructor
/*!	This function provides the basic initialization for an IMG3_FileSection object.  The only task is ensure the sections list is 
	cleared before being used.
*/

IMG3_FileInterface::IMG3_FileInterface() {
	sections.clear();
}

//! ~IMG3_FileInterface destructor
/*!	This function provides the basic destruction for an IMG3_FileInterface object.  The sections list contains a collection of 
	dynamically allocated IMG3_FileSection objects.  These need to be released prior to destroying the IMG3_FileInterface object.
*/

IMG3_FileInterface::~IMG3_FileInterface() {
	list<IMG3_FileSection *>::iterator listIt;

	for (listIt = sections.begin(); listIt != sections.end(); ++listIt) {
		IMG3_FileSection *sec = *listIt;
		if (sec != NULL)
			delete(sec);
	}
	sections.clear();
}

//! IMG3_FileInterface::PrintSections function
/*!	This function is basically for debug purposes only.  It is used to visually list all sections found within a given img3 file.  It
	should be called after the sections list has populated; otherwise it's fairly useless. :)
*/

void IMG3_FileInterface::PrintSections()
{
	list< IMG3_FileSection * >::iterator listIt;

	if ( sections.empty() ) {
		fprintf( stdout, "The sections listing for this file is empty.\n");
		return;
	}

	for ( listIt = sections.begin(); listIt != sections.end(); ++listIt ) {
		IMG3_FileSection *sec = *listIt;
		switch( sec->GetSectionType() ) {
		case IMG3_BASE:
			fprintf( stdout, "Found BASE section.\n" );
			break;
		case IMG3_CERT:
			fprintf( stdout, "Found CERT section.\n" );
			break;
		case IMG3_DATA:
			fprintf( stdout, "Found DATA section.\n" );
			break;
		case IMG3_KBAG:
			fprintf( stdout, "Found KBAG section.\n" );
			break;
		case IMG3_SEPO:
			fprintf( stdout, "Found SEPO section.\n" );
			break;
		case IMG3_SHSH:
			fprintf( stdout, "Found SHSH section.\n" );
			break;
		case IMG3_TYPE:
			fprintf( stdout, "Found TYPE section.\n" );
			break;
		case IMG3_SDOM:
			fprintf( stdout, "Found SDOM section.\n" );
			break;
		case IMG3_PROD:
			fprintf( stdout, "Found PROD section.\n" );
			break;
		case IMG3_CHIP:
			fprintf( stdout, "Found CHIP section.\n" );
			break;
		case IMG3_ECID:
			fprintf( stdout, "Found ECID section.\n" );
			break;
		case IMG3_VERS:
			fprintf( stdout, "Found VERS section.\n" );
			break;
		case IMG3_SCEP:
			fprintf( stdout, "Found SCEP section.\n" );
			break;
		case IMG3_BORD:
			fprintf( stdout, "Found BORD section.\n" );
			break;
		case IMG3_BDID:
			fprintf( stdout, "Found BDID section.\n" );
			break;
		case IMG3_UNKNOWN:
			fprintf( stdout, "Found UNKNOWN section.\n" );
			break;
		default:
			fprintf( stderr, "Invalid section data.\n" );
			break;
		}
	}
	return;
}

//! IMG3_FileInterface::ParseFile function
/*! This file is used to parse through an img3 file looking for all available sections.  It simply reads the data four bytes at a time 
	looking for any recognized tags.  Once a supported tag is found, it will allocate a new IMG3_FileSection object and call the 
	IMG3_FileSection::ParseSection function to extract all the section information.  Once that's done, it will add the object to the 
	sections list.  Each section may contain multiple other sections.  A primary example of this is the CERT section.  The CERT section 
	typically contains an Img3, SDOM, PROD, and CHIP section inside of it.  For the purposes of this program, we are not concerned with
	these internal sections.  The only sections we are concern about are the one main sections and the initial Img3 section.  The size 
	fields in the Img3 header are based upon the size of all the main sections, not the internal sections.

	\param [in] fileData a pointer to the img3 file data, which should be memory mapped
	\param [in] fileLength the length of the img3 file data 
	\return int32_t 0 for success, -1 otherwise
*/

int32_t IMG3_FileInterface::ParseFile(uint8_t *fileData, uint32_t fileLength)
{
	uint32_t magicNumber, index = 0;
	IMG3_FileSection *newSection;
	list< IMG3_FileSection * >::iterator listIt;
	uint32_t totalImg3Length = 0;
	uint8_t *currPtr = fileData;
	uint32_t currOffset = 0;
	uint8_t foundEnd = 0;

	ASSERT_RET( fileData, -1 );
	ASSERT_RET( fileLength, -1 );

	// We may be analyzing more than one file, so before we create a list for this file, make sure our list is empty.
	for (listIt = sections.begin(); listIt != sections.end(); ++listIt) {
		IMG3_FileSection *sec = *listIt;
		if (sec != NULL)
			delete(sec);
	}
	sections.clear();

	// The first section inside an img3 file should be an IMG3 section.  This will tell us exactly how long the file should
	// be.  If that doesn't match, report an error.
	newSection = new IMG3_FileSection();
	if( newSection->ParseSection( fileData ) != 0 ) {
		errorCode = IMG3_FILE_SECTION_ERROR_INVALID_DATA;
		goto PARSEFILE_PARSE_ERROR;
	}
	
	if( newSection->GetSectionType() != IMG3_BASE ) {
		errorCode = IMG3_FILE_SECTION_ERROR_INVALID_FORMAT;
		goto PARSEFILE_PARSE_ERROR;
	}

	// Now that we know we're looking at a valid img3 file, we record the total size and begin looking for sections.
	totalImg3Length = newSection->GetSectionTotalLength();
	if( totalImg3Length != fileLength ) {
		errorCode = IMG3_FILE_SECTION_ERROR_LENGTHS_DIFFER;
		goto PARSEFILE_PARSE_ERROR;
	}

	// Add the Img3 section to our list.
	sections.push_back( newSection );

	// Now move our location up only past the header.  The initial Img3 header contains the entire file as its data, 
	// so we just start looking for new tags after the header.
	currOffset += newSection->GetSectionHeaderLength();
	currPtr = fileData + currOffset;
	
	while( currOffset < totalImg3Length ) {
		magicNumber = *(uint32_t *)( currPtr );
		switch( magicNumber ) {
		case IMG3_DATA_MAGIC:
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
			// Once we find a tag, create a new FileSection object for it.
			newSection = new IMG3_FileSection();
			if (newSection == NULL) {
				PRINT_SYSTEM_ERROR();
				return -1;
			}
			// Parse that section to determine type and store values and data.
			if (newSection->ParseSection( currPtr ) != 0)
				return -1;
			// Add it to our list and move our pointer.  From this point though, we move the pointer by the size of the data
			// not the header.  If a section contains multiple other sections (like the CERT section), just copy them over
			// as is.
			sections.push_back(newSection);
			currOffset += newSection->GetSectionTotalLength();
			currPtr = fileData + currOffset;
				
			break;
		default:
			// This could probably be changed to increment by four as all tags appear to be four-byte aligned.
			currOffset++;
			currPtr = fileData + currOffset;
			break;
		}
	}

	return 0;

PARSEFILE_PARSE_ERROR:
//	PRINT_PROGRAM_ERROR( fileSection_errorMessage[errorCode] );
	delete newSection;
	return -1;
}

//! IMG3_FileInterface::GetSections function
/*! This function is used to retrieve a pointer to an IMG3_FileSystem object that represents the given section.  Aside from
	KBAG sections, there should never be more than one instance of a given individual section inside of an img3 file.  Some 
	sections may contain multiple other sections, for instance the CERT section.  This section will often have an Img3, CHIP, 
	SDOM, and one other section inside of it, some of which exist in other sections of the file as well.  However, there 
	should only ever be one independent CERT section.  If, for instance, you wanted all CHIP sections, this will only return 
	any independent CHIP sections; it will not return every CHIP section that is contained within every section in the file.
	The return value is a list simply to support the fact that there may be more than one KBAG.

	\param [in] section the IMG3_SectionType of the section whose data is being requested
	\return [out] list<IMG3_FileSection *> a list of pointers to IMG3_FileSection objects that represents the given section type
*/

list<IMG3_FileSection *> IMG3_FileInterface::GetSection( IMG3_SectionType section )
{
	list<IMG3_FileSection *>::iterator listIt;
	list<IMG3_FileSection *> allSections;

	// First make sure the list is empty.
	allSections.clear();

	// Then simply iterate through all available sections and add a pointer to them to the list.
	for ( listIt = sections.begin(); listIt != sections.end(); ++listIt ) {
		IMG3_FileSection *sec = *listIt;
		if ( sec->GetSectionType() == section )
			allSections.push_back( sec );
	}

	return allSections;
}

uint8_t * IMG3_FileInterface::GetSectionData( IMG3_SectionType section )
{
	list<IMG3_FileSection *>::iterator listIt;

	// Then simply iterate through all available sections and add a pointer to them to the list.
	for ( listIt = sections.begin(); listIt != sections.end(); ++listIt ) {
		IMG3_FileSection *sec = *listIt;
		if ( sec->GetSectionType() == section )
			return sec->GetSectionData();
	}

	return NULL;
}

uint32_t IMG3_FileInterface::GetSectionDataLength( IMG3_SectionType section )
{
	list<IMG3_FileSection *>::iterator listIt;

	// Then simply iterate through all available sections and add a pointer to them to the list.
	for ( listIt = sections.begin(); listIt != sections.end(); ++listIt ) {
		IMG3_FileSection *sec = *listIt;
		if ( sec->GetSectionType() == section )
			return sec->GetSectionDataLength();
	}

	return 0;
}

//! IMG3_FileInterface::WriteSectionData function
/*! This function is used to overwrite the contents of the current data section for a provided section type with new data.
	\param [in] section a pointer to the IMG3_FileSection whose data is to be overwritten
	\param [in] data a pointer to the new data that should be written
	\param [in] length the length of the new data
	\return int32_t 0 for success, -1 otherwise
*/

int32_t IMG3_FileInterface::WriteSectionData( IMG3_FileSection *section, uint8_t *data, uint32_t length )
{
	list< IMG3_FileSection * >::iterator listIt;
	IMG3_FileSection *sec = NULL;
	uint8_t *secData;
	uint32_t secLength;
	uint8_t found = 0;

	ASSERT_RET( data, -1 );
	ASSERT_RET( length, -1 );

	for ( listIt = sections.begin(); listIt != sections.end(); ++listIt ) {
		sec = *listIt;
		if ( sec == section ) { 
			found = 1;
			break;
		}
	}

	if ( found == 0 ) {
		fprintf( stderr, "Received a write request for an object that is not in the section list.\n" );
		return -1;
	}

	section->WriteData( data, length );

	return 0;
}
