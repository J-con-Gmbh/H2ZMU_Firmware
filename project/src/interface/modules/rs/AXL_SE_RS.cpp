//
// Created by jriessner on 17.12.23.
//

#include "interface/modules/rs/AXL_SE_RS.h"

#include "Plc/Gds/IDataAccessService.grpc.pb.h"
#include <grpcpp/security/credentials.h>
#include <bitset>
#include <thread>
#include <numeric>

const uint8_t AXL_SE_RS::ACTION_SEND = 0x00;
const uint8_t AXL_SE_RS::ACTION_READ = 0x01;

const uint8_t AXL_SE_RS::CMD_READ_REC_BUFFER_STATE = 0x00;
const uint8_t AXL_SE_RS::CMD_SEND_DATA = 0x10;
const uint8_t AXL_SE_RS::CMD_CACHE_DATA_TO_SEND = 0x20;
const uint8_t AXL_SE_RS::CMD_READ_CHARS_OR_COUNTER = 0x30;

const uint8_t AXL_SE_RS::CMD_TOGGLE_SEND_DATA = 0x50;
const uint8_t AXL_SE_RS::CMD_TOGGLE_CACHE_DATA_TO_SEND = 0x60;
const uint8_t AXL_SE_RS::CMD_TOGGLE_READ_DATA = 0x70;

const uint8_t AXL_SE_RS::CMD_READ_REC_BUFFER_COUNTER = 0x30;
const uint8_t AXL_SE_RS::CMD_READ_DATA = 0x30;
const uint8_t AXL_SE_RS::CMD_READ_COUNTER = 0x3E;
const uint8_t AXL_SE_RS::CMD_SAVE_FOR_SEND = 0x20;
const uint8_t AXL_SE_RS::CMD_SEND_DATA_17 = 0x10;

const uint8_t AXL_SE_RS::STS_TX_BUF_NOT_EMPTY  = 0b01000000;
const uint8_t AXL_SE_RS::STS_TX_BUF_FULL       = 0b00100000;
const uint8_t AXL_SE_RS::STS_RX_BUF_FULL       = 0b00010000;
const uint8_t AXL_SE_RS::STS_RX_BUF_NOT_EMPTY  = 0b00000001;

void printBinary(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        std::cout << ((value >> i) & 1);
    }
}

std::tuple<bool, uint8_t, std::vector<uint8_t>> AXL_SE_RS::receiveData() {
    //std::cout << "Start Recieve Data" << std::endl;
    
    std::vector<uint8_t> return_array;

    auto response = Arp::Plc::Gds::Services::Grpc::IDataAccessServiceReadResponse();
    int c = 100;

    // Schritt 1
    this->sendPayload(CMD_READ_REC_BUFFER_STATE, 0x00, 0x00, {});

    //Schritt 2
    while (!this->writeCmdOk(CMD_READ_REC_BUFFER_STATE, ACTION_READ) && MAX_IT(c)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    //std::cout << "Needed " << 100 - c << " tries" << std::endl;

    c = 100;
    bool toggle = false;
    uint8_t command = 0x00;
    // Read amount of data available
    auto buffer = this->input.readByteVector();
    if (!std::get<bool>(buffer)) {
        return {false, 0, {}};
    }

    uint8_t buffer_size = std::get<std::vector<uint8_t>>(buffer)[3];

    //std::cout << "buffer size: " << unsigned(buffer_size) << std::endl;
    // Interiere, solange buffer nicht empty
    while (return_array.size() < buffer_size) {
        
        if(!toggle)
        {
            command = CMD_READ_DATA;
        }
        else
        {
            command = CMD_TOGGLE_READ_DATA;
        }

        this->sendPayload(command, 0x00, 0x00, {});

        if(toggle)
        {
            toggle = false;
        }
        else
        {
            toggle = true;
        }

        //Schritt 2
        while (!this->writeCmdOk(command, ACTION_READ) && MAX_IT(c)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        //Schritt 5
        uint8_t indicator = 0;
        uint8_t size = 0;
        auto tuple_res = this->input.readByteVector();
        if (!std::get<bool>(tuple_res)) {
            return {false, 0, {}};
        }
        
        //std::cout << "Recieve data raw: " << std::endl;
        //Iteriere durch den Array
        for (auto it: std::get<std::vector<uint8_t>>(tuple_res))
        {
            //std::cout << it;
            if (size != 0 && indicator > (size + 2))
            {
                break;
            }
            if (indicator == 2)
            {
                size = it;
            }

            if (indicator >= 3)
            {
                return_array.emplace_back(it);
            }

            indicator += 1;
        }

        // Print 
        //std::cout << std::endl << "Recieve data size: " << unsigned(size) << std::endl;
        //std::cout << "Recieve array size: " << return_array.size() << std::endl;
        //std::cout << "Recieve data value: " << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    //std::cout << std::endl << "END RECIEVE" << std::endl;
    return {true, 0, return_array};
}



// Receive memory  Transmit memory
// 4096 bytes      1023 bytes

std::tuple<bool, uint16_t> AXL_SE_RS::sendData(std::vector<uint8_t> data, bool validate) {
    //std::cout << "Start Send Data" << std::endl;

    // Schritt 2
    int c = 100;

    // Schritt 1
    this->sendPayload(CMD_READ_REC_BUFFER_STATE, 0x00, 0x00, {});
    
    while (!this->writeCmdOk(CMD_READ_REC_BUFFER_STATE, ACTION_SEND) && MAX_IT(c)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    //std::cout << "Needed " << 100 - c << " tries" << std::endl;

    // Print 
    //std::cout << "Send value data size: " << data.size() << std::endl;
    //std::cout << "Send data value: ";

    //std::cout << " " << std::endl;

    bool toggle = false;
    // Falls vom Buffer aus versandt werden soll.
    while (data.size() > 17) {
        auto part = std::vector<uint8_t>(data.begin(), data.begin() + 17);
        bool success = this->cacheData(part, toggle);
        if (!success) {
            std::cout << "Error Caching bytes" << std::endl;
        }
        //std::cout << "Caching " << part.size() << " bytes of data" << std::endl;
        //std::cout << "Caching data value: ";

        //std::cout << " " << std::endl;
        data.erase(data.begin(), data.begin() + 17);

        if(toggle)
        {
            toggle = false;
        }
        else
        {
            toggle = true;
        }
    }

    // Schritt 3 in äußerer funktion
    // Schritt 4 und 5
    std::tuple<bool, uint16_t> ret = this->sendPayload(CMD_SEND_DATA, 0x00, (uint8_t) data.size(), data);


    //std::cout << "Sending " << data.size() << " bytes of data" << std::endl;
    //std::cout << "Sending data value: ";

    //std::cout << " " << std::endl;

    // Schritt 6 und 7
    if (validate) {
        c = 100;
        while (!this->writeCmdOk(CMD_SEND_DATA, ACTION_SEND) && MAX_IT(c)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    //std::cout << "Needed " << 100 - c << " tries" << std::endl;

    //std::cout << "END SEND" << std::endl;

    return ret;
}

bool AXL_SE_RS::dataPending() {
    return false;
}

bool AXL_SE_RS::cacheData(std::vector<uint8_t> data, bool toggle) {
    uint8_t command = 0x00;

    if(!toggle)
    {
        command = CMD_CACHE_DATA_TO_SEND;
    }
    else
    {
        command = CMD_TOGGLE_CACHE_DATA_TO_SEND;
    }
    
    uint8_t size = data.size();
    data.insert(data.begin(), {command, 0x00, size});

    int c = 100;

    auto ret = this->output.writeByteVector(data);
    while (!this->writeCmdOk(command, ACTION_SEND) && MAX_IT(c)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return true;
}

bool AXL_SE_RS::writeCmdOk(uint8_t cmd, uint8_t action) {

    std::tuple<bool, std::vector<uint8_t>> handshake;
    handshake = this->input.readByteVector();

    if (    std::get<bool>(handshake)
        && !std::get<std::vector<uint8_t>>(handshake).empty()
        &&  std::get<std::vector<uint8_t>>(handshake)[0] == cmd)
    {   
        if(cmd == CMD_SEND_DATA)
        {
            if(!this->input.Tx_buf_not_empty())
            {
                //std::cout << "Send data succ"<< std::endl;
                return true;
            }
            else
            {
                //std::cout << "Transmission not completed yet"<< std::endl;
                return false;
            }
        }
        else if(cmd == CMD_READ_DATA)
        {
            //std::cout << "Read data succ"<< std::endl;
            return true;
        }
        else if(cmd == CMD_READ_REC_BUFFER_STATE)
        {
            if(std::get<std::vector<uint8_t>>(handshake)[1] == 0x01 && action == ACTION_READ)
            {
                //std::cout << "Read buffer succ"<< std::endl;
                return true;
            }
            else if(std::get<std::vector<uint8_t>>(handshake)[1] == 0x00 && action == ACTION_SEND)
            {
                //std::cout << "Send buffer succ"<< std::endl;
                return true;
            }
            else
            {  
                if(action == ACTION_SEND && std::get<std::vector<uint8_t>>(handshake)[1] == 0x40)
                {
                    //std::cout << "Transmit buffer not empty"<< std::endl;            
                }
                else if(action == ACTION_READ)
                {
                    //std::cout << "Recieve buffer empty"<< std::endl;
                }
                else
                {
                    std::cout << "Unknown Action"<< std::endl;
                }

                return false;
            }
        }
        else if(cmd == CMD_CACHE_DATA_TO_SEND)
        {
            //std::cout << "Data cached succ"<< std::endl;
            return true;
        }
        else if(cmd == CMD_TOGGLE_CACHE_DATA_TO_SEND)
        {
            //std::cout << "Data (toggle) cached succ"<< std::endl;
            return true;
        }
        else if(cmd == CMD_TOGGLE_READ_DATA)
        {
            //std::cout << "Data (toggle) read succ"<< std::endl;
            return true;
        }
        else
        {
            //std::cout << "Wrong command"<< std::endl;
            return false;
        }
    }

    return false;
}

bool AXL_SE_RS::decrTillZero(int &counter) {
    if (counter == 0) {
        return false;
    }
    counter -= 1;

    return true;
}

std::tuple<bool, uint16_t> AXL_SE_RS::sendPayload(uint8_t one, uint8_t two, uint8_t three, std::vector<uint8_t> data) {

    if (data.size() > 17) {
        return {false, 0x00};
    }

    data.insert(data.begin(), {one, two, three});

    while (data.size() < 20)
        data.emplace_back(0x00);

    auto ret = this->output.writeByteVector(data);

    return ret;
}



























