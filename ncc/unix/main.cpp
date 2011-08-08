#include <iostream>
#include <fcntl.h>

char buf[4096] __attribute__((aligned (16)));

int main (int argc, char * const argv[]) {
	if (argc != 2) {
		std::cout << "Incorrect usage\n";
		return 0;
	}

	//align buffer to 4096 boundary. Linker alignment limited this to 128 bytes, so doing it 
	//brute-force as have tons of memory to waste
	char buf[8192];
	char* pBuf = (char*)(((unsigned int)&buf / 4096) * 4096 + 4096);

	int fHandle;
	if (fHandle = open(argv[1], O_RDONLY | O_DIRECT)) {
		int bytesRead = read(fHandle, pBuf, 4096);
		pBuf[bytesRead] = '\0';
		std::cout << pBuf;
	} else {
		std::cerr << "Failed to open direct access";
	}
	close(fHandle);
	
	return 0;
}

