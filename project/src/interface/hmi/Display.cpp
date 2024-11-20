//
// Created by ltudayakumar on 23.10.2024.
//
#include <chrono>

#include <iostream>
#include "interface/hmi/Display.h"
#include "interface/hmi/Gui.h"
#include "interface/modules/rs/AXL_SE_RS.h"

#include <thread>

#define TIME_SLEEP 50

AXL_SE_RS serial_1("Arp.Plc.Eclr/RS232serial1In", "Arp.Plc.Eclr/RS232serial1Out");

std::shared_ptr<Display> Display::instance;

int Display::initDisplay() 
{
    Display::clearFullDisplay();
    Display::setFont();

    return 1;
}

void Display::clearFullDisplay() {
    std::vector<uint8_t> data = { 'D', (this->inverted) ? 'S' : 'L' };
    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
    }
}

void Display::clearMainScreen() {
    //
    std::vector<uint8_t> data =  {(inverted) ? 'S' : 'L' , 0, 0, 213, 128 };
    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
    }
}

void Display::clearIndicatorScreen()
{
    std::vector<uint8_t> data =  { 'S', 213, 0, 240, 128 };
    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
    }
}

 
void Display::writeString(std::string message, uint8_t x, uint8_t line) {
    uint8_t y_position = (31 * line) + 1; // to divide the display into 5 lines eg: y_position = (space for 1 line * line number) + starting pixel of line 1 )
    std::vector<uint8_t> data =  { 'Z', x, y_position };

#define MAX_LENGTH_GUI_LINE 26
    std::vector<uint8_t> vec(message.begin(), message.end());
    data.insert(data.end(), vec.begin(), (vec.size() > MAX_LENGTH_GUI_LINE) ? vec.begin() + MAX_LENGTH_GUI_LINE : vec.end());
    data.emplace_back(0x00);

    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
    }
}

void Display::drawBitmap(gui_icon image) {

    auto item_icon = this->icons[image];
    uint8_t line = std::get<int>(item_icon);
    std::vector<uint8_t> picture = std::get<std::vector<uint8_t>>(item_icon);;

    uint8_t y_position = (line*31) + 1;
    std::vector<uint8_t> data = { 'U', 215, y_position, 24, 24 };

    data.reserve(picture.size());
    data.insert(data.end(), picture.begin(), picture.end());

    std::vector<uint8_t> inverse = { 'I', 215, y_position, 215 + 23, (uint8_t) (y_position + 23) };
    if (!inverted) data.insert(data.end(), inverse.begin(), inverse.end());

    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
    }

}

void Display::clearBitmapArea(gui_icon image) {

    auto item_icon = this->icons[image];
    uint8_t line = std::get<int>(item_icon);

    uint8_t y_position = (31*line)+1;
    std::vector<uint8_t> data = { 'L', 215, y_position, 215 + 24, (uint8_t) (y_position + 24) };

    if (this->cache) {
        this->cacheData.insert(this->cacheData.end(), data.begin(), data.end());
    } else {
        serial_1.sendData(data, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
    }

}


void Display::setFont()
{
    std::vector<uint8_t> cmdFont = { 'F', 3, 1, 2 };
    std::vector<uint8_t> cmdText = { 'T', 'R', 3, 1 };
    serial_1.sendData(cmdFont, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important

    serial_1.sendData(cmdText, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));//sleep(1); // this delay is important
}

void Display::getDateTime()
{
   time_t now = time(0);
   std::tm* localTime = std::localtime(&now);
   char formattedDateTime[20]; // Assuming a maximum length of 20 characters for the formatted time
   std::strftime(formattedDateTime, sizeof(formattedDateTime), "%d.%m.%y  %H:%M", localTime);
}

void Display::flushCached() {
    if (!this->cacheData.empty()) {
        serial_1.sendData(this->cacheData, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
        this->cacheData = {};
    }
}

void Display::setCaching(bool caching) {
    this->cache = caching;
}

void Display::setInverted(bool invert) {
    this->inverted = invert;
}
