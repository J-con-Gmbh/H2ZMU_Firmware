//
// Created by jriessner on 08.06.23.
//

#ifndef UART_COMMUNICATION_UART_H
#define UART_COMMUNICATION_UART_H

#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <string>
#include <vector>

#define BUFFER_LEN 256

#define PING true
#define PONG false

namespace uart {

    inline std::string strip(std::string toStrip) {
        const char *chars = toStrip.c_str();
        int stringLen = toStrip.size();
        int cutFront = 0;
        int cutBack = 0;

        for (int i = 0; i < stringLen; ++i) {
            if (((int) chars[i] < 33) || ((int) chars[i] > 126)) {
                cutFront += 1;
            } else {
                break;
            }
        }
        for (int i = stringLen - 1; i >= 0; i--) {
            if (((int) chars[i] < 33) || ((int) chars[i] > 126)) {
                cutBack += 1;
            } else {
                break;
            }
        }

        std::string stripped = toStrip.substr(cutFront, stringLen - (cutFront + cutBack));

        return stripped;
    }

    struct config {
        std::string connection_point;
        int rw_policy = O_RDWR;
        bool blocking = false; // when false set O_NONBLOCK at open()
        bool terminal = false; // when false set O_NOCTTY at open()
        unsigned int usec_timeout_serial = 10000; // = 10 ms
        struct termios options{
                .c_iflag = IGNPAR,
                .c_oflag = 0,
                .c_cflag = B9600 | CS8 | PARENB | CLOCAL | CREAD,
                .c_lflag = 0,
        };
        std::vector<unsigned char> handshake_ping = {'r', 'd', 'y', '?'};
        std::vector<unsigned char> handshake_pong = {'r', 'd', 'y', '!'};
        uint32_t ms_timeout_handshake = 30000; // 30 seconds
    };

    class Uart {
    private:
        struct config conf;
        int uartFilestream = -1;
        int restoreSocketModes;

        bool handshakeMatching(bool with, std::vector<unsigned char> handshake);

    public:
        Uart();

        void setup(struct config conf);

        bool connect();

        bool disconnect() const;

        bool handshake(bool active);

        bool handshake(bool active, uint32_t timeout_ms);

        //std::vector<unsigned char> readUartBinaryByteCount() const;
        std::vector<unsigned char> readUartBinary(char until = 0x00, uint32_t timeout_ms = 10) const;
        std::vector<unsigned char> readUartBinaryByteCount(uint32_t bytes = 0, uint32_t timeout_ms = 10) const;

        bool writeUartBinary(std::vector<unsigned char> data) const;

        std::string readUart() const;

        bool writeUart(std::string data) const;

        void flushWrite() const;

        void flushRead() const;

        void flushAll() const;
    };
}

#endif //UART_COMMUNICATION_UART_H
