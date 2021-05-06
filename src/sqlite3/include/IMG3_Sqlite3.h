#ifndef __IMG3_SQLITE_H_
#define __IMG3_SQLITE_H_

#include <stdint.h>
#include <list>
#include "IMG3_typedefs.h"

using namespace std;

extern "C" {
#include "sqlite3.h"
}

#define MAX_SQL_COMMAND_LENGTH  1024*2

#define DATABASE_FILE "Resources/img3.db"

class IMG3_Sqlite3 {
private:
	sqlite3 *db;

	int32_t ConnectToDatabase(void);
	void DisconnectFromDatabase(void);

	int32_t InsertTable(char *device);

	int32_t DoesDeviceExist(char *tableName);
	int32_t DoesSectionExist(char *device, char *version, char *section);

	int32_t InsertDecryptionInfo(char *device, char *build, char *version, char *area, char *key, char *iv);

public:
	IMG3_Sqlite3();
	virtual ~IMG3_Sqlite3();

	int32_t UpdateDatabase(char *device, char *build, char *version, list<DecryptionInfo *> *info);
	int32_t RetrieveKeyAndIV(char *device, char *version, char *section, char **key, char **iv);

};

#endif // __IMG3_SQLITE_H_

