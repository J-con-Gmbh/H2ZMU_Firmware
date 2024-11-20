#include <iostream>

#include <vector>
#include <algorithm>
#include <thread>

#include "lib/include/Uart.h"

class InputParser {
public:
    InputParser (int &argc, char **argv){
        for (int i=1; i < argc; ++i)
            this->tokens.emplace_back(argv[i]);
    }

    const std::string& getCmdOption(const std::string &option) const{
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end()){
            return *itr;
        }
        static const std::string empty_string;

        return empty_string;
    }

    bool cmdOptionExists(const std::string &option) const{
        return std::find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }

    const std::string& getLastOption() {
        return tokens.back();
    }

private:
    std::vector <std::string> tokens;
};

int main(int argc, char **argv) {

    InputParser input(argc, argv);

    bool reading = false;
    bool active = false;
    bool writing = false;
    bool bidirectional = false;
    bool keepWriting = true;
    std::string toWrite = "";

    if (input.cmdOptionExists("-r")) {
        reading = true;
    }

    if (input.cmdOptionExists("-a")) {
        active = true;
    }

    if (input.cmdOptionExists("-w")) {
        writing = true;
    }

    if (input.cmdOptionExists("-b")) {
        bidirectional = true;
    }

    if (input.cmdOptionExists("-1")) {
        keepWriting = false;
        toWrite = input.getCmdOption("-1");
    }

    std::cout << "Port: " << input.getLastOption() << std::endl;

    uart::Uart uart;
    uart.setup({
        .connection_point = input.getLastOption(),

    });
    uart.connect();
    uart.handshake(active);
    std::cout << "handshake complete" << std::endl;

    if (reading) {
        std::cout << "reading..." << std::endl;
        std::string data;
        do {
            data = uart.readUart();
            if (!data.empty()) {
                std::cout << data << std::flush;

                std::cout << data.size() << std::endl;
                data = uart::strip(data);
                std::cout << data.size() << std::endl;

                if (data == "recon") {
                    std::cout << "Disconnecting" << std::endl;
                    uart.disconnect();
                    std::cout << "Sleep for 5 seconds" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    std::cout << "Reconnecting" << std::endl;
                    uart.connect();
                }
            }
        } while (data != "exit");
    } else if (writing) {
        std::cout << "writing";
        if (!keepWriting) {
            std::cout << ": " << toWrite << std::endl;
            if (!uart.writeUart(toWrite)) {
                std::cout << "Error writing to Serial Port!" << std::endl;
            }
            uart.disconnect();
            return 0;
        } else {
            std::cout << ", type message and confirm with [ENTER]\ntype exit to leave"<< std::endl;
            std::string data;
            do {
                data = "";
                std::getline(std::cin, data);
                if (!uart.writeUart(data)) {
                }
            } while (data != "exit");
        }
    } else if (bidirectional) {
        std::string data;
        do {
            char cstr_input[10];
            std::cin.getline(cstr_input, 10);
            uart.writeUart(std::string(cstr_input));

            data = uart.readUart();
            if (!data.empty()) {
                std::cout << data << std::flush;

                std::cout << data.size() << std::endl;
                data = uart::strip(data);
                std::cout << data.size() << std::endl;
            }

        } while (data != "exit");
    }

    return 0;
}
