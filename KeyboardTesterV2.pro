QT       += core gui widgets network

CONFIG += c++17

TARGET   = KeyboardTesterV2
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    DeviceManager.cpp \
    KeyHook.cpp \
    src/KeyLayout.cpp \
    src/KeyboardView.cpp \
    src/ScanCodeMap.cpp \
    src/ApiClient.cpp \
    src/LoginDialog.cpp \
    OpenAgreementHID/OHID/OHID_Pack.c \
    OpenAgreementHID/OHID/OHID_Port.c \
    OpenAgreementHID/OHID/OHID_KeyBoard.c \
    Layout/OHID_Layout.c \
    hid/InterfaceHID.cpp

HEADERS += \
    MainWindow.h \
    DeviceManager.h \
    KeyHook.h \
    src/KeyLayout.h \
    src/KeyboardView.h \
    src/ScanCodeMap.h \
    src/ApiClient.h \
    src/LoginDialog.h \
    OpenAgreementHID/OHID.h \
    OpenAgreementHID/OHID/OHID_Board.h \
    OpenAgreementHID/OHID/OHID_Port.h \
    OpenAgreementHID/OHID/OHID_KeyBoard.h \
    OpenAgreementHID/OHID/OHID_type_keyboard.h \
    Layout/OHID_Layout.h \
    Layout/HID_Usage_Tables.h \
    hid/InterfaceHID.h

DISTFILES += \
    README.md \
    layouts/sample_75.json \
    layouts/sample_104.json

RESOURCES += \
    icon.qrc

INCLUDEPATH += $$_PRO_FILE_PWD_
INCLUDEPATH += $$_PRO_FILE_PWD_/src
INCLUDEPATH += $$_PRO_FILE_PWD_/OpenAgreementHID
INCLUDEPATH += $$_PRO_FILE_PWD_/Library

win32: LIBS += -L$$_PRO_FILE_PWD_/Library/hidapi/win/x64 -lhidapi
win32: LIBS += -luser32

# ---------- Post-Build: copy layouts/ + hidapi.dll next to exe ----------
win32 {
    CONFIG(debug, debug|release) {
        OUT_DIR = $$OUT_PWD/debug
    } else {
        OUT_DIR = $$OUT_PWD/release
    }

    DLL_SRC = $$PWD/Library/hidapi/win/x64/hidapi.dll
    DLL_DST = $$OUT_DIR/hidapi.dll
    DLL_SRC ~= s,/,\\,g
    DLL_DST ~= s,/,\\,g
    QMAKE_POST_LINK += copy /y \"$$DLL_SRC\" \"$$DLL_DST\"
    QMAKE_POST_LINK += $$escape_expand(\n\t)

    LAY_SRC = $$PWD/layouts
    LAY_DST = $$OUT_DIR/layouts
    LAY_SRC ~= s,/,\\,g
    LAY_DST ~= s,/,\\,g
    # 仅当源 layouts/ 目录存在时再拷贝, 否则 xcopy 返回非零会导致整个构建失败.
    QMAKE_POST_LINK += if exist \"$$LAY_SRC\" xcopy /S /E /Y /I \"$$LAY_SRC\" \"$$LAY_DST\"
}
