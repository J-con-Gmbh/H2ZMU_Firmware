//
// Created by jriessner on 13.02.24.
//

#include "interface/hmi/Gui.h"

#include <utility>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "interface/hmi/Button.h"
#include "interface/hmi/Display.h"

#include "interface/modules/dio16/AXL_SE_DO16.h"

#include "Core.h"
#include "rgfc/RgfCorr.h"
#include "epoch_time.h"
#include "data/db/repositories/OccurredErrorRepository.h"
#include "data/db/repositories/OccurredWarningRepository.h"
#include "data/db/repositories/ErrorRepository.h"
#include "data/db/repositories/WarningRepository.h"

bool operator==(const struct screen& lhs, const struct screen& rhs)
{
    if (lhs.scroll != rhs.scroll) return false;
    if (lhs.content_scroll != rhs.content_scroll) return false;
    if (lhs.title != rhs.title) return false;
    if (lhs.content.size() != rhs.content.size()) return false;
    for (int i = 0; i < lhs.content.size(); ++i) { 
        if (lhs.content[i].content != rhs.content[i].content) return false;
        if (lhs.content[i].bullet_point != rhs.content[i].bullet_point) return false;
    }

    return true;
}

std::shared_ptr<Gui> Gui::instance;

AXL_SE_DO16 digitalOut1("Arp.Plc.Eclr/wDO16");

void Gui::init() {
    this->pSensorSub = queue::Queues::instance->pressureSensorData.subscribe();
    this->tSensorSub = queue::Queues::instance->temperatureSensorData.subscribe();

    this->controlInterface = Core::instance->control.registerInterface(10);
    screen_wrapper main;
    main.wrapper_right = gui_screen::ERRORS;
    main.index = gui_screen::MAIN;

    main._screen = {};
    main._screen.title = "H2ZMU Real Time Data";
    main._screen.content = {};
    this->screens[gui_screen::MAIN] = std::make_shared<struct screen_wrapper>(main);

    auto lastMsrmnts = MeasurementRepository::instance->getLastMeasurementsByCount(10);
        
    for (const auto &item: lastMsrmnts) {
        this->addScreenWrapper(this->msrmntToGuiWrapper(item));
    }

    screen_wrapper errors;
    errors.index = gui_screen::ERRORS;
    errors._screen = {};
    errors.wrapper_left = gui_screen::MAIN;
    errors.wrapper_right = gui_screen::WARNINGS;
    errors._screen.title = "Errors (Press Enter)";

    this->screens[gui_screen::ERRORS] = std::make_shared<struct screen_wrapper>(errors);

    auto lastErrors = OccurredErrorRepository::instance->getLastOccurredErrorsByCount(10);

    for (const auto &item: lastErrors) {
        this->addScreenWrapper(this->errorToGuiWrapper(item));
    }

    screen_wrapper warnings;
    warnings.wrapper_left = gui_screen::ERRORS;
    warnings.wrapper_right = gui_screen::MAIN;

    warnings._screen = {};
    warnings._screen.title = "Warnings (Press Enter)";

    this->screens[gui_screen::WARNINGS] = std::make_shared<struct screen_wrapper>(warnings);

    auto lastWarnings = OccurredWarningRepository::instance->getLastOccurredWarningsByCount(10);

    for (const auto &item: lastWarnings) {
        this->addScreenWrapper(this->warningToGuiWrapper(item));
    }

    /*
    
    int index = this->addScreenWrapper(arch);

    arch.wrapper_up = index;
    arch._screen.title = "Errors:";
    arch._screen.content = {{"Error 1"}, {"Error 2"}, {"Error 3"}, {"Error 4"}};

    index = this->addScreenWrapper(arch);

    arch.wrapper_up = index;
    arch._screen.title = "Warnings:";
    arch._screen.content = {{"Warning 1"}, {"Warning 2"}, {"Warning 3"}, {"Warning 4"}};

    index = this->addScreenWrapper(arch);
    */
    
}

int Gui::addScreenWrapper(screen_wrapper screenWrapper) {

    this->addScreenWrapper(std::move(screenWrapper), this->wrapperIndex);
    this->wrapperIndex += 1;

    return (this->wrapperIndex - 1);
}

void Gui::addScreenWrapper(screen_wrapper screenWrapper, int index) {

    screenWrapper.index = index;

    if (screenWrapper.wrapper_up != 0 && this->screens.count(screenWrapper.wrapper_up)) {
        this->screens[screenWrapper.wrapper_up]->wrapper_down = index;
    }
    if (screenWrapper.wrapper_down != 0 && this->screens.count(screenWrapper.wrapper_down)) {
        this->screens[screenWrapper.wrapper_down]->wrapper_up = index;
    }

    // Not sure if this is working with current gui concept
/*    if (screenWrapper.wrapper_left != 0 && this->screens.count(screenWrapper.wrapper_left)) {
        this->screens[screenWrapper.wrapper_left]->wrapper_right = index;
    }
    if (screenWrapper.wrapper_right != 0 && this->screens.count(screenWrapper.wrapper_right)) {
        this->screens[screenWrapper.wrapper_right]->wrapper_left = index;
    }
*/
    this->screens.insert({index, std::make_shared<struct screen_wrapper>(screenWrapper)});


}


std::shared_ptr<screen_wrapper> Gui::getScreenWrapper(int wrapper_index) {

    if (this->screens.count(wrapper_index)) {
        return this->screens[wrapper_index];
    }
    return {};
}

void Gui::render() {

    auto screen_wrapper = this->getScreenWrapper(this->activeScreen);
    auto screen = screen_wrapper->_screen;

    uint8_t line = 1;
    if (!(this->lastRenderScreen == screen)) {
        Display::instance->clearMainScreen();

        Display::instance->writeString(screen.title, 1, 0);

        for (int i = screen.content_scroll; i < screen.content.size(); ++i) {
            Display::instance->writeString(screen.content[i].content, 1, line);
            line++;
        }

        Display::instance->flushCached();

        this->lastRenderScreen = screen;
    }
    for (const auto &item: this->icons_state) {

        enum gui_icon icon = item.first;
        enum icon_state state = item.second;

        if (state == icon_state::ICON_FLASHING) {
            icon_render_state[icon] = !icon_render_state[icon];
        } else if ((state == icon_state::ICON_ON) == icon_render_state[icon] ) {
            continue;
        }

        if (state != icon_state::ICON_FLASHING) {
            icon_render_state[icon] = (state == icon_state::ICON_ON);
        }

        if (icon_render_state[icon]) {
            Display::instance->drawBitmap(icon);
        } else {
            Display::instance->clearBitmapArea(icon);
        }

        Display::instance->flushCached();
    }
}

bool Gui::setErrorIcon() {
    auto lastErrors = OccurredErrorRepository::instance->getLastOccurredErrorsByCount(10);
    bool hasUnresolvedError = false;
    for (const auto &item: lastErrors) {
        if(item.resolved_timestamp != "")
        {
            hasUnresolvedError= true;
        }
    }

    if(hasUnresolvedError)
    {
        digitalOut1.setStateOfBit(2, true);
        digitalOut1.setStateOfBit(0, false);
        digitalOut1.setStateOfBit(4, true);
        this->setIconState(gui_icon::ALARM, icon_state::ICON_ON);
    }
    else
    {
        digitalOut1.setStateOfBit(2, false);
        digitalOut1.setStateOfBit(0, true);
        digitalOut1.setStateOfBit(4, false);
        this->setIconState(gui_icon::ALARM, icon_state::ICON_OFF);
    }

    return hasUnresolvedError;
}


void Gui::update() {

    if (this->activeScreen == 0) {
        this->activeScreen = 1;
    }

    this->updateButtons();
    this->updateMainScreen();
    this->setErrorIcon();
    this->render();

}

void Gui::setActiveWrapper (int wrapper_index) {
    if (this->screens.count(wrapper_index)) {
        this->activeScreen = wrapper_index;
    }
}


bool Gui::updateButtons() {

    std::vector<enum button> buttons_pressed;

    auto screen_wrapper = this->getScreenWrapper(this->activeScreen);

    buttons_pressed = Button::instance->getPressed();

    if(!buttons_pressed.empty()) {
        switch (buttons_pressed.front()){
            case BUTTON_UP : {
                std::cout << "BUTTON UP" << std::endl;
                if (screen_wrapper->_screen.content_scroll > 0) {
                    screen_wrapper->_screen.content_scroll--;
                }
            } break;
            case BUTTON_DOWN : {
                std::cout << "BUTTON DOWN" << std::endl;
                if (screen_wrapper->_screen.content_scroll < screen_wrapper->_screen.content.size())
                    screen_wrapper->_screen.content_scroll++;
            }break;
            case BUTTON_RIGHT : {
                std::cout << "BUTTON RIGHT" << std::endl;
                screen_wrapper->_screen.btn_right(this, screen_wrapper);
            } break;
            case BUTTON_LEFT : {
                std::cout << "BUTTON LEFT" << std::endl;
                screen_wrapper->_screen.btn_left(this, screen_wrapper);
            } break;
            case BUTTON_ENTER : {
                std::cout << "BUTTON ENTER" << std::endl;
                screen_wrapper->_screen.btn_down(this, screen_wrapper);
            } break;
            case BUTTON_ESCAPE : {
                std::cout << "BUTTON ESCAPE" << std::endl;
                screen_wrapper->_screen.btn_up(this, screen_wrapper);
            } break;
            case MEASURE_START : {
                std::cout << "BUTTON START" << std::endl;
                if(!Core::instance->getBlockMeas())
                {
                    Core::instance->control.claimControl(this->controlInterface);
                    this->setIconState(gui_icon::GAUGE, icon_state::ICON_ON);
                    digitalOut1.setStateOfBit(1, true);
                    Core::instance->control.send(this->controlInterface, core::control::action::MEASUREMENT_START);
                }
            } break;
            case MEASURE_STOP : {
                std::cout << "BUTTON STOP" << std::endl;
                this->setIconState(gui_icon::GAUGE, icon_state::ICON_OFF);
                digitalOut1.setStateOfBit(1, false);
                Core::instance->control.send(this->controlInterface, core::control::action::MEASUREMENT_STOP);
                Core::instance->control.releaseControl(this->controlInterface);
            } break;
            case SERVICE_SWITCH : {

            } break;
            case CALIBRATION_SWITCH : {

            } break;
            default:
                break;
        }

        return true;
    } else {
        return false;
    }

}

void Gui::setScreenLine(int screen_index, int line_index, std::string content) {
    auto screen_wrapper = this->getScreenWrapper(screen_index);
    if (!screen_wrapper) {
        // return if wrapper with this index not exists -> shared_ptr is invalid
        return;
    }
    for (size_t i = screen_wrapper->_screen.content.size(); i <= line_index; ++i) {
        screen_wrapper->_screen.content.emplace_back(line {"", ""});
    }

    screen_wrapper->_screen.content[line_index].content = std::move(content);

}

void Gui::setIconState(enum gui_icon icon, enum icon_state state) {
    if (!this->icons_state.count(icon)) {
        return;
    }
    this->icons_state[icon] = state;
}

enum icon_state Gui::getIconState(enum gui_icon icon) {
    if (!this->icons_state.count(icon)) {
        return icon_state::ICON_OFF;
    }

    return this->icons_state[icon];
}

void Gui::pushMeasurement(int id) {

    std::shared_ptr<screen_wrapper> wrapper_main = this->getScreenWrapper(gui_screen::MAIN);

    struct measurement msrmnt = MeasurementRepository::instance->getMeasurementById(id);
    screen_wrapper msrmnt_wrapper = this->msrmntToGuiWrapper(msrmnt);

    this->addScreenWrapper(msrmnt_wrapper);
}

void Gui::pushError(int id) {

    std::shared_ptr<screen_wrapper> wrapper_error = this->getScreenWrapper(gui_screen::ERRORS);

    struct occurrederror error = OccurredErrorRepository::instance->getOccurredErrorById(id);
    screen_wrapper error_wrapper = this->errorToGuiWrapper(error);

    this->addScreenWrapper(error_wrapper);
}

void Gui::pushWarning(int id) {
    
    std::shared_ptr<screen_wrapper> wrapper_warning = this->getScreenWrapper(gui_screen::WARNINGS);

    struct occurredwarning warning = OccurredWarningRepository::instance->getOccurredWarningById(id);
    screen_wrapper warning_wrapper = this->warningToGuiWrapper(warning);

    this->addScreenWrapper(warning_wrapper);
}

struct screen_wrapper Gui::msrmntToGuiWrapper(const struct measurement& msrmnt) {

    std::shared_ptr<screen_wrapper> wrapper_main = this->getScreenWrapper(gui_screen::MAIN);

    float nm3 = msrmnt.amountStartNm3 - msrmnt.amountEndNm3;

    std::ostringstream nm3_value_stream;
    nm3_value_stream << std::fixed << std::setprecision(2) << nm3;
    std::string nm3_value_str = nm3_value_stream.str();

    float kg = RgfCorr::instance->convertNormVolToKg(nm3);

    std::ostringstream kg_value_stream;
    kg_value_stream << std::fixed << std::setprecision(2) << kg;
    std::string kg_value_str = kg_value_stream.str();

    time_t duration_t = msrmnt.timestamp_end - msrmnt.timestamp_start;

    char time_cstr_start[26];
    strftime(time_cstr_start, 26, "%d-%m-%Y %H:%M:%S", localtime(&msrmnt.timestamp_start));

    char time_cstr_end[26];
    strftime(time_cstr_end, 26, "%d-%m-%Y %H:%M:%S", localtime(&msrmnt.timestamp_end));

    char time_cstr_duration[10];
    strftime(time_cstr_duration, 10, "%H:%M:%S", localtime(&duration_t));

    std::string mnr_str = "";

    if(msrmnt.id < 10)
    {
        mnr_str = "00" + std::to_string(msrmnt.id);
    }
    else if(msrmnt.id < 100)
    {
        mnr_str = "0" + std::to_string(msrmnt.id);
    }

    screen_wrapper msrmnt_wrapper;
    msrmnt_wrapper.wrapper_up = gui_screen::MAIN;
    msrmnt_wrapper.wrapper_down = wrapper_main->wrapper_down;
    msrmnt_wrapper.wrapper_right = gui_screen::ARCHIVE;

    msrmnt_wrapper._screen = {};
    msrmnt_wrapper._screen.title = "MNR    " + mnr_str;
    msrmnt_wrapper._screen.content = {
            {"Nm3  : " + nm3_value_str},
            {"Kg   : " + kg_value_str},
            {"Start: " + std::string(time_cstr_start)},
            {"Ende : " + std::string(time_cstr_end)},
            {"Dauer: " + std::string(time_cstr_duration)},
    };

    return msrmnt_wrapper;
}

struct screen_wrapper Gui::errorToGuiWrapper(const struct occurrederror& error) {

    std::shared_ptr<screen_wrapper> wrapper_error = this->getScreenWrapper(gui_screen::ERRORS);

    std::string timestamp = error.occurred_timestamp;
    std::string resolved = error.resolved_timestamp;
    std::string errCode = ErrorRepository::instance->getErrorById(error.fk_error).errCode;

    screen_wrapper error_wrapper;
    error_wrapper.wrapper_up = gui_screen::ERRORS;
    error_wrapper.wrapper_down = wrapper_error->wrapper_down;
    error_wrapper.wrapper_right = gui_screen::WARNINGS;

    error_wrapper._screen = {};
    error_wrapper._screen.title = "Error ID " + std::to_string(error.id);
    error_wrapper._screen.content = {
            {"Timestamp  : " + timestamp },
            {"Error Code : " + errCode },
            {"Resolved   : " + resolved },
    };

    return error_wrapper;
}

struct screen_wrapper Gui::warningToGuiWrapper(const struct occurredwarning& warning) {

    std::shared_ptr<screen_wrapper> wrapper_warning = this->getScreenWrapper(gui_screen::WARNINGS);

    std::string timestamp = warning.occurred_timestamp;
    std::string resolved = warning.resolved_timestamp;
    std::string warnCode = WarningRepository::instance->getWarningById(warning.fk_warning).warnCode;

    screen_wrapper warning_wrapper;
    warning_wrapper.wrapper_up = gui_screen::WARNINGS;
    warning_wrapper.wrapper_down = wrapper_warning->wrapper_down;
    warning_wrapper.wrapper_right = gui_screen::MAIN;

    warning_wrapper._screen = {};
    warning_wrapper._screen.title = "Warning ID " + std::to_string(warning.id);
    warning_wrapper._screen.content = {
            {"Timestamp    : " + timestamp },
            {"Warning Code : " + warnCode },
            {"Resolved   : " + resolved },
    };

    return warning_wrapper;
}

void Gui::updateMainScreen() {

    auto new_p_sensor_data = queue::Queues::instance->pressureSensorData.getNew(this->pSensorSub);
    auto new_t_sensor_data = queue::Queues::instance->temperatureSensorData.getNew(this->tSensorSub);
    std::string timestamp = utils::epoch_time::getTimestamp("%d.%m.%y   %T");
    this->setScreenLine(gui_screen::MAIN, 0, timestamp);

if (!new_p_sensor_data.empty()) {
    std::ostringstream p_value_stream;
    p_value_stream << std::fixed << std::setprecision(2) << new_p_sensor_data.back().value;
    std::string p_value_str = p_value_stream.str();
    this->setScreenLine(gui_screen::MAIN, 1, "Druck: " + p_value_str);
}

if (!new_t_sensor_data.empty()) {
    std::ostringstream t_value_stream;
    t_value_stream << std::fixed << std::setprecision(2) << new_t_sensor_data.back().value;
    std::string t_value_str = t_value_stream.str();
    this->setScreenLine(gui_screen::MAIN, 2, "Temp : " + t_value_str);
}
}

void shift_screen_up(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper) {
    gui->setActiveWrapper(wrapper->wrapper_up);
}

void shift_screen_down(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper) {
    gui->setActiveWrapper(wrapper->wrapper_down);
}

void shift_screen_left(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper) {
    gui->setActiveWrapper(wrapper->wrapper_left);
}

void shift_screen_right(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper) {
    gui->setActiveWrapper(wrapper->wrapper_right);
}
