#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main (int argc, char * const argv[]) {
    // insert code here...
	if (argc != 2) {
		std::cout << "Incorrect usage\n";
		return 0;
	}
	
	/*r1, r2, err := syscall.Syscall(syscall.SYS_FCNTL, uintptr(self.file.Fd()), syscall.F_NOCACHE, 1)
    if err != 0 {
        fmt.Printf("Syscall to SYS_FCNTL failed\n\tr1=%v, r2=%v, err=%v\n", r1, r2, err)
        self.Close()
        return false
    }*/
	
	char buf[4096];
			 
	int fHandle = open(argv[1], O_RDONLY);
	if (fcntl(fHandle, F_NOCACHE, 1) >= 0 ) {
		int bytesRead = read(fHandle, &buf, sizeof(buf));
		buf[bytesRead] = '\0';
		std::cout << buf;
	} else {
		printf("Failed to open direct access");
	}
	close(fHandle);
	
	return 0;
}
