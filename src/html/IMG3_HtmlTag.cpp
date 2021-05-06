/*
 * HtmlTag.cpp
 *
 *  Created on: Mar 14, 2011
 *      Author: mareno
 */

#include "IMG3_HtmlTag.h"

IMG3_HtmlTag::IMG3_HtmlTag() {
	tag = NULL;
	data = NULL;
	attrs.clear();
	type = UNDEFINED_TAG;
}

IMG3_HtmlTag::~IMG3_HtmlTag() {
	list<pair<char *, char *> >::iterator listIt;

	for (listIt = attrs.begin(); listIt != attrs.end(); ++listIt) {
		pair<char *, char *> attr = *listIt;
		if (attr.first != NULL)
			delete(attr.first);
		if (attr.second != NULL)
			delete(attr.second);
	}
	attrs.clear();
	if (tag != NULL)
		delete(tag);
	if (data != NULL)
		delete(data);
}

IMG3_HtmlTag::IMG3_HtmlTag(char *newTag, uint32_t length, TagType tagType)
{
	ASSERT(newTag);
	ASSERT(length);

	if (length == MAX_TAG_VALUE_LENGTH) {
		length = MAX_TAG_VALUE_LENGTH - 1;
	}
	tag = new char[length + 1];
	if (tag == NULL) {
		PRINT_SYSTEM_ERROR();
		return;
	}
	memcpy(tag,newTag,length);
	tag[length] = '\0';

	type = tagType;
}

#ifdef IMG3_DEBUG

void IMG3_HtmlTag::PrintTag()
{
	list<pair<char *, char *> >::iterator listIt;

	fprintf(stdout,"\tValue: %s.\n",tag);
	if (type == OPENING_TAG) {
		fprintf(stdout,"\tType: Opening.\n");
	} else if (type == CLOSING_TAG) {
		fprintf(stdout,"\tType: Closing.\n");
	} else if (type == COMPLETE_TAG) {
		fprintf(stdout,"\tType: Complete.\n");
	} else {
		fprintf(stdout,"\tType: Unknown.\n");
	}
	for (listIt = attrs.begin(); listIt != attrs.end(); ++listIt) {
		pair<char *, char *> attr = *listIt;
		fprintf(stdout,"\tAttribute Name: %s\n\tAttribute Value: %s\n",attr.first, attr.second);
	}
	if (data != NULL) {
		fprintf(stdout,"\tData: %s\n",data);
	}
	fprintf(stdout,"\n");
}

#endif

int32_t IMG3_HtmlTag::SetValue(char *tagValue, uint32_t length)
{
	ASSERT_RET(tagValue, -1);
	ASSERT_RET(length, -1);

	if (length > MAX_TAG_VALUE_LENGTH) {
		errorCode = IMG3_HTMLTAG_ERROR_TAG_TOO_LONG;
		PRINT_CLASS_ERROR( "the tag value is too long" );
		return -1;
	}
	tag = new char[length + 1];
	if (tag == NULL) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}
	memcpy(tag,tagValue,length);
	tag[length] = '\0';

	tagAddress = tagValue;

	return 0;
}

int32_t IMG3_HtmlTag::SetData(char *tagData, uint32_t length)
{
	char *dataPtr;

	ASSERT_RET(tagData,-1);
	if (length == 0)
		ASSERT_RET(length,-1);

	if (length == MAX_TAG_VALUE_LENGTH) {
		errorCode = IMG3_HTMLTAG_ERROR_DATA_TOO_LONG;
		PRINT_CLASS_ERROR( "the data for the tag is too long" );
		return -1;
	}
	data = new char[length+1];
	if (data == NULL) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	// Okay, before we just assign the data, we need to remove any whitespace before or after it.
	dataPtr = tagData;
	// First remove the trailing whitespace by decrementing the length value.
	while(dataPtr[length-1] == ' ')
		length--;
	// Next, remove the beginning whitespace by incrementing our data pointer.
	while(*dataPtr == ' ') {
		dataPtr++;
		length--;
	}
	memcpy(data,dataPtr,length);
	data[length] = '\0';

	return 0;
}

int32_t IMG3_HtmlTag::SetType(TagType tagType)
{
	if (tagType > UNDEFINED_TAG) {
		errorCode = IMG3_HTMLTAG_ERROR_UNDEFINED_TAG;
		PRINT_CLASS_ERROR( "found invalid tag" );
		return -1;
	}
	type = tagType;
	return 0;
}

int32_t IMG3_HtmlTag::SetAttributes(char *attrStr, uint32_t length)
{
	pair<char *, char *> newAttr;
	char *nameStart, *nameEnd, *valueStart, *valueEnd;
	char *currPtr, *endPtr;
	uint32_t stringLength;
	char temp[MAX_TAG_ATTRS_LENGTH+1];

	ASSERT_RET(attrStr,-1);
	ASSERT_RET(length,-1);

	currPtr = attrStr;
	endPtr = attrStr + length;

	while(currPtr < endPtr) {
		newAttr.first = NULL;
		newAttr.second = NULL;

		// HTML tag attributes take on the following syntax: id="value"
		// Therefore, the first step is to find our equal sign.
		nameEnd = strchr(currPtr,'=');
		// If none exist, we're either at the end or there aren't any.
		if (nameEnd == NULL) {
			break;
		}
		// Now, the name portion should be everything left of the equal sign that is not
		// a space.
		nameStart = nameEnd;
		do {
			nameStart--;
		} while (*nameStart != ' ' && nameStart > currPtr);
		if (nameStart != currPtr)
			nameStart++;
		// If no space exist, our starting pointer was likely pointing at the beginning
		// of the attribute id, so just use that.
		if (nameStart == NULL)
			nameStart = currPtr;

		// The value portion should be surrounded by quotes, so verify that this is true.
		valueStart = strchr(nameEnd,'\"');
		if (valueStart == NULL) {
			length = nameEnd - nameStart;
			memcpy(temp,attrStr,length);
			temp[length] = '\0';
			errorCode = IMG3_HTMLTAG_ERROR_INVALID_ATTRIBUTE;
			PRINT_CLASS_ERROR( "invalid attribute" );
			break;
		}
		valueStart++;
		// nameEnd should still be pointing at the equal sign, so the value would start
		// two places further.  It should then end at the next quote.
		valueEnd = strchr(valueStart,'\"');
		if (valueEnd == NULL) {
			memcpy(temp,attrStr,length);
			temp[length] = '\0';
			errorCode = IMG3_HTMLTAG_ERROR_INVALID_ATTRIBUTE;
			PRINT_CLASS_ERROR( "invalid attribute" );
			break;
		}
		// Now that we've isolated a single attribute, we're can add it to our list and
		// keep looking.
		stringLength = nameEnd - nameStart;
		newAttr.first = new char[stringLength + 1];
		if (newAttr.first == NULL) {
			PRINT_SYSTEM_ERROR();
			break;
		}
		memcpy(newAttr.first,nameStart,stringLength);
		newAttr.first[stringLength] = '\0';
		stringLength = valueEnd - valueStart;
		newAttr.second = new char[stringLength + 1];
		if (newAttr.second == NULL) {
			PRINT_SYSTEM_ERROR();
			delete(newAttr.first);
			break;
		}
		memcpy(newAttr.second,valueStart,stringLength);
		newAttr.second[stringLength] = '\0';
		attrs.push_back(newAttr);

		currPtr = valueEnd + 1;
	}
	return 0;
}

char * IMG3_HtmlTag::DoesTagContainAttribute(char *attr)
{
	list<pair<char *, char *> >::iterator listIt;

	for (listIt = attrs.begin(); listIt != attrs.end(); ++listIt)
	{
		pair<char *, char *> temp = *listIt;
		if (strcmp(temp.first,attr) == 0)
			return temp.second;
	}
	return NULL;
}
