#include "fileops.h"


BOOL FileExists(char *filename) {
	if( access( filename, F_OK ) != -1 ) {
	    return TRUE;
	} else {
	    return FALSE;
	}
}

BOOL FileEmpty(char *filename) {
	FILE *fp = fopen(filename, "rb");
	fseek(fp, 0L, SEEK_END);
	return (ftell(fp)==0)? TRUE:FALSE;
}

uint64_t FileSize(char *filename) {
	FILE *fp = fopen(filename, "rb");
	uint64_t file_size;
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fclose(fp);
	return file_size;
}
