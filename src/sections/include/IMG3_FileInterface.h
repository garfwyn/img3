/*! \file 		IMG3_FileInterface.h
 	\author 	Matthew Areno
	\date 		March 21, 2011
    \version 	1.0

	This class is used to provide an interface to the FileSection class that maintain data about each section found inside 
	an img3 file.
*/

#ifndef IMG3_FILEINTERFACE_H_
#define IMG3_FILEINTERFACE_H_

#include <list>
#include "IMG3_FileSection.h"

using namespace std;

//! IMG3_FileInterface class
/*!
	The FileInterface class provides the mechanisms for analyzing img3 files and 
	maintaining structure information.  All sections found in an img3 file are stored
	in a dynamic list.  These sections can then be accessed through the public functions
	provided by this class.
*/

class IMG3_FileInterface {
private:
	//! Private list variable
	/*! This variable maintains a list of all sections found within an img3 file.
	*/
	list<IMG3_FileSection *> sections;

	uint32_t errorCode;

public:
	//! IMG3_FileInterface constructor
	IMG3_FileInterface();
	//! IMG3_FileInterface destructor
	virtual ~IMG3_FileInterface();

	//! ParseFile public function
	/*! This function parses a block of data and populates the sections list with all
		sections contained within the block.
		\param fileData a pointer to the block of data representing an img3 file.
		\param fileLength the length of the block of data
		\return int32_t 0 for success, -1 otherwise
	*/
	int32_t ParseFile(uint8_t *fileData, uint32_t fileLength);
	
	//! GetSections public function
	/*! This function returns a list of all sections that match the provided section identifier
		\param section the type of section being requested
		\return list<IMG3_FileSection *>
	*/
	list<IMG3_FileSection *> GetSection( IMG3_SectionType section );


	uint8_t * GetSectionData( IMG3_SectionType section );

	uint32_t GetSectionDataLength( IMG3_SectionType section );

	//! WriteSectionData public function
	/*! This function can be used to overwrite the current contents of the provided sections data
		portion.a
		\return int32_t 0 for success, -1 otherwise
	*/
	int32_t WriteSectionData( IMG3_FileSection *section, uint8_t *data, uint32_t length );
	
	//! PrintSection public function.
	/*!	This function is just for debugging purposes and prints out all sections that
		were found via the ParseFile function.
	*/
	void PrintSections();
};

#endif /* IMG3_FILEINTERFACE_H_ */
