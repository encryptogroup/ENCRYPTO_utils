#ifndef _FILEOPS_H_
#define _FILEOPS_H_

#include <stdio.h>
#include "typedefs.h"

	//TODO move to utils
	/*File Operation Methods*/
	/**
		Function to check if the file exists.
	 	\param filename		Name of the file.
	*/
	BOOL FileExists(char *filename);
	/**
		Function to check if the file is empty or not.
		\param filename		Name of the file.
	*/
	BOOL FileEmpty(char *filename);
	/**
		Function to obtain the size of a given file.
		\param filename		Name of the file.
	*/
	uint64_t FileSize(char *filename);

#endif
