/*
 * HtmlParser.h
 *
 *  Created on: Mar 14, 2011
 *      Author: mareno
 */

#ifndef HTMLPARSER_H_
#define HTMLPARSER_H_

#include <list>
#include <stdint.h>
#include <string.h>
#include "IMG3_HtmlTag.h"
#include "IMG3_defines.h"
#include "IMG3_typedefs.h"

#define IMG3_HTMLPARSER_ERROR_FILTER_TOO_LONG			0x0001
#define IMG3_HTMLPARSER_ERROR_NO_CLOSING_TAG			0x0002
#define IMG3_HTMLPARSER_ERROR_TAG_TOO_LONG				0x0003
#define IMG3_HTMLPARSER_ERROR_NO_OPENING_TAG			0x0004
#define IMG3_HTMLPARSER_ERROR_INVALID_TAG_SET			0x0005
#define IMG3_HTMLPARSER_ERROR_DATA_TOO_LONG				0x0006
#define IMG3_HTMLPARSER_ERROR_FAILED_TO_RETRIEVE_TAGS	0x0007
#define IMG3_HTMLPARSER_ERROR_NO_IV_FOUND				0x0008
#define IMG3_HTMLPARSER_ERROR_NO_KEY_FOUND				0x0009

typedef enum ParseState {
	SCANNING_FOR_OPENING_TAG_START,
	SCANNING_FOR_OPENING_TAG_END,
	SCANNING_FOR_CLOSING_TAG_START,
	SCANNING_FOR_CLOSING_TAG_END,
	SCANNING_FOR_TAG_END,
	PARSING_TAG_ATTRIBUTES,
	PARSING_TAG_VALUE
} ParseState;

typedef struct HtmlTagNode {
	IMG3_HtmlTag	tag;
	uint32_t		id;
} HtmlTagNode;

class IMG3_HtmlParser {
private:
	int32_t errorCode;

	list<HtmlTagNode *> openingTags;
	list<HtmlTagNode *> closingTags;
	list<pair<HtmlTagNode *,HtmlTagNode *> > completeTags;
	list<pair<HtmlTagNode *,HtmlTagNode *> > sortedTags;
	uint32_t	numberOfTags;

	list<char *> tagFilters;

	list<DecryptionInfo *> decryptionList;

	void AddOpeningTag(char *, uint32_t);
	void AddClosingTag(char *, uint32_t);
	void RemoveOpeningTag(uint32_t tagId);
	void RemoveClosingTag(uint32_t tagId);

	void RetrieveDataBetweenTags();
	void ExtractDecryptionInfo();

	void SortTagsById();
	bool TagIsOfInterest(char *tag);

	int32_t CombineTags();

public:
	IMG3_HtmlParser();
	virtual ~IMG3_HtmlParser();

	int32_t	AddFilter(char *filter);

	list<HtmlTagNode *> *GetAllTagsByName(char *);
	list<HtmlTagNode *> *GetAllOpeningTagsByName(char *);
	list<HtmlTagNode *> *GetAllClosingTagsByName(char *);
	list<pair<HtmlTagNode *,HtmlTagNode *> > *GetAllSortedTagsByName(char *tagName);
	HtmlTagNode *GetNextTag(uint32_t currentTagId);
	list<DecryptionInfo *> *GetDecryptionInfo() { return &decryptionList; }

	bool ParseHtmlTags(uint8_t *buffer, uint32_t bufferLength);
};

#endif /* HTMLPARSER_H_ */
