//
// Created by jriessner on 08.06.23.
//

#include <iostream>
#include <utility>
#include <chrono>
#include <thread>
#include <cstring>

#include <sys/ioctl.h>
#include <vector>
#include <bits/stdc++.h>

#include "../include/Uart.h"

namespace uart {
    Uart::Uart() = default;

    void Uart::setup(struct config conf) {
        this->conf = std::move(conf);
    }

    bool Uart::connect() {

        int no_block = 0;
        int no_ctty = 0;

        if (!this->conf.blocking) {
            no_block = O_NONBLOCK;
        }
        if (!this->conf.terminal) {
            no_ctty = O_NOCTTY;
        }

        this->uartFilestream = open(this->conf.connection_point.c_str(), this->conf.rw_policy | no_block | no_ctty);
        this->restoreSocketModes = fcntl(this->uartFilestream, F_GETFD);
        if (this->uartFilestream == -1) {
            printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
            return false;
        }
        // Flush Serial Port
        //ioctl(this->uartFilestream, TCFLSH, 2); // flush input and output buffer

        struct termios options;

        if (-1 == tcgetattr(this->uartFilestream, &options)) {
            std::cout << "tclflush failed, errno: " << errno << std::endl;
            return false;
        }

        options.c_cflag = this->conf.options.c_cflag;
        options.c_iflag = this->conf.options.c_iflag;
        options.c_oflag = this->conf.options.c_oflag;
        options.c_lflag = this->conf.options.c_lflag;
        /*
        // Flush UART -> no confusing, old data
        if (-1 == tcflush(this->uartFilestream, TCIFLUSH)) {
            perror("tclflush failed");
            return false;
        }

        if (-1 == tcsetattr(this->uartFilestream, TCSANOW, &options)) {
            perror("tcsetattr failed");
            return false;
        }
        */

        return true;
    }

    bool Uart::disconnect() const {
        this->flushAll();

        return (0 == close(this->uartFilestream));
    }

    std::string Uart::readUart() const {

        std::vector<unsigned char> data = this->readUartBinary();
        std::string s_data(data.begin(), data.end());

        return s_data;
    }

    /**
    std::vector<unsigned char> Uart::readUartBinaryByteCount() const {

        fd_set fd;
        if (!this->conf.blocking) {
            FD_ZERO(&fd);
            FD_SET(this->uartFilestream, &fd);
        }

        std::vector<unsigned char> data = {};

        if (this->uartFilestream > 0) {

            unsigned char rx_buffer[BUFFER_LEN];

            // clear buffer
            memset(&rx_buffer[0], 0, sizeof(rx_buffer));

            int rx_length = (int) read(this->uartFilestream, (void *) rx_buffer, BUFFER_LEN - 1);        //Filestream, buffer to store in, number of bytes to read (max)
            if (rx_length < 0) {
                if (errno == 11) {
                    // ms_timeout_handshake
                    std::this_thread::sleep_for(std::chrono::microseconds(this->conf.usec_timeout_serial));
                    return {};
                }
                std::cout << "An error occured, rx_length " << rx_length << std::endl;
                std::cout << "errno: " << errno << std::endl;
                //exit(1);
            } else {
                data = std::vector<unsigned char>(rx_buffer, rx_buffer + rx_length);
            }
        }

        return data;
    }
    */

    bool Uart::writeUart(std::string data) const {

        int char_count = data.size() + 2;
        char const *c_data = data.c_str();

        std::vector<unsigned char> v_data = std::vector<unsigned char>(c_data, c_data + data.size());

        v_data.emplace_back('\n');
        v_data.emplace_back('\r');
        v_data.emplace_back('\0');

        return this->writeUartBinary(v_data);
    }

    bool Uart::writeUartBinary(std::vector<unsigned char> data) const {

        int char_count = (int) data.size();
        unsigned char cmd[char_count];
        copy(data.begin(), data.end(), cmd);

        int n_written = 0;
        while (n_written < char_count) {
            int toWrite = char_count - n_written;
            if (toWrite > BUFFER_LEN) {
                toWrite = BUFFER_LEN;
            }
            n_written += (int) write(this->uartFilestream, cmd + n_written, toWrite);

        }
        if (n_written == -1) {
            return false;
        } else if (n_written == char_count) {
            return true;
        }

        return false;
    }

    void Uart::flushWrite() const {
        ioctl(this->uartFilestream, TCFLSH, 1); // flush transmit
    }

    void Uart::flushRead() const {
        ioctl(this->uartFilestream, TCFLSH, 0); // flush receive
    }

    void Uart::flushAll() const {
        ioctl(this->uartFilestream, TCFLSH, 2); // flush receive
    }

    bool Uart::handshake(bool active) {
        return this->handshake(active, this->conf.ms_timeout_handshake);
    }

    bool Uart::handshake(bool active, uint32_t timeout_ms) {

        // Recording the timestamp at the start of the code
        auto beg = std::chrono::high_resolution_clock::now();

        if (active) {
            this->writeUartBinary(this->conf.handshake_ping);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            while (!this->handshakeMatching(PONG, this->readUartBinary())) {
                auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - beg).count();
                if (timeout_ms < timeElapsed) {
                    return false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else {
            auto received = this->readUartBinary();
            while (!this->handshakeMatching(PING, received)) {
                for (const auto &item: received) {
                    std::cout << item;
                }
                if (!received.empty()) std::cout << std::endl;

                auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - beg).count();
                if (timeout_ms < timeElapsed) {
                    return false;
                }
                received = this->readUartBinary();
            }
            this->writeUartBinary(this->conf.handshake_pong);
        }

        return true;
    }

    bool Uart::handshakeMatching(bool with, std::vector<unsigned char> handshake) {
        std::vector<unsigned char> toCompare = (with) ? this->conf.handshake_ping : this->conf.handshake_pong;

        size_t handshakeSize = handshake.size();
        if (handshakeSize != toCompare.size()) {
            return false;
        }
        for (int i = 0; i < handshakeSize; ++i) {
            if (handshake[i] != toCompare[i]) {
                return false;
            }
        }

        return true;
    }

    std::vector<unsigned char> Uart::readUartBinary(char until, uint32_t timeout_ms) const {

        fd_set fd;
        if (!this->conf.blocking) {
            FD_ZERO(&fd);
            FD_SET(this->uartFilestream, &fd);
        }

        uint16_t buffer_len = BUFFER_LEN - 1;
        if (until != 0x00) {
            buffer_len = 1;
        }

        std::vector<unsigned char> data = {};
        auto lastTansmit = std::chrono::high_resolution_clock::now();

        while (true) {

            if (this->uartFilestream > 0) {

                unsigned char rx_buffer[BUFFER_LEN];

                // clear buffer
                memset(&rx_buffer[0], 0, sizeof(rx_buffer));

                int rx_length = (int) read(this->uartFilestream, (void *) rx_buffer, buffer_len);        //Filestream, buffer to store in, number of bytes to read (max)

                if (rx_length == 0) {
                    auto elapsed = lastTansmit - std::chrono::high_resolution_clock::now();
                    if (elapsed.count() > timeout_ms * 1000) {
                        // TODO Warnung / Log eintrag
                        break;
                    }
                } else if (rx_length < 0) {
                    if (errno == 11) {
                        // ms_timeout_handshake
                        std::this_thread::sleep_for(std::chrono::microseconds(this->conf.usec_timeout_serial));
                        return {};
                    }
                    std::cout << "An error occured, rx_length " << rx_length << std::endl;
                    std::cout << "errno: " << errno << std::endl;

                } else {
                    auto tmp = std::vector<unsigned char>(rx_buffer, rx_buffer + rx_length);
                    data.insert(data.end(), tmp.begin(), tmp.end());
                    lastTansmit = std::chrono::high_resolution_clock::now();

                    if (until != 0x00 && data.back() == until) {
                        break;
                    }
                }
            } else {
                break;
            }
            if (until == 0x00) {
                break;
            }
        }

        return data;
    }

    std::vector<unsigned char> Uart::readUartBinaryByteCount(uint32_t bytes, uint32_t timeout_ms) const {

        fd_set fd;
        if (!this->conf.blocking) {
            FD_ZERO(&fd);
            FD_SET(this->uartFilestream, &fd);
        }

        uint16_t buffer_len = BUFFER_LEN - 1;

        std::vector<unsigned char> data = {};
        auto lastTansmit = std::chrono::high_resolution_clock::now();

        while (true) {

            if (this->uartFilestream > 0) {

                unsigned char rx_buffer[BUFFER_LEN];

                // clear buffer
                memset(&rx_buffer[0], 0, sizeof(rx_buffer));

                uint16_t bytes_to_read = (bytes > buffer_len) ? buffer_len : bytes % buffer_len;

                int rx_length = (int) read(this->uartFilestream, (void *) rx_buffer, bytes_to_read); //Filestream, buffer to store in, number of bytes to read (max)

                bytes = bytes - rx_length;

                if (rx_length == 0) {
                    auto elapsed = lastTansmit - std::chrono::high_resolution_clock::now();
                    if (elapsed.count() > timeout_ms * 1000) {
                        // TODO Warnung / Log eintrag
                        break;
                    }
                } else if (rx_length < 0) {
                    if (errno == 11) {
                        // ms_timeout_handshake
                        std::this_thread::sleep_for(std::chrono::microseconds(this->conf.usec_timeout_serial));
                        return {};
                    }
                    std::cout << "An error occured, rx_length " << rx_length << std::endl;
                    std::cout << "errno: " << errno << std::endl;

                } else {
                    auto tmp = std::vector<unsigned char>(rx_buffer, rx_buffer + rx_length);
                    data.insert(data.end(), tmp.begin(), tmp.end());
                    lastTansmit = std::chrono::high_resolution_clock::now();

                    if (bytes <= 0) {
                        break;
                    }
                }
            } else {
                break;
            }
        }

        return data;
    }
}