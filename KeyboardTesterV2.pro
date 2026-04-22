QT       += core gui widgets

CONFIG += c++17

TARGET   = KeyboardTesterV2
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    KeyboardGrid.cpp \
    LayoutConfig.cpp \
    DeviceManager.cpp \
    KeyHook.cpp \
    OpenAgreementHID/OHID/OHID_Pack.c \
    OpenAgreementHID/OHID/OHID_Port.c \
    OpenAgreementHID/OHID/OHID_KeyBoard.c \
    Layout/OHID_Layout.c \
    hid/InterfaceHID.cpp

HEADERS += \
    MainWindow.h \
    KeyboardGrid.h \
    LayoutConfig.h \
    DeviceManager.h \
    KeyHook.h \
    OpenAgreementHID/OHID.h \
    OpenAgreementHID/OHID/OHID_Board.h \
    OpenAgreementHID/OHID/OHID_Port.h \
    OpenAgreementHID/OHID/OHID_KeyBoard.h \
    OpenAgreementHID/OHID/OHID_type_keyboard.h \
    Layout/OHID_Layout.h \
    Layout/HID_Usage_Tables.h \
    hid/InterfaceHID.h

DISTFILES += config.json

INCLUDEPATH += $$_PRO_FILE_PWD_/OpenAgreementHID
INCLUDEPATH += $$_PRO_FILE_PWD_/Library

win32: LIBS += -L$$_PRO_FILE_PWD_/Library/hidapi/win/x64 -lhidapi

# ---------- Post-Build: copy config.json + hidapi.dll next to exe ----------
win32 {
    CONFIG(debug, debug|release) {
        OUT_DIR = $$OUT_PWD/debug
    } else {
        OUT_DIR = $$OUT_PWD/release
    }

    CFG_SRC = $$PWD/config.json
    CFG_DST = $$OUT_DIR/config.json
    CFG_SRC ~= s,/,\\,g
    CFG_DST ~= s,/,\\,g

    DLL_SRC = $$PWD/Library/hidapi/win/x64/hidapi.dll
    DLL_DST = $$OUT_DIR/hidapi.dll
    DLL_SRC ~= s,/,\\,g
    DLL_DST ~= s,/,\\,g

    QMAKE_POST_LINK += copy /y \"$$CFG_SRC\" \"$$CFG_DST\"
    QMAKE_POST_LINK += $$escape_expand(\n\t)
    QMAKE_POST_LINK += copy /y \"$$DLL_SRC\" \"$$DLL_DST\"
}
