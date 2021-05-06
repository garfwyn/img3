/*
 * HtmlParser.cpp
 *
 *  Created on: Mar 14, 2011
 *      Author: mareno
 */

#include "IMG3_HtmlParser.h"
#include <ctype.h>

IMG3_HtmlParser::IMG3_HtmlParser()
{
	// TODO Auto-generated constructor stub
	openingTags.clear();
	closingTags.clear();
	completeTags.clear();
	numberOfTags = 0;
	errorCode = IMG3_ERROR_NONE;
}

IMG3_HtmlParser::~IMG3_HtmlParser()
{
	list<pair<HtmlTagNode *,HtmlTagNode *> >::iterator sortedTagsIt;
	list<char *>::iterator tagFiltersIt;
	list<DecryptionInfo *>::iterator decryptionInfoIt;

	openingTags.clear();
	closingTags.clear();
	completeTags.clear();

	for (sortedTagsIt = sortedTags.begin(); sortedTagsIt != sortedTags.end(); ++sortedTagsIt) {
		pair<HtmlTagNode *, HtmlTagNode *> nodes = *sortedTagsIt;
		if (nodes.first != NULL)
			delete(nodes.first);
		if (nodes.second != NULL)
			delete(nodes.second);
	}
	sortedTags.clear();

	for (tagFiltersIt = tagFilters.begin(); tagFiltersIt != tagFilters.end(); ++tagFiltersIt) {
		char *filter = *tagFiltersIt;
		if (filter != NULL)
			delete(filter);
	}
	tagFilters.clear();

	for (decryptionInfoIt = decryptionList.begin(); decryptionInfoIt != decryptionList.end(); ++decryptionInfoIt) {
		DecryptionInfo *info = *decryptionInfoIt;
		if (info->section != NULL)
			delete(info->section);
		if (info->iv != NULL)
			delete(info->iv);
		if (info->key != NULL)
			delete(info->key);
	}
	tagFilters.clear();
}

bool IMG3_HtmlParser::TagIsOfInterest(char *tag)
{
	list<char *>::iterator listIt;

	for (listIt = tagFilters.begin(); listIt != tagFilters.end(); listIt++) {
		if (strcmp(*listIt,tag) == 0)
			return true;
	}
	return false;
}

int32_t IMG3_HtmlParser::AddFilter(char *filter)
{
	char *newFilter;
	uint32_t length;

	ASSERT_RET(filter,-1);
	length = strlen(filter);
	if (length >= MAX_TAG_VALUE_LENGTH) {
		errorCode = IMG3_HTMLPARSER_ERROR_FILTER_TOO_LONG;
		PRINT_CLASS_ERROR( "filter to long" );
		return -1;
	}
	newFilter = new char[length+1];
	if (newFilter == NULL) {
		PRINT_SYSTEM_ERROR();
		ASSERT_RET(newFilter,-1);
	}
	strncpy(newFilter,filter,length);
	tagFilters.push_back(newFilter);

	return 0;
}

list<HtmlTagNode *> * IMG3_HtmlParser::GetAllTagsByName(char *tagName)
{
	list<pair<HtmlTagNode *, HtmlTagNode *> >::iterator listIt;
	list<HtmlTagNode *> *allTags;

	allTags = new list<HtmlTagNode *>;
	if (!allTags) {
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	for (listIt = completeTags.begin(); listIt != completeTags.end(); ++listIt) {
		HtmlTagNode *node = listIt->first;
		if (strcmp(node->tag.GetTagValue(),tagName) == 0) {
			allTags->push_back(node);
		}
	}
	return allTags;
}

list<HtmlTagNode *> * IMG3_HtmlParser::GetAllOpeningTagsByName(char *tagName)
{
	list<HtmlTagNode *>::iterator listIt;
	list<HtmlTagNode *> *allTags;

	allTags = new list<HtmlTagNode *>;
	if (!allTags) {
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	for (listIt = openingTags.begin(); listIt != openingTags.end(); ++listIt) {
		HtmlTagNode *node = *listIt;
		if (strcmp(node->tag.GetTagValue(),tagName) == 0) {
			allTags->push_back(node);
		}
	}
	return allTags;
}

list<HtmlTagNode *> * IMG3_HtmlParser::GetAllClosingTagsByName(char *tagName)
{
	list<HtmlTagNode *>::iterator listIt;
	list<HtmlTagNode *> *allTags;

	allTags = new list<HtmlTagNode *>;
	if (!allTags) {
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	for (listIt = closingTags.begin(); listIt != closingTags.end(); ++listIt) {
		HtmlTagNode *node = *listIt;
		if (strcmp(node->tag.GetTagValue(),tagName) == 0) {
			allTags->push_back(node);
		}
	}
	return allTags;
}

list<pair<HtmlTagNode *, HtmlTagNode *> > * IMG3_HtmlParser::GetAllSortedTagsByName(char *tagName)
{
	list<pair<HtmlTagNode *, HtmlTagNode *> >::iterator listIt;
	list<pair<HtmlTagNode *, HtmlTagNode *> > *allTags;

	allTags = new list<pair<HtmlTagNode *, HtmlTagNode *> >;
	if (!allTags) {
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	for (listIt = sortedTags.begin(); listIt != sortedTags.end(); ++listIt) {
		pair<HtmlTagNode *, HtmlTagNode *> nodes = *listIt;
		if (strcmp(nodes.first->tag.GetTagValue(),tagName) == 0) {
			allTags->push_back(nodes);
		}
	}
	return allTags;

}

HtmlTagNode * IMG3_HtmlParser::GetNextTag(uint32_t currentTagId)
{
	list<pair<HtmlTagNode *, HtmlTagNode *> >::iterator listIt;

	for (listIt = completeTags.begin(); listIt != completeTags.end(); ++listIt) {
		HtmlTagNode *node = listIt->first;
		if (node->id == currentTagId) {
			++listIt;
			if (listIt != completeTags.end()) {
				return listIt->first;
			}
		}
	}
	return NULL;
}

void IMG3_HtmlParser::AddOpeningTag(char *buffer, uint32_t bufferLength)
{
	char *openingTagStart, *openingTagEnd;
	char *tagValueStart, *tagValueEnd;
	char *tagAttrsStart, *tagAttrsEnd;
	char tempValue[MAX_TAG_LENGTH];
	uint32_t tagLength;
	HtmlTagNode *node;

	// HTML tags will all start with the '<' character and take on the following syntax:
	//	<[Tag Value] [Tag Attributes(optional)] [/(optional)]>
	// If the tag is closed with a "/>" rather than '>', that means there is no associated
	// closing tag.  So this function should look for opening tags, parse their value and
	// attributes, and then determine if the tag should be closed, or if there should be
	// an associated closing tag.
	openingTagStart = buffer;
	openingTagEnd = buffer + bufferLength - 1;

	tagValueStart = openingTagStart + 1;
	tagValueEnd = strchr(tagValueStart,' ');
	if (tagValueEnd == NULL || tagValueEnd > openingTagEnd) {
		// If there is no space after the tag, it has no attributes.
		tagValueEnd = openingTagEnd;
		tagLength = tagValueEnd - tagValueStart;
		tagAttrsStart = NULL;
		tagAttrsEnd = NULL;
	} else {
		tagAttrsStart = tagValueEnd + 1;
	}

	tagLength = tagValueEnd - tagValueStart;
	memcpy(tempValue,tagValueStart,tagLength);
	tempValue[tagLength] = '\0';
	if (!TagIsOfInterest(tempValue)) {
		return;
	}
	node = new HtmlTagNode();

	tagLength = tagValueEnd - tagValueStart;
	node->tag.SetValue(tagValueStart,tagLength);

	if (tagAttrsStart != NULL) {
		// The attributes end at the closing '>', regardless of whether this is an opening tag
		// closing marker, or a complete tag closing marker.
		tagAttrsEnd = strchr(tagAttrsStart,'>');
		tagLength = tagAttrsEnd - tagAttrsStart;
		node->tag.SetAttributes(tagAttrsStart,tagLength);
	}

	if (*(buffer+bufferLength-2) == '/') {
		node->tag.SetType(COMPLETE_TAG);
		node->id = numberOfTags++;
		completeTags.push_back(pair<HtmlTagNode *, HtmlTagNode *>(node,node));
	} else {
		node->tag.SetType(OPENING_TAG);
		node->id = numberOfTags++;
		openingTags.push_back(node);
	}
	return;
}

void IMG3_HtmlParser::AddClosingTag(char *buffer, uint32_t bufferLength)
{
	char *closingTagStart, *closingTagEnd;
	char tagValue[MAX_TAG_VALUE_LENGTH+1];
	uint32_t length;
	HtmlTagNode *node;

	closingTagStart = buffer+2;
	closingTagEnd = strchr(closingTagStart,'>');
	if (closingTagEnd == NULL) {
		errorCode = IMG3_HTMLPARSER_ERROR_NO_CLOSING_TAG;
		PRINT_CLASS_ERROR( "no closing tag" );
		return;
	}
	length = closingTagEnd - closingTagStart;
	if (length > MAX_TAG_VALUE_LENGTH) {
		errorCode = IMG3_HTMLPARSER_ERROR_TAG_TOO_LONG;
		PRINT_CLASS_ERROR( "tag too long" );
		return;
	}

	memcpy(tagValue,closingTagStart,length);
	tagValue[length] = '\0';
	if (!TagIsOfInterest(tagValue))
		return;
	node = new HtmlTagNode();
	node->tag.SetValue(closingTagStart,length);
	node->tag.SetType(CLOSING_TAG);
	node->id = numberOfTags++;
	closingTags.push_back(node);
	//fprintf(stdout,"Found closing tag:\n");
	//node->tag.PrintTag();
}

void IMG3_HtmlParser::RemoveOpeningTag(uint32_t tagId)
{
	list<HtmlTagNode *>::iterator listIt;

	for (listIt = openingTags.begin(); listIt != openingTags.end(); ++listIt) {
		HtmlTagNode *node = *listIt;
		if (node->id == tagId) {
			openingTags.erase(listIt);
			return;
		}
	}
}

void IMG3_HtmlParser::RemoveClosingTag(uint32_t tagId)
{
	list<HtmlTagNode *>::iterator listIt;

	for (listIt = closingTags.begin(); listIt != closingTags.end(); ++listIt) {
		HtmlTagNode *node = *listIt;
		if (node->id == tagId) {
			closingTags.erase(listIt);
			return;
		}
	}
}

int32_t IMG3_HtmlParser::CombineTags()
{
	list<HtmlTagNode *>::iterator openingListIt, closingListIt, temp;
	list<HtmlTagNode *> *openingTagsOfType, *closingTagsOfType;
	HtmlTagNode *node;

	if (openingTags.size() == 0) {
		return 0;
	}
	// First step is to determine what the tag value is we're looking for.
	node = closingTags.front();
	// Next, we need to get all opening and closing tags with that value.
	closingTagsOfType = GetAllClosingTagsByName(node->tag.GetTagValue());
	if (closingTagsOfType == NULL) {
		return -1;
	}
	// Then, starting with our closing tags because they come last, we're going to look for
	// the last opening tag who's ID is greater than ours.  That should be our match.
	for (closingListIt = closingTagsOfType->begin(); closingListIt != closingTagsOfType->end(); ++closingListIt) {
		HtmlTagNode *closingTag = *closingListIt;
		HtmlTagNode *prevOpeningTag, *currOpeningTag;
		openingTagsOfType = GetAllOpeningTagsByName(node->tag.GetTagValue());
		if (openingTagsOfType == NULL) {
			errorCode = IMG3_HTMLPARSER_ERROR_NO_OPENING_TAG;
			PRINT_CLASS_ERROR( "no opening tag" );
			return -1;
		}
		currOpeningTag = prevOpeningTag = NULL;
		for (openingListIt = openingTagsOfType->begin(); openingListIt != openingTagsOfType->end(); ++openingListIt) {
			currOpeningTag = *openingListIt;
			if (currOpeningTag->id > closingTag->id) {
				if (prevOpeningTag == NULL) {
					errorCode = IMG3_HTMLPARSER_ERROR_INVALID_TAG_SET;
					PRINT_CLASS_ERROR( "invalid tag combination found" );
					break;
				}
				completeTags.push_back(pair<HtmlTagNode *, HtmlTagNode *>(prevOpeningTag,closingTag));
				RemoveOpeningTag(prevOpeningTag->id);
				RemoveClosingTag(closingTag->id);
				break;
			} else if (openingTagsOfType->size() == 1) {
				completeTags.push_back(pair<HtmlTagNode *, HtmlTagNode *>(currOpeningTag,closingTag));
				RemoveOpeningTag(currOpeningTag->id);
				RemoveClosingTag(closingTag->id);
				break;
			}
			prevOpeningTag = currOpeningTag;
		}
		delete(openingTagsOfType);
	}
	delete(closingTagsOfType);

	if (openingTags.size() == 0) {
		return 0;
	}
	return 1;
}

void IMG3_HtmlParser::RetrieveDataBetweenTags()
{
	list<pair<HtmlTagNode *, HtmlTagNode *> >::iterator listIt;
	HtmlTagNode *opening, *closing;
	uint32_t dataSize;
	char *ptr;

	for (listIt = completeTags.begin(); listIt != completeTags.end(); ++listIt) {
		opening = listIt->first;
		closing = listIt->second;

		// Find the end of our opening tag.
		ptr = strchr(opening->tag.GetTagAddress()+1,'>');
		if (ptr == NULL) {
			errorCode = IMG3_HTMLPARSER_ERROR_NO_CLOSING_TAG;
			PRINT_CLASS_ERROR( "no closing tag found");
			return;
		}
		// Move past the '>' character
		ptr++;
		// The length of our data is from our current position to the start of the closing tag, minus 2
		// because our closing tag points to the tag value, which is preceeded by the "</" characters.
		dataSize = closing->tag.GetTagAddress() - ptr - 2;
		if (dataSize > MAX_TAG_DATA_LENGTH) {
			errorCode = IMG3_HTMLPARSER_ERROR_DATA_TOO_LONG;
			PRINT_CLASS_ERROR( "data too long" );
			return;
		}
		if (dataSize > 0) 
			opening->tag.SetData(ptr, dataSize);
	}
}

void IMG3_HtmlParser::SortTagsById()
{
	list<pair<HtmlTagNode *, HtmlTagNode *> >::iterator listIt, currMin;
	uint32_t minTagId;
	uint32_t listSize;

	// For this, we're just going to perform a standard bubble sort.  No need for anything fancy here.
	// To ensure we don't screw up the list by deleted crap, we're going to just loop based off the size
	// of the completeTags list.  As we find the current min, we take it from the completeTags list
	// and add it to the end of the sortedTags list.
	listSize = completeTags.size();


	while(sortedTags.size() != listSize) {
		pair<HtmlTagNode *, HtmlTagNode *> currNodes = completeTags.front();
		minTagId = currNodes.first->id;
		currMin = completeTags.begin();
		for (listIt = completeTags.begin(); listIt != completeTags.end(); ++listIt) {
			currNodes = *listIt;
			if (currNodes.first->id < minTagId) {
				minTagId = currNodes.first->id;
				currMin = listIt;
			}
		}
		currNodes = *currMin;
		sortedTags.push_back(currNodes);
		completeTags.erase(currMin);
	}
}

void IMG3_HtmlParser::ExtractDecryptionInfo()
{
	list<pair<HtmlTagNode *,HtmlTagNode *> > *tagList;
	list<pair<HtmlTagNode *,HtmlTagNode *> >::iterator matchingListIt, sortedListIt;
	char *dataPtr, *endMarker;
	char openingTag[] = "ul";
	DecryptionInfo *info;
	uint32_t length, index;

	// Okay, see each decryption entry has the following structure:
	//
	// <span>[Area Name]</span><ul><li> <b>IV</b>: [IV value] </li><li> <b>Key</b>: [Key value] </li></ul>
	//
	// Therefore, to find everything I need, I'll grab all of the <ul> tags.  From there I can bactrace to the
	// previous <span> tag and its data should be the area name.  I can then grab subsequent IV from the ul data.
	// I then scan for the key as well and I'm all done.

	tagList = GetAllSortedTagsByName(openingTag);
	if (tagList == NULL) {
		errorCode = IMG3_HTMLPARSER_ERROR_FAILED_TO_RETRIEVE_TAGS;
		PRINT_CLASS_ERROR( "uUnable to retrive list of all sorted <ul> tags" );
		return;
	}
	for (matchingListIt = tagList->begin(); matchingListIt != tagList->end(); ++matchingListIt) {
		HtmlTagNode *node = matchingListIt->first;
		// First step is to check and see if the <ul> tag I have contains decryption information.
		dataPtr = strstr(node->tag.GetTagData(),"<b>IV</b>");
		if (dataPtr != NULL) {
			HtmlTagNode *prevNode;
			// Now that we've identified a decryption <ul> tag, we need to find the previous tag.
			// It should be a <span> tag, though some entries have a html link after the <span> tag,
			// and the identifier is there instead.  Either way, it should be the tag immediately before.
			for (sortedListIt = sortedTags.begin(); sortedListIt != sortedTags.end(); ++sortedListIt) {
				prevNode = sortedListIt->first;
				if (prevNode->id == node->id) {
					prevNode = (--sortedListIt)->first;
					break;
				}
			}
			// Now that we have our <span> tag, we can get the name of this info
			info = new DecryptionInfo;
			if (info == NULL) {
				PRINT_SYSTEM_ERROR();
				ASSERT(false);
			}

			length = strlen(prevNode->tag.GetTagData());
			info->section = new char[length+1];
			info->section[length] = '\0';
			if (info->section == NULL) {
				PRINT_SYSTEM_ERROR();
				ASSERT(false);
			}
			strncpy(info->section,prevNode->tag.GetTagData(),length);
			// For simplicity sake, convert all section names to lower case.
			for (index = 0; index < length; index++) {
				info->section[index] = tolower(info->section[index]);
			}

			// Next step is to get the IV.  dataPtr still points at our <b> tag, so there should be a colon after the </b> tag and
			// the actual IV.  Same is true for the Key.
			dataPtr = strchr(dataPtr,':');
			if (dataPtr == NULL) {
				errorCode = IMG3_HTMLPARSER_ERROR_NO_IV_FOUND;
				PRINT_CLASS_ERROR( "couldn't find the IV");
				return;
			}
			// Move the pointer past the ':' and passed any spaces in between.
			dataPtr++;
			while (*dataPtr == ' ') {
				dataPtr++;
			}

			// Newer site has added a collection of <code> tags around the actual data.  If they exist, skip over them.
			if( *dataPtr == '<' ) {
				dataPtr = strchr( dataPtr, '>' );
				dataPtr++;
				endMarker = strchr( dataPtr, '<' );
			} else {
				endMarker = strchr(dataPtr,0x0a);
			}
			if (endMarker == NULL) {
				errorCode = IMG3_HTMLPARSER_ERROR_NO_IV_FOUND;
				PRINT_CLASS_ERROR( "couldn't find the IV");
				return;
			}
			length = endMarker - dataPtr;
			info->iv = new char[length+1];
			memcpy(info->iv,dataPtr,length);
			info->iv[length] = '\0';

			dataPtr = strchr(dataPtr,':');
			if (dataPtr == NULL) {
				errorCode = IMG3_HTMLPARSER_ERROR_NO_KEY_FOUND;
				PRINT_CLASS_ERROR( "couldn't find the key");
				return;
			}

			dataPtr++;
			while(*dataPtr == ' ') {
				dataPtr++;
			}

			// Again, check for <code> tags around data.
			if( *dataPtr == '<' ) {
				dataPtr = strchr( dataPtr, '>' );
				dataPtr++;
				endMarker = strchr( dataPtr, '<' );
			} else {
				endMarker = strchr(dataPtr,0x0a);
			}
			if (endMarker == NULL) {
				errorCode = IMG3_HTMLPARSER_ERROR_NO_KEY_FOUND;
				PRINT_CLASS_ERROR( "couldn't find the key");
				return;
			}
			length = endMarker - dataPtr;
			info->key = new char[length+1];
			memcpy(info->key,dataPtr,length);
			info->key[length] = '\0';

			decryptionList.push_back(info);
		}
	}
}

bool IMG3_HtmlParser::ParseHtmlTags(uint8_t *buffer, uint32_t bufferLength)
{
	char *tagStart, *tagEnd;
	char *currPtr, *endPtr;
	uint32_t length;
	int32_t result;

	currPtr = (char *)buffer;
	endPtr = (char *)buffer + bufferLength;

	while (currPtr < endPtr) {
		tagStart = strchr(currPtr,'<');
		// If we don't find a '<' character, we're should be at the end.
		if (tagStart == NULL) {
			break;
		}
		// Next, find the end of the tag.
		tagEnd = strchr(tagStart,'>');
		if (tagEnd == NULL) {
			errorCode = IMG3_HTMLPARSER_ERROR_NO_CLOSING_TAG;
			PRINT_CLASS_ERROR( "no closing tag" );
			break;
		}
		// Now, we just need to know if it's an opening or closing tag.
		length = tagEnd - tagStart + 1;

		// Make sure we're not looking at a comment
		if (tagStart[1] == '!' && tagStart[2] == '-' && tagStart[3] == '-') {
			currPtr += length;
			continue;
		}
		if (length > MAX_TAG_LENGTH) {
			errorCode = IMG3_HTMLPARSER_ERROR_TAG_TOO_LONG;
			PRINT_CLASS_ERROR( "tag to long" );
			break;
		}

		if (*(tagStart+1) == '/') {
			AddClosingTag(tagStart,length);
		} else {
			AddOpeningTag(tagStart,length);
		}

		currPtr = tagEnd;
	}
#ifdef IMG3_DEBUG
	fprintf(stdout,"Found %i opening tags, %i closing tags, and %i complete tags.\n",(int)openingTags.size(),(int)closingTags.size(),(int)completeTags.size());
#endif
	// Now, we need to verify that we have the same number of opening and closing tags.  Otherwise,
	// our HTML data is malformed and something is missing.  If that's the case, we just abandon
	// hope.  This isn't a perfect tool!
	if (openingTags.size() != closingTags.size()) {
#ifdef IMG3_DEBUG
		fprintf(stderr,"The number of opening tags and closing tags should be the same.\n");
#endif
	}
	result = 1;
	while (result > 0) {
		result = CombineTags();
	}
	if (result == 0) {
#ifdef IMG3_DEBUG
		fprintf(stdout,"Found matches for all tags.\n");
#endif
		RetrieveDataBetweenTags();
#ifdef IMG3_DEBUG
		fprintf(stdout,"Extracted all data values.  Sorting tags...\n");
#endif
		SortTagsById();
#ifdef IMG3_DEBUG
		fprintf(stdout,"Sorting completed.  Ready for examination.\n");
#endif
		ExtractDecryptionInfo();
#ifdef IMG3_DEBUG
		fprintf(stdout,"Examination completed.\n");
#endif
	} else {
#ifdef IMG3_DEBUG
		fprintf(stderr,"Error while searching for matches.\n");
#endif
		return false;
	}
	return true;
}
