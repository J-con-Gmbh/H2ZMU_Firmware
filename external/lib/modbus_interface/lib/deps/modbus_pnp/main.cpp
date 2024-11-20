
#include <cstdio>
#include <memory.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <thread>
#include "modbus_pnp.h"


int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty{};
    if (tcgetattr (fd, &tty) != 0)
    {
        fprintf (stderr, "error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        fprintf (stderr, "error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void
set_blocking (int fd, int should_block)
{
    struct termios tty{};
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        fprintf (stderr, "error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        fprintf (stderr, "error %d setting term attributes", errno);
}

void serve(int fd, struct mb_ctx ctx) {
    uint8_t buf [256];
    int n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read

    if (n < 1) {
        return;
    }

    std::cout << "\nEmpfangen:" << std::endl;
    auto tel = parse_telegram(buf, n);

    if (*tel.crc != calculate_crc(&tel)) {
        std::cout << "Error: received crc (" << std::hex << calculate_crc(&tel) << ") not matching calculated " << *tel.crc << std::endl;
        return;
    }


    if (*tel.mb_fn_code == 0x01) {

        auto reply = reply_read_do(ctx, tel);
        std::cout << "MB fn 0x01 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;


        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x02) {

        auto reply = reply_read_di(ctx, tel);
        std::cout << "MB fn 0x02 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;


        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x03) {

        auto reply = reply_read_ao(ctx, tel);
        std::cout << "MB fn 0x03 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;


        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x04) {

        auto reply = reply_read_ai(ctx, tel);
        std::cout << "MB fn 0x04 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;


        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x05) {

        auto reply = reply_write_single_do(ctx, tel);
        std::cout << "MB fn 0x05 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;


        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x06) {

        auto reply = reply_write_single_ao(ctx, tel);
        std::cout << "MB fn 0x06 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;

        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x0F) {

        auto reply = reply_write_multiple_do(ctx, tel);
        std::cout << "MB fn 0x0F invoked" << std::endl;

        std::cout << "Sende:" << std::endl;

        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    } else if (*tel.mb_fn_code == 0x10) {

        auto reply = reply_write_multiple_ao(ctx, tel);
        std::cout << "MB fn 0x10 invoked" << std::endl;

        std::cout << "Sende:" << std::endl;

        for (int i = 0; i < reply.size; ++i) {
            printf("0x%02x ", (uint8_t) (reply.payload[i] & 0xff));
        }
        printf("\n");

        write(fd, reply.payload, reply.size);
    }
}

int main(int argc, char**argv) {
    std::string port = "/tmp/ttyV0";
    if (argc > 1) {
        port = std::string(argv[1]);
    }

    int fd = open (port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        fprintf (stderr, "error %d opening %s: %s", errno, port.c_str(), strerror (errno));
        return 1;
    }

    set_interface_attribs (fd, B19200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd, 0);                // set no blocking

    //write (fd, "hello!\n", 7);           // send 7 character greeting

    usleep ((7 + 25) * 100);             // sleep enough to transmit the 7 plus
    // receive 25:  approx 100 uS per char transmit
    char buf [100];
    int n = read (fd, buf, sizeof buf);  // read up to 100 characters if ready to read




    mb_ctx ctx = init_context(1, 10, 10, 10, 10);
    ctx.s_id = 1;
    std::cout << "Ready!" << std::endl;

    ctx.reg_do[0] = 0x00;
    ctx.reg_di[0] = 0b01101001;
    ctx.reg_ai[0] = 0x00;
    ctx.reg_ai[1] = 0x10;
    ctx.reg_ai[2] = 0x10;
    ctx.reg_ai[3] = 0x00;
    ctx.reg_ai[4] = 0x00;
    ctx.reg_ai[5] = 0x00;

    ctx.reg_ao[0] = 0x00;
    ctx.reg_ao[1] = 0x01;
    ctx.reg_ao[2] = 0x01;
    ctx.reg_ao[3] = 0x00;
    ctx.reg_ao[4] = 0x00;
    ctx.reg_ao[5] = 0x00;


    while (true) {
        serve(fd, ctx);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }



    free_context(ctx);



    return 0;
}
