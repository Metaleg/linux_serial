#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#include <time.h>

int set_interface_attribs(int fd, int speed) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) < 0) {
        perror("Error from tcgetattr\n");
        return EXIT_FAILURE;
    }
    
    cfsetospeed(&tty, (speed_t) speed);
    cfsetispeed(&tty, (speed_t) speed);
    
    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */
    
    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;
    
    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int open_port_linux(char const * portname) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port_linux: Unable to open the port\n");
        return EXIT_FAILURE;
    }
    else
        fcntl(fd, F_SETFL, 0);

    return fd;
}

int send_linux(char const * portname) {
    srand(time(NULL));
    int fd = open_port_linux(portname);
    set_interface_attribs(fd, B115200);
    
    int counter = 1000;
    while (counter > 0) {
        char src = 97 + random() % 25;
        if (write(fd, &src, 1) < 0) {
            perror("Write failed!\n");
            return EXIT_FAILURE;
        }
        --counter;
    }
    
    close(fd);
    return EXIT_SUCCESS;
}

int receive_linux(char const * portname) {
    int fd = open_port_linux(portname);
    set_interface_attribs(fd, B115200);
    
    int counter = 1000;
    while (counter > 0) {
        char dst = 0;
        if (read(fd, &dst, 1) < 0) {
            perror("Read failed!\n");
            return EXIT_FAILURE;
        }
        printf("%c", dst);
        --counter;
    }

    close(fd);
    return EXIT_SUCCESS;
}


int main() {
    pid_t pid1 = 0, pid2 = 0;
    int st;
    
    pid1 = fork();
    if (pid1 == 0) {
        char const * portname = "/dev/pts/2";
        st = send_linux(portname);
        exit(pid1);
    }
    
    pid2 = fork();
    if (pid2 == 0) {
        char const * portname = "/dev/pts/4";
        st = receive_linux(portname);
        exit(pid2);
    }
    
    return 0;
}