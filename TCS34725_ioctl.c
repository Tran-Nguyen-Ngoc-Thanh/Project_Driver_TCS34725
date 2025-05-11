#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>

#define DEVICE_FILE "/dev/tcs34725"

// IOCTL commands
#define TCS34725_IOCTL_MAGIC 't'
#define TCS34725_IOCTL_READ_C _IOR(TCS34725_IOCTL_MAGIC, 1, int)
#define TCS34725_IOCTL_READ_R _IOR(TCS34725_IOCTL_MAGIC, 2, int)
#define TCS34725_IOCTL_READ_G _IOR(TCS34725_IOCTL_MAGIC, 3, int)
#define TCS34725_IOCTL_READ_B _IOR(TCS34725_IOCTL_MAGIC, 4, int)
#define TCS34725_IOCTL_INIT   _IOW(TCS34725_IOCTL_MAGIC, 5, struct tcs34725_config)
#define TCS34725_IOCTL_READ_ChuanHoa _IOR(TCS34725_IOCTL_MAGIC, 6, int)

// config
struct tcs34725_config {
    uint8_t enable_reg;
    uint8_t rgbc_timing;
    uint8_t wait_time;
    uint8_t persistence;
    uint8_t config;
    uint8_t control;
};

// normalized
struct rgb_values {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

int main() {
    int fd;
    uint16_t cdata, rdata, gdata, bdata;
    struct rgb_values rgb;

    // Mở device file
    fd = open(DEVICE_FILE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    printf("Device opened successfully.\n");

    // Thiết lập cấu hình ban đầu
    struct tcs34725_config config = {
        .enable_reg = 0x1B,     
        .rgbc_timing = 0xF6,      
        .wait_time = 0xFF,        
        .persistence = 0x07,      
        .config = 0x00,           
        .control = 0x10           
    };

    // Gửi cấu hình xuống kernel module qua IOCTL
    if (ioctl(fd, TCS34725_IOCTL_INIT, &config) < 0) {
        perror("Failed to configure TCS34725");
        close(fd);
        return -1;
    }

    while(1)
    {
        // read clear
        if (ioctl(fd, TCS34725_IOCTL_READ_C, &cdata) < 0) {
            perror("Failed to read C data");
        }
        // read red
        if (ioctl(fd, TCS34725_IOCTL_READ_R, &rdata) < 0) {
            perror("Failed to read R data");
        } 
        // read green
        if (ioctl(fd, TCS34725_IOCTL_READ_G, &gdata) < 0) {
            perror("Failed to read G data");
        }
        //read blue
        if (ioctl(fd, TCS34725_IOCTL_READ_B, &bdata) < 0) {
            perror("Failed to read B data");
        } 
        //printf("C:%d | R: %d | G: %d | B: %d\n",cdata,rdata,gdata,bdata);

        //doc gia tri chuan hoa
        if (ioctl(fd, TCS34725_IOCTL_READ_ChuanHoa, &rgb) < 0) {
            perror("Failed to read normalized RGB data");
        } else {
            printf("Normalized -> R: %d | G: %d | B: %d\n", rgb.r, rgb.g, rgb.b);
        }
        usleep(500000);
    }

    // Đóng file
    close(fd);
    printf("Device file closed.\n");

    return 0;
}

