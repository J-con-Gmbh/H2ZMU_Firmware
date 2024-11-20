//
// Created by jriessner on 13.02.24.
//

#ifndef H2ZMU_V1_GUI_H
#define H2ZMU_V1_GUI_H

#include <map>
#include <vector>
#include <string>
#include <memory>

struct screen_wrapper;

class Gui;

typedef void (*screen_action) (Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper);

struct line {
    std::string content;

    // Todo change default value to ""
    std::string bullet_point = " - ";
};

void shift_screen_up(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper);
void shift_screen_down(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper);
void shift_screen_left(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper);
void shift_screen_right(Gui *gui, const std::shared_ptr<struct screen_wrapper>& wrapper);

struct screen {
    int scroll = 0;
    int content_scroll = 0;

    std::string title;

    std::vector<struct line> content;

    screen_action btn_enter = nullptr;
    screen_action btn_esc = nullptr;
    screen_action btn_up = shift_screen_up;
    screen_action btn_down = shift_screen_down;
    screen_action btn_left = shift_screen_left;
    screen_action btn_right = shift_screen_right;
};

struct screen_wrapper {
    int index = 0;
    struct screen _screen;
    int wrapper_up = 0;
    int wrapper_down = 0;
    int wrapper_left = 0;
    int wrapper_right = 0;
};

enum gui_screen {
    MAIN = 1,
    MEASUREMENT = 2,
    ARCHIVE = 3,
    DIAGNOSIS = 4,
    STATUS_DATA = 5,
    ERRORS = 6,
    WARNINGS = 7
};

enum gui_icon {
    ALARM = 1,
    GAUGE = 2,
    SIGNAL = 3,
    LOCK_OPEN = 4,
    LOCK_CLOSED = 5,
    BLANK = 5
};

enum icon_state {
    ICON_ON,
    ICON_OFF,
    ICON_FLASHING
};

enum gui_direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE,
};

class Gui {
private:
    /// offset to make room for default screens ( see enum gui_screen )
    int wrapperIndex = 10;
    int activeScreen = 0;
    struct screen lastRenderScreen = {};

    std::map<int, std::shared_ptr<struct screen_wrapper>> screens;
    std::map<enum gui_icon, enum icon_state> icons_state {{ALARM, ICON_OFF}, {GAUGE, ICON_OFF}, {SIGNAL, ICON_OFF}, {LOCK_OPEN, ICON_OFF}, {LOCK_CLOSED, ICON_OFF}, };
    std::map<enum gui_icon, bool> icon_render_state {{ALARM, false}, {GAUGE, false}, {SIGNAL, false}, {LOCK_OPEN, false}, {LOCK_CLOSED, false}, };

    uint pSensorSub = -1;
    uint tSensorSub = -1;


    void addScreenWrapper(screen_wrapper screenWrapper, int index);

    void render();

public:

    static std::shared_ptr<Gui> instance;
    void init();
    int controlInterface = -1;

    std::shared_ptr<screen_wrapper> getScreenWrapper(int wrapper_index);
    void setActiveWrapper(int wrapper_index);
    void update();
    bool updateButtons();
    bool setErrorIcon();

    int addScreenWrapper(screen_wrapper screenWrapper);
    void setScreenLine(int screen_index, int line_index, std::string content);
    void setIconState(enum gui_icon icon, enum icon_state state);
    enum icon_state getIconState(enum gui_icon icon);

    void pushMeasurement(int id);
    void pushError(int id);
    void pushWarning(int id);
    struct screen_wrapper msrmntToGuiWrapper(const struct measurement& msrmnt);
    struct screen_wrapper errorToGuiWrapper(const struct occurrederror& error);
    struct screen_wrapper warningToGuiWrapper(const struct occurredwarning& warning);

    void updateMainScreen();
};

#endif //H2ZMU_V1_GUI_H
