#include <stdio.h>
#include "IMG3_Sqlite3.h"
#include "IMG3_defines.h"

/***************************************************************************************
 *  This class is used to interact with the SQLite3 database that is used to maintain
 *  information regarding the iPhone/iPad/iPod devices and their respective keys.  A
 *  separate table is maintained for each device.  Within those tables are entries
 *  that contain the following information:
 *
 *  | Firmware Version | Firmware Area | IV | Key |
 *
 *  Functions will be provided to (re-)create the database, retrieve information from
 *  the database, and to store information in the database.  Not all database entries 
 *  will be valid as not all information is known.  Some IV/Key combos are not publicly
 *  known at the current time, or perhaps only one or the other is known.
 ***************************************************************************************/

IMG3_Sqlite3::IMG3_Sqlite3()
{
	db = NULL;
}

IMG3_Sqlite3::~IMG3_Sqlite3()
{
	DisconnectFromDatabase();
}

int32_t IMG3_Sqlite3::UpdateDatabase(char *device, char *build, char *version, list<DecryptionInfo *> *infoList)
{
	list<DecryptionInfo *>::iterator listIt;

	if (db == NULL) {
		fprintf(stdout, "Attempting to connect to database...\n");
		if(ConnectToDatabase())
			return -1;
	}

	for (listIt = infoList->begin(); listIt != infoList->end(); ++listIt) {
		DecryptionInfo *info = *listIt;
		if (InsertDecryptionInfo(device, build, version, info->section, info->key, info->iv) != 0) {
			fprintf(stderr,"%s: failed to insert information for %s, version %s.\n", __FUNCTION__, device, version);
			return -1;
		}
	}
	fprintf(stdout,"Successfully updated all information for %s, version %s.\n", device, version);
	return 0;
}

int32_t IMG3_Sqlite3::ConnectToDatabase(void)
{
	int result;

	result = sqlite3_open_v2(
			DATABASE_FILE,
			&db,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
			NULL);
	if (result != SQLITE_OK) {
		fprintf(stderr,"Error opening database: %s.\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		db = NULL;
		return -1;
	}
	// If everything went according to plan, return success.
	return 0;
}

void IMG3_Sqlite3::DisconnectFromDatabase(void)
{
	if (db != NULL)
		sqlite3_close(db);
}

int32_t IMG3_Sqlite3::DoesSectionExist(char *device, char *version, char *section)
{
	sqlite3_stmt *stmt;
	char commandTemplate[] = "SELECT * FROM '%s' WHERE Version='%s' AND Section='%s'";
	char selectTableCommand[MAX_SQL_COMMAND_LENGTH+1];
	uint32_t totalLengthOfString;
	int result;

	ASSERT_RET(device,-1);
	ASSERT_RET(version,-1);

	totalLengthOfString = strlen(device) + strlen(commandTemplate) + strlen(version);
	if (totalLengthOfString > MAX_SQL_COMMAND_LENGTH) {
		fprintf(stderr,"%s: sql command too long.\n",__FUNCTION__);
		return -1;
	}

	sprintf(selectTableCommand,commandTemplate,device,version,section);

	result = sqlite3_prepare_v2(db, selectTableCommand, strlen(selectTableCommand) + 1, &stmt, NULL);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s\n", __FUNCTION__, selectTableCommand, sqlite3_errmsg(db));
		return -1;
	}

	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return 0;
	} else if (result == SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return -1;
	} else {
		return -1;
	}
}

int32_t IMG3_Sqlite3::DoesDeviceExist(char *device)
{
	sqlite3_stmt *stmt;
	char commandTemplate[] = "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'";
	char selectTableCommand[MAX_SQL_COMMAND_LENGTH+1];
	uint32_t totalLengthOfString;
	int result;

	ASSERT_RET(device,-1);

	// To determine if a table exist for the specified device, we simply need to try and grab a row.
	totalLengthOfString = strlen(device) + strlen(commandTemplate);
	if (totalLengthOfString > MAX_SQL_COMMAND_LENGTH) {
		fprintf(stderr,"%s: sql command too long.\n",__FUNCTION__);
		return -1;
	}

	sprintf(selectTableCommand,commandTemplate,device);

	result = sqlite3_prepare_v2(db, selectTableCommand, strlen(selectTableCommand) + 1, &stmt, NULL);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s\n",selectTableCommand,__FUNCTION__,sqlite3_errmsg(db));
		return -1;
	}

	// This command should return each row of the specified table if it exist.
	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return 0;
	} else if (result == SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return -1;
	} else {
		return -1;
	}
}

int32_t IMG3_Sqlite3::InsertTable(char *device)
{
	char insertTableTemplate[] = "CREATE TABLE '%s' (Build char (50), Version char (50) , Section char (100), IV char(128), Key char(256))";
	char insertTableCommand[MAX_SQL_COMMAND_LENGTH+1];
	uint32_t totalLengthOfString;
	int result;
	char *errmsg;

	ASSERT_RET(device,-1);

	totalLengthOfString = strlen(device) + strlen(insertTableTemplate);
	if (totalLengthOfString > MAX_SQL_COMMAND_LENGTH) {
		fprintf(stderr,"%s: sql command too long.\n",__FUNCTION__);
		return -1;
	}

	sprintf(insertTableCommand,insertTableTemplate,device);

	result = sqlite3_exec(
						db,
						insertTableCommand,
						NULL,
						NULL,
						&errmsg);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s.\n",__FUNCTION__,insertTableCommand,errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	return 0;
}

int32_t IMG3_Sqlite3::InsertDecryptionInfo(char *device, char * build, char *version, char *section, char *key, char *iv)
{
	char insertCommandTemplate[] = "INSERT INTO '%s' (Build,Version,Section,IV,Key) VALUES ('%s','%s','%s','%s','%s')";
	char updateCommandTemplate[] = "UPDATE %s SET %s='%s' WHERE Version='%s' AND Section='%s'";
	char infoCommand[MAX_SQL_COMMAND_LENGTH+1];
	uint32_t totalLengthOfString;
	int result;
	char *errmsg;

	ASSERT_RET(device,-1);
	ASSERT_RET(version,-1);
	ASSERT_RET(section,-1);
	ASSERT_RET(key,-1);
	ASSERT_RET(iv,-1);

	totalLengthOfString = strlen(device) + strlen(version) + strlen(section) + strlen(key) + strlen(iv) + strlen(insertCommandTemplate);
	if (totalLengthOfString > MAX_SQL_COMMAND_LENGTH) {
		fprintf(stderr,"%s: sql command too long.\n",__FUNCTION__);
		return -1;
	}

	// First we have to determine what this is.  Does the device table exist?
	if (DoesDeviceExist(device) == 0) {
		if (DoesSectionExist(device,version,section) == 0) {
			// If both the table and the row exist, we need to update the values.
			// Update the Key.
			sprintf(infoCommand,updateCommandTemplate,device,"Key",key,version,section);
			result = sqlite3_exec(
								db,
								infoCommand,
								NULL,
								NULL,
								&errmsg);
			if (result != SQLITE_OK) {
				fprintf(stderr,"%s: SQL Error on command %s: %s.\n",__FUNCTION__,infoCommand,errmsg);
				sqlite3_free(errmsg);
				return -1;
			}
			// Update the IV.
			sprintf(infoCommand,updateCommandTemplate,device,"IV",iv,version,section);
			result = sqlite3_exec(
								db,
								infoCommand,
								NULL,
								NULL,
								&errmsg);
			if (result != SQLITE_OK) {
				fprintf(stderr,"%s: SQL Error on command %s: %s.\n",__FUNCTION__,infoCommand,errmsg);
				sqlite3_free(errmsg);
				return -1;
			}
			return 0;
		}
	} else {
		// If the table did not exist, create it.
		if(InsertTable(device) != 0)
			return -1;
	}
	// We come here if either the table didn't exist, or the row didn't exist.  We will
	// have created the table previously, so we're all set to insert the row.
	sprintf(infoCommand,insertCommandTemplate,device,build,version,section,iv,key);

	result = sqlite3_exec(
						db,
						infoCommand,
						NULL,
						NULL,
						&errmsg);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s.\n",__FUNCTION__,infoCommand,errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	return 0;
}

int32_t IMG3_Sqlite3::RetrieveKeyAndIV(char *device, char *version, char *section, char **key, char **iv)
{
	sqlite3_stmt *stmt;
	char commandTemplate[] = "SELECT %s FROM '%s' WHERE Version='%s' AND Section='%s'";
	char retrieveValuesCommand[MAX_SQL_COMMAND_LENGTH+1];
	uint32_t totalLengthOfString;
	int result;

	ASSERT_RET(device,-1);
	ASSERT_RET(version,-1);
	ASSERT_RET(section,-1);

	if (db == NULL) {
		result = ConnectToDatabase();
		if (result)
			return result;
	}

	totalLengthOfString = strlen(device) + strlen(version) + strlen(commandTemplate) + 3;
	if (totalLengthOfString > MAX_SQL_COMMAND_LENGTH) {
		fprintf(stderr,"%s: sql command too long.\n",__FUNCTION__);
		return -1;
	}

	sprintf(retrieveValuesCommand,commandTemplate,"IV",device,version,section);

	result = sqlite3_prepare_v2(db, retrieveValuesCommand, strlen(retrieveValuesCommand) + 1, &stmt, NULL);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s\n",__FUNCTION__,retrieveValuesCommand,sqlite3_errmsg(db));
		return -1;
	}

	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		unsigned char *text;
		// Version should be the first column in the row.
		text = (unsigned char *)sqlite3_column_text(stmt, 0);
		totalLengthOfString = strlen( ( const char * )text );
		*iv = new char[ totalLengthOfString + 1 ];
		if (*iv == NULL) {
			fprintf(stderr,"%s: System Error!\n",__FUNCTION__);
			PRINT_SYSTEM_ERROR();
			sqlite3_finalize(stmt);
			return -1;
		}
		memset( *iv, 0, totalLengthOfString + 1 );
		strncpy(*iv,(const char *)text, totalLengthOfString );
		sqlite3_finalize(stmt);
	} else {
		return -1;
	}
	sprintf(retrieveValuesCommand,commandTemplate,"Key",device,version,section);

	result = sqlite3_prepare_v2(db, retrieveValuesCommand, strlen(retrieveValuesCommand) + 1, &stmt, NULL);
	if (result != SQLITE_OK) {
		fprintf(stderr,"%s: SQL Error on command %s: %s\n",__FUNCTION__,retrieveValuesCommand,sqlite3_errmsg(db));
		return -1;
	}

	result = sqlite3_step(stmt);
	if (result == SQLITE_ROW) {
		unsigned char *text;
		// Version should be the first column in the row.
		text = (unsigned char *)sqlite3_column_text(stmt, 0);
		totalLengthOfString = strlen( ( const char * )text );
		*key = new char[ totalLengthOfString + 1 ];
		if (*key == NULL) {
			fprintf(stderr,"%s: System Error!\n",__FUNCTION__);
			PRINT_SYSTEM_ERROR();
			sqlite3_finalize(stmt);
			return -1;
		}
		memset( *key, 0, totalLengthOfString + 1 );
		strncpy(*key,(const char *)text, totalLengthOfString );
		sqlite3_finalize(stmt);
	} else {
		return -1;
	}
	return 0;
}

