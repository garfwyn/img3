#ifndef __IMG3_GEN_TYPEDEFS_H_
#define __IMG3_GEN_TYPEDEFS_H_

typedef struct DecryptionInfo {
	char *section;
	char *iv;
	char *key;
} DecryptionInfo;

typedef enum ParserOperation {
	DECRYPT_IMG3_FILE		= 0x1, 
	UPDATE_IMG3_DATABASE	= 0x2, 
	LIST_ARCHIVE_FILES		= 0x3, 
	EXTRACT_FILE			= 0x4, 
	PATCH_KERNEL			= 0x5,
	PARSE_FILE				= 0x6,
	DECOMPRESS_FILE			= 0x7,
} ParserOperation;

#endif //__IMG3_GEN_TYPEDEFS_H_
