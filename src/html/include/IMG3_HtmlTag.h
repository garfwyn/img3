/*
 * HtmlTag.h
 *
 *  Created on: Mar 14, 2011
 *      Author: mareno
 */

#ifndef HTMLTAG_H_
#define HTMLTAG_H_

#include <list>
#include <stdint.h>
#include "IMG3_defines.h"

using namespace std;

// Maximum length for an entire tag
#define MAX_TAG_LENGTH			1028
// Maximum length for just the value portion of the tag
#define MAX_TAG_VALUE_LENGTH	128
// Maximum length for all attribute values combined
#define MAX_TAG_ATTRS_LENGTH	900
// Maximum length for any single attribute
#define MAX_TAG_ATTR_LENGTH		256
#define MAX_TAG_DATA_LENGTH		1024*1024

#define IMG3_HTMLTAG_ERROR_NONE					0x0000
#define IMG3_HTMLTAG_ERROR_TAG_TOO_LONG			0x0001
#define IMG3_HTMLTAG_ERROR_DATA_TOO_LONG		0x0002
#define IMG3_HTMLTAG_ERROR_UNDEFINED_TAG		0x0003
#define IMG3_HTMLTAG_ERROR_INVALID_ATTRIBUTE	0x0004

typedef enum TagType {
	OPENING_TAG,
	CLOSING_TAG,
	COMPLETE_TAG,
	UNDEFINED_TAG
} TagType;

class IMG3_HtmlTag {
private:
	char *tag;
	list< pair< char*, char* > > attrs;
	char *data;
	TagType	type;

	char *tagAddress;
	char *dataAddress;

	int32_t errorCode;

public:
	IMG3_HtmlTag();
	IMG3_HtmlTag(char *newTag, uint32_t length, TagType tagType = UNDEFINED_TAG);
	virtual ~IMG3_HtmlTag();

	int32_t SetValue(char *value, uint32_t length);
	int32_t SetData(char *data, uint32_t length);
	int32_t	SetType(TagType tagType);
	int32_t SetAttributes(char *attrs, uint32_t length);

	list<pair<char*,char*> > * GetAttributes() { return &attrs; }
	char *GetTagValue() { return tag; }
	char *GetTagData() { return data; }
	TagType GetTagType() { return type; }
	char * GetTagAddress() { return tagAddress; }
	char * DoesTagContainAttribute(char *attr);

#ifdef IMG3_DEBUG
	void PrintTag();
#endif
};

#endif /* HTMLTAG_H_ */
