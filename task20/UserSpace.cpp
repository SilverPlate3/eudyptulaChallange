#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>

#define FAT_IOCTL_SET_LABEL	_IOW('r', 0x14, char *)


int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " {path}" << std::endl;
        return 1;
    }
    
    std::cout << "trying ot open: " << argv[1] << std::endl;
    int fd = open(argv[1], O_RDONLY);
    if(fd < 0)
    {
        std::cout << "open failed. errno: " << errno << std::endl;
        return 1;
    }

    if(ioctl(fd, FAT_IOCTL_SET_LABEL, "NEW") < 0)
    {
        std::cout << "first ioctl failed. errno: " << errno << std::endl;
        return 1;
    }

    return 0;
}