// ncc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2) {
		printf("Usage: ncc <filename>\n");
		return 1;
	}

	char* pBuf = new char[4096];
	HANDLE hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);

	DWORD bytesRead;
	ReadFile(hFile, pBuf, 4096, &bytesRead, NULL);
	printf("%s", pBuf);

	CloseHandle(hFile);
	delete pBuf;

	return 0;
}

