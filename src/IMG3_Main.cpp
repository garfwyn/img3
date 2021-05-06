#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <list>

//extern "C" {
#include "IMG3_defines.h"
#include "IMG3_typedefs.h"
#include "IMG3_includes.h"
#include "IMG3_Functions.h"
//}

using namespace std;

ParserOperation operation;

char *deviceName = NULL;
char *deviceBuild = NULL;
char *deviceVersion = NULL;
char *archiveFileName = NULL;
char *patchFileName = NULL;
char *section = NULL;

char img3SupportedFiles[][30] = {
		"AppleLogo",
		"BatteryCharging0",
		"BatteryCharging1",
		"BatteryFull",
		"BatteryLow0",
		"BatteryLow1",
		"DeviceTree",
		"GlyphCharging",
		"GlyphPlugin",
		"iBEC",
		"iBoot",
		"iBSS",
		"Kernelcache",
		"LLB",
		"RecoveryMode"
};

list<DecryptionInfo *> *decryptionInfo;

/*****************************************************************************************
 * There are several potential tags that might exist in an img3 file, including:
 *	DATA - payload encrypted with an encryption key.
 *	KBAG - encryption key used to encrypt the DATA section, wrapped by the GID key.
 *	PROD - production status.
 *	SDOM - security domain.
 *	SEPO - security epoch.
 *	SHSH - signature tag generated by performing a hash operation on at least a part of the header,
 and one or more constructed tags as specified in the header.  Store the signed hash.
 *	CERT - stores the certificate chain used to sign the hash stored in the signature tag.
 ****************************************************************************************/

void PrintUsage(char *progName, char *command) {
	if (command == NULL) {
		fprintf(stdout,	"%s: a program for interacting with Apple img3 files.\n\n",	progName);
		fprintf(stdout, "Standard commands:\n");
		fprintf(stdout, "extract\t\tlist\t\tdec\t\tupdate\t\tpatch\n\n");
		fprintf(stdout,	"For more information on each, enter the command and use '-h'.\n\n");
		return;
	}
	if (strcmp(command, "extract") == 0) {
		fprintf(stdout,	"%s extract command: extracts one, or all, sections of an img3 archive.\n",	progName);
		fprintf(stdout, "Syntax: %s %s -s <section> img3_file\n\n", progName, command);
		fprintf(stdout, "Options:\n");
		fprintf(stdout,	"-s\tSpecifies the specific area that should be extracted from the archive.  The currently\n");
		fprintf(stdout, "\tsupported areas include the following:\n");
		fprintf(stdout, "\t\tAppleLogo\n");
		fprintf(stdout, "\t\tBatteryCharging0\n");
		fprintf(stdout, "\t\tBatteryCharging1\n");
		fprintf(stdout, "\t\tBatteryFull\n");
		fprintf(stdout, "\t\tBatteryLow0\n");
		fprintf(stdout, "\t\tBatteryLow1\n");
		fprintf(stdout, "\t\tDeviceTree\n");
		fprintf(stdout, "\t\tGlyphCharging\n");
		fprintf(stdout, "\t\tGlyphPlugin\n");
		fprintf(stdout, "\t\tiBoot\n");
		fprintf(stdout, "\t\tLLB\n");
		fprintf(stdout, "\t\tRecoveryMode\n");
		fprintf(stdout, "\t\tiBEC\n");
		fprintf(stdout, "\t\tiBSS\n");
		fprintf(stdout, "\t\tKernelCache\n");
		fprintf(stdout,	"\tAll areas are case-insensitive.  If this parameter is left blank, all files will be \n");
		fprintf(stdout, "\textracted from the archive.\n");
	} else if (strcmp(command, "list") == 0) {
		fprintf(stdout,	"%s list command: list all files contained within the img3 archive.\n",	progName);
		fprintf(stdout, "Syntax: %s %s img3_file\n\n", progName, command);
	} else if (strcmp(command, "decrypt") == 0) {
		fprintf(stdout,	"%s decrypt command: decrypts a specific device section within an img3 archive.\n",	progName);
		fprintf(stdout, "Syntax: %s %s -d <device> -v <version -o <output file> -s <section> img3_file\n\n", progName, command);
		fprintf(stdout, "Options:\n");
		fprintf(stdout,	"-d\tSpecifies the device that the img3 file references.  Currently supported devices include:\n");
		fprintf(stdout, "\t\tAppleTV\n");
		fprintf(stdout, "\t\tiPad       (1st generation)\n");
		fprintf(stdout, "\t\tiPad2_Wifi (2nd generation Wi-Fi model)\n");
		fprintf(stdout, "\t\tiPad2_GSM  (2nd generation GSM model)\n");
		fprintf(stdout, "\t\tiPad2_CDMA (2nd generation CDMA model)\n");
		fprintf(stdout, "\t\tiPhone\n");
		fprintf(stdout, "\t\tiPhone3G\n");
		fprintf(stdout, "\t\tiPhone3GS\n");
		fprintf(stdout, "\t\tiPhone4    (GSM and CDMA models)\n");
		fprintf(stdout, "\t\tiPod       (1st generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod2      (2nd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod3      (3rd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod4      (4th generation iPod Touch)\n");
		fprintf(stdout, "\tThese are case sensitive.  If this is NOT specified, the program will attempt to determine this\n");
		fprintf(stdout,	"\tinformation based upon the name of the file.  This is not perfect though, so neglect at your\n");
		fprintf(stdout, "\town risk!\n");
		fprintf(stdout,	"-v\tSpecifies the version of firmware the img3 archive represents.  As with the device, if this\n");
		fprintf(stdout,	"\toption is not specified, the program will attempt to determine it based upon the file name.\n");
		fprintf(stdout,	"-s\tSpecifies the specific area that should be extracted from the archive and decrypted.  The currently\n");
		fprintf(stdout, "\tsupported areas include the following:\n");
		fprintf(stdout, "\t\tAppleLogo\n");
		fprintf(stdout, "\t\tBatteryCharging0\n");
		fprintf(stdout, "\t\tBatteryCharging1\n");
		fprintf(stdout, "\t\tBatteryFull\n");
		fprintf(stdout, "\t\tBatteryLow0\n");
		fprintf(stdout, "\t\tBatteryLow1\n");
		fprintf(stdout, "\t\tDeviceTree\n");
		fprintf(stdout, "\t\tGlyphCharging\n");
		fprintf(stdout, "\t\tGlyphPlugin\n");
		fprintf(stdout, "\t\tiBoot\n");
		fprintf(stdout, "\t\tLLB\n");
		fprintf(stdout, "\t\tRecoveryMode\n");
		fprintf(stdout, "\t\tiBEC\n");
		fprintf(stdout, "\t\tiBSS\n");
		fprintf(stdout, "\t\tKernelCache\n");
		fprintf(stdout,	"\tAll areas are case-insensitive.  This parameter is NOT optional.\n");
	} else if (strcmp(command, "update") == 0) {
		fprintf(stdout,	"%s update command: updates the SQLite3 database with new keys and iv for a specified device using HTML \n", progName);
		fprintf(stdout, "pages from the website 'theiphonewiki.com'.\n");
		fprintf(stdout,	"Syntax: %s %s -d <device> -v <version> theiphonewiki.com_html_page.\n\n", progName, command);
		fprintf(stdout, "Options:\n");
		fprintf(stdout, "-d\tThis parameter is NOT optional.  It must be included.  Currently supported devices include: \n");
		fprintf(stdout, "\t\tAppleTV\n");
		fprintf(stdout, "\t\tiPad       (1st generation)\n");
		fprintf(stdout, "\t\tiPad2_Wifi (2nd generation Wi-Fi model)\n");
		fprintf(stdout, "\t\tiPad2_GSM  (2nd generation GSM model)\n");
		fprintf(stdout, "\t\tiPad2_CDMA (2nd generation CDMA model)\n");
		fprintf(stdout, "\t\tiPhone\n");
		fprintf(stdout, "\t\tiPhone3G\n");
		fprintf(stdout, "\t\tiPhone3GS\n");
		fprintf(stdout, "\t\tiPhone4    (GSM and CDMA models)\n");
		fprintf(stdout, "\t\tiPod       (1st generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod2      (2nd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod3      (3rd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod4      (4th generation iPod Touch)\n");
		fprintf(stdout, "\tThese are case sensitive.\n");
		fprintf(stdout,	"-v\tSpecifies the version of firmware the img3 archive represents.  This parameter is NOT optional.\n");
		fprintf(stdout, "-b\tSpecifies the build version of the firmware the img3 archive represents.  The program will \n" );
		fprintf(stdout, "\t\tattempt to extract this information from the file name if possible.  Otherwise, you are \n" );
		fprintf(stdout, "\t\trequired to enter it.\n" );
	} else if (strcmp(command, "patch") == 0) {
		fprintf(stdout,	"%s patch command: patches the kernel section within an img3 archive.\n", progName);
		fprintf(stdout,	"Syntax: %s %s -d <device> -v <version> patch_file img3_file\n\n",	progName, command);
		fprintf(stdout, "Options:\n");
		fprintf(stdout,	"-d\tSpecifies the device that the img3 file references.  Currently supported devices include:\n");
		fprintf(stdout, "\t\tAppleTV\n");
		fprintf(stdout, "\t\tiPad       (1st generation)\n");
		fprintf(stdout, "\t\tiPad2_Wifi (2nd generation Wi-Fi model)\n");
		fprintf(stdout, "\t\tiPad2_GSM  (2nd generation GSM model)\n");
		fprintf(stdout, "\t\tiPad2_CDMA (2nd generation CDMA model)\n");
		fprintf(stdout, "\t\tiPhone\n");
		fprintf(stdout, "\t\tiPhone3G\n");
		fprintf(stdout, "\t\tiPhone3GS\n");
		fprintf(stdout, "\t\tiPhone4    (GSM and CDMA models)\n");
		fprintf(stdout, "\t\tiPod       (1st generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod2      (2nd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod3      (3rd generation iPod Touch)\n");
		fprintf(stdout, "\t\tiPod4      (4th generation iPod Touch)\n");
		fprintf(stdout,	"\tThese are case sensitive.  If this is NOT specified, the program will attempt to determine this\n");
		fprintf(stdout,	"\tinformation based upon the name of the file.  This is not perfect though, so neglect at your\n");
		fprintf(stdout, "\town risk!\n");
		fprintf(stdout,	"-v\tSpecifies the version of firmware the img3 archive represents.  As with the device, if this\n");
		fprintf(stdout,	"\toption is not specified, the program will attempt to determine it based upon the file name.\n");
	} else {
		fprintf(stdout, "Unknown command entered: %s.\n", command);
	}
	fprintf(stdout, "\n");
}

int32_t ParseArguments(int argc, char *argv[]) {
	char *tok, *tok1;
	int count, index, strIndex;
	char response;

	if (argc < 2) {
		PrintUsage(argv[0], NULL);
		return -1;
	}

	if (strcmp(argv[1], "decrypt") == 0) {
		operation = DECRYPT_IMG3_FILE;
	} else if (strcmp(argv[1], "update") == 0) {
		operation = UPDATE_IMG3_DATABASE;
	} else if (strcmp(argv[1], "list") == 0) {
		operation = LIST_ARCHIVE_FILES;
	} else if (strcmp(argv[1], "extract") == 0) {
		operation = EXTRACT_FILE;
	} else if (strcmp(argv[1], "patch") == 0) {
		operation = PATCH_KERNEL;
	} else if (strcmp(argv[1], "parse") == 0) {
		operation = PARSE_FILE;
	} else if( strcmp(argv[1], "decompress") == 0) {
		operation = DECOMPRESS_FILE;
	} else {
		PrintUsage(argv[0], NULL);
		return -1;
	}

	switch (operation) {
	case DECRYPT_IMG3_FILE: {
		if (argc < 3) {
			PrintUsage(argv[0], argv[1]);
			return -1;
		}

		for (index = 2; index < argc; index++) {
			if (index + 1 >= argc) {
				break;
			}
			if (strcmp(argv[index], "-d") == 0) {
				count = strlen(argv[++index]);
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, argv[index], count);
				deviceName[count] = '\0';
			} else if (strcmp(argv[index], "-b") == 0) {
				count = strlen(argv[++index]);
				deviceBuild = new char[count + 1];
				if (deviceBuild == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceBuild, argv[index], count);
				deviceBuild[count] = '\0';
			} else if (strcmp(argv[index], "-v") == 0) {
				count = strlen(argv[++index]);
				deviceVersion = new char[count + 1];
				if (deviceVersion == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceVersion, argv[index], count);
				deviceVersion[count] = '\0';
			} else if (strcmp(argv[index], "-s") == 0) {
				count = strlen(argv[++index]);
				section = new char[count + 1];
				if (section == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(section, argv[index], count);
				for (strIndex = 0; strIndex < count; strIndex++) {
					section[strIndex] = tolower(section[strIndex]);
				}
				section[count] = '\0';
			} else if (strcmp(argv[index], "-h") == 0) {
				PrintUsage(argv[0], argv[1]);
				return -1;
			} else {
				fprintf(stderr, "%s: invalid option %s.\n", __FUNCTION__,
						argv[index]);
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
		}

		count = strlen(argv[argc - 1]);
		archiveFileName = new char[count + 1];
		if (archiveFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(archiveFileName, argv[argc - 1], count);
		archiveFileName[count] = '\0';

		// The file name for the devices typically takes on the following structure:
		//		<device>,1_<firmware_number>_<firmware_name>_Restore.ipsw

		if (deviceName == NULL) {
			// If the device name wasn't specified, attempt to get it from the file name.  The device name
			// should be everything up to the comma.
			if (strstr(archiveFileName, "iPhone1,1") != NULL) {
				count = strlen("iPhone");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone", count);
			} else if (strstr(archiveFileName, "iPhone1,2") != NULL) {
				count = strlen("iPhone_3");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_3", count);
			} else if (strstr(archiveFileName, "iPhone2,1") != NULL) {
				count = strlen("iPhone_3GS");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_3GS", count);
			} else if (strstr(archiveFileName, "iPhone3,1") != NULL) {
				count = strlen("iPhone_4");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_4", count);
			} else {
				tok = strchr(archiveFileName, ',');
				if (tok == NULL) {
					fprintf(
							stderr,
							"%s: Unable to determine device name from the file name.\n",
							__FUNCTION__);
					return -1;
				}
				count = tok - archiveFileName;
				deviceName = new char[count + 10];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				memcpy(deviceName, archiveFileName, count);
				deviceName[count] = '\0';
			}
		}
		if (deviceVersion == NULL) {
			// If the device version wasn't specified, attempt to get it from the file name.  The version name
			// should be everything between the '_' characters.
			tok = strchr(archiveFileName, '_');
			if (tok == NULL) {
				fprintf(
						stderr,
						"%s: Unable to determine device version from file name.\n",
						__FUNCTION__);
				return -1;
			}
			tok1 = strchr(tok + 1, '_');
			if (tok1 == NULL) {
				fprintf(
						stderr,
						"%s: Unable to determine device version from file name.\n",
						__FUNCTION__);
				return -1;
			}
			count = tok1 - tok - 1;
			deviceVersion = new char[count + 1];
			if (deviceVersion == NULL) {
				PRINT_SYSTEM_ERROR();
				return -1;
			}
			memcpy(deviceVersion, tok + 1, count);
			deviceName[count] = '\0';
		}
		break;
	}
	case LIST_ARCHIVE_FILES: {
		if (argc < 3) {
			PrintUsage(argv[0], argv[1]);
			return -1;
		}
		if (strcmp(argv[2], "-h") == 0) {
			PrintUsage(argv[0], argv[1]);
			return -1;
		}
		count = strlen(argv[argc - 1]);
		archiveFileName = new char[count + 1];
		if (archiveFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(archiveFileName, argv[argc - 1], count);
		archiveFileName[count] = '\0';
		break;
	}
	case UPDATE_IMG3_DATABASE: {
		if (argc < 3) {
			fprintf(stderr,
					"Invalid number of arguments for database update.\n");
			PrintUsage(argv[0], argv[1]);
			return -1;
		}
		for (index = 2; index < 8; index++) {
			if (index + 1 >= argc) {
				break;
			}
			if (strcmp(argv[index], "-d") == 0) {
				count = strlen(argv[++index]);
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, argv[index], count);
				deviceName[count] = '\0';
			} else if (strcmp(argv[index], "-b") == 0) {
				count = strlen(argv[++index]);
				deviceBuild = new char[count + 1];
				if (deviceBuild == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceBuild, argv[index], count);
				deviceBuild[count] = '\0';
			} else if (strcmp(argv[index], "-v") == 0) {
				count = strlen(argv[++index]);
				deviceVersion = new char[count + 1];
				if (deviceVersion == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceVersion, argv[index], count);
				deviceVersion[count] = '\0';
			} else if (strcmp(argv[index], "-h") == 0) {
				PrintUsage(argv[0], argv[1]);
				return -1;
			} else {
				fprintf(stderr, "Invalid option: %s.\n", argv[index]);
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
		}
		count = strlen(argv[argc - 1]);
		archiveFileName = new char[count + 1];
		if (archiveFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(archiveFileName, argv[argc - 1], count);
		archiveFileName[count] = '\0';

		if (deviceName == NULL) {
			tok = strchr(archiveFileName,'=');
			if (tok == NULL) {
				fprintf(stderr, "No device name found.\n");
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
			tok++;
			tok1 = strchr(tok,'(');
			if (tok1 == NULL) {
				fprintf(stderr, "No device name found.\n");
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
			count = tok1 - tok - 1; // There's a trailing '_' character after the device name.
			deviceName = new char[count+1];
			if (deviceName == NULL) {
				PRINT_SYSTEM_ERROR();
				return -1;
			}
			memcpy(deviceName,tok,count);
			deviceName[count] = '\0';
		}
		if (deviceBuild == NULL) {
			tok = strchr(archiveFileName,'(');
			if (tok == NULL) {
				fprintf(stderr, "No device build found.\n");
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
			count = tok - archiveFileName - 1;
			deviceBuild = new char[count+1];
			if (deviceBuild == NULL) {
				PRINT_SYSTEM_ERROR();
				return -1;
			}
			memcpy(deviceBuild,archiveFileName,count);
			deviceBuild[count] = '\0';
			fprintf(stdout,"Attempting to use device build: %s.  Is that correct?([Y]/N) ", deviceBuild);
			response = getchar();
			while( response == 'N' || response == 'n' ) {
				delete deviceBuild;
				deviceBuild = new char[50];
				if( deviceBuild == NULL ) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				fprintf( stdout, "Please enter a device build: " );
				scanf( "%s", deviceBuild );
				printf( "Using %s as the device build.  Is that correct?([Y]/N) ", deviceBuild );
				response = getchar();
			}
		}
		if (deviceVersion == NULL) {
			fprintf(stdout,"No device version specified.\n");
			return -1;
		}
		break;
	}
	case EXTRACT_FILE: {
		if (argc < 3) {
			PrintUsage(argv[0], argv[1]);
			return -1;
		}
		for (index = 2; index < 4; index++) {
			if (index + 1 >= argc) {
				break;
			}
			if (strcmp(argv[index], "-s") == 0) {
				count = strlen(argv[++index]);
				section = new char[count + 1];
				if (section == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(section, argv[index], count);
				for (strIndex = 0; strIndex < count; strIndex++) {
					section[strIndex] = tolower(section[strIndex]);
				}
				section[count] = '\0';
			} else if (strcmp(argv[index], "-h") == 0) {
				PrintUsage(argv[0], argv[1]);
				return -1;
			} else {
				fprintf(stderr, "Invalid option: %s.\n", argv[index]);
				PrintUsage(argv[0], argv[1]);
				return -1;
			}
		}
		count = strlen(argv[argc - 1]);
		archiveFileName = new char[count + 1];
		if (archiveFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(archiveFileName, argv[argc - 1], count);
		archiveFileName[count] = '\0';
		break;
	}
	case PATCH_KERNEL: {
		if (argc < 7) {
			PrintUsage(argv[0], argv[1]);
			return -1;
		}

		for (index = 2; index < argc; index++) {
			if (index + 1 >= argc) {
				break;
			}
			if (strcmp(argv[index], "-d") == 0) {
				count = strlen(argv[++index]);
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, argv[index], count);
				deviceName[count] = '\0';
			} else if (strcmp(argv[index], "-v") == 0) {
				count = strlen(argv[++index]);
				deviceVersion = new char[count + 1];
				if (deviceVersion == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceVersion, argv[index], count);
				deviceVersion[count] = '\0';
			}
		}

		count = strlen(argv[argc - 2]);
		patchFileName = new char[count + 1];
		if (patchFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(patchFileName, argv[argc - 2], count);
		patchFileName[count] = '\0';

		count = strlen(argv[argc - 1]);
		archiveFileName = new char[count + 1];
		if (archiveFileName == NULL) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		strncpy(archiveFileName, argv[argc - 1], count);
		archiveFileName[count] = '\0';

		// The file name for the devices typically takes on the following structure:
		//		<device>,1_<firmware_number>_<firmware_name>_Restore.ipsw

		if (deviceName == NULL) {
			// If the device name wasn't specified, attempt to get it from the file name.  The device name
			// should be everything up to the comma.
			if (strstr(archiveFileName, "iPhone1,1") != NULL) {
				count = strlen("iPhone");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone", count);
			} else if (strstr(archiveFileName, "iPhone1,2") != NULL) {
				count = strlen("iPhone_3");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_3", count);
			} else if (strstr(archiveFileName, "iPhone2,1") != NULL) {
				count = strlen("iPhone_3GS");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_3GS", count);
			} else if (strstr(archiveFileName, "iPhone3,1") != NULL) {
				count = strlen("iPhone_4");
				deviceName = new char[count + 1];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				strncpy(deviceName, "iPhone_4", count);
			} else {
				tok = strchr(archiveFileName, ',');
				if (tok == NULL) {
					fprintf(
							stderr,
							"%s: Unable to determine device name from the file name.\n",
							__FUNCTION__);
					return -1;
				}
				count = tok - archiveFileName;
				deviceName = new char[count + 10];
				if (deviceName == NULL) {
					PRINT_SYSTEM_ERROR();
					return -1;
				}
				memcpy(deviceName, archiveFileName, count);
				deviceName[count] = '\0';
			}
		}
		if (deviceVersion == NULL) {
			// If the device version wasn't specified, attempt to get it from the file name.  The version name
			// should be everything between the '_' characters.
			tok = strchr(archiveFileName, '_');
			if (tok == NULL) {
				fprintf(
						stderr,
						"%s: Unable to determine device version from file name.\n",
						__FUNCTION__);
				return -1;
			}
			tok1 = strchr(tok + 1, '_');
			if (tok1 == NULL) {
				fprintf(
						stderr,
						"%s: Unable to determine device version from file name.\n",
						__FUNCTION__);
				return -1;
			}
			count = tok1 - tok - 1;
			deviceVersion = new char[count + 1];
			if (deviceVersion == NULL) {
				PRINT_SYSTEM_ERROR();
				return -1;
			}
			memcpy(deviceVersion, tok + 1, count);
			deviceName[count] = '\0';
		}
		section = new char[20];
		strncpy(section, "kernelcache", 20);
		break;
	}
	case PARSE_FILE:
		if ( argc != 3 ) {
			fprintf( stderr, "Invalid parameter.\n" );
			return -1;
		}
		count = strlen( argv[2] );
		archiveFileName = new char[count + 1];
		if ( archiveFileName == NULL ) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		memcpy( archiveFileName, argv[2], count );
		archiveFileName[count] = '\0';
		break;
	case DECOMPRESS_FILE:
		if( argc != 3 ) {
			fprintf( stderr, "Invalid parameter.\n" );
			return -1;
		}
		count = strlen( argv[2] );
		archiveFileName = new char[count + 1];
		if( archiveFileName == NULL ) {
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		memcpy( archiveFileName, argv[2], count );
		archiveFileName[count] = '\0';
		break;
	default:
		fprintf( stderr, "Invalid operation selected.\n" );
		PrintUsage( argv[0], NULL );
		return -1;
	}
	return 0;
}

int main( int argc, char *argv[] ) {
	if ( ParseArguments( argc, argv ) )
		exit(1);

	switch ( operation ) {
	case DECRYPT_IMG3_FILE: 
		DecryptIMG3File( archiveFileName, NULL, deviceName, deviceVersion, section );
		break;
	case LIST_ARCHIVE_FILES: 
		ListArchiveFiles( archiveFileName );
		break;
	case EXTRACT_FILE: 
		ExtractFileFromArchive( archiveFileName, section );
		break;
	case UPDATE_IMG3_DATABASE: 
		UpdateIMG3Database( archiveFileName, deviceName, deviceVersion, deviceBuild );
		break;
	case PATCH_KERNEL: 
		fprintf( stdout, "Patching with file: %s.\n", patchFileName );
		PatchKernelFile( archiveFileName, NULL, patchFileName, deviceName, deviceVersion );
		break;
	case PARSE_FILE:
		fprintf( stdout, "Parsing file: %s.\n", archiveFileName );
		ParseIMG3File( archiveFileName );
		break;
	case DECOMPRESS_FILE:
		fprintf( stdout, "Decompressing file: %s.\n", archiveFileName );
		DecompressLZSSFile( archiveFileName );
		break;
	default: 
		fprintf( stderr, "Unknown command!  Aborintg...\n" );
		break;
	}

	return 0;
}
