#include "KeyHook.h"

#include <windows.h>
#include <QDebug>

KeyHook* KeyHook::s_instance = nullptr;

static LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && KeyHook::instance()) {
        const KBDLLHOOKSTRUCT* k = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        const bool press   = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        const bool release = (wParam == WM_KEYUP   || wParam == WM_SYSKEYUP);

        if (press || release) {
            int scanCode = (int)k->scanCode;
            const int EXT = 0x100;
            if (k->flags & LLKHF_EXTENDED) scanCode += EXT;

            if (k->scanCode == 0) {
                switch (k->vkCode) {
                case VK_PAUSE:               scanCode = 0x5B + EXT; break;
                case 0xE2:                   scanCode = 0x53 + EXT; break; // Non-US Bslash
                // Browser
                case VK_BROWSER_REFRESH:     scanCode = 0x67 + EXT; break;
                case VK_BROWSER_SEARCH:      scanCode = 0x65 + EXT; break;
                case VK_BROWSER_FAVORITES:   scanCode = 0x66 + EXT; break;
                case VK_BROWSER_HOME:        scanCode = 0x32 + EXT; break;
                case VK_BROWSER_BACK:        scanCode = 0x6A + EXT; break;
                case VK_BROWSER_FORWARD:     scanCode = 0x69 + EXT; break;
                case VK_BROWSER_STOP:        scanCode = 0x68 + EXT; break;
                // Media transport
                case VK_MEDIA_PLAY_PAUSE:    scanCode = 0x22 + EXT; break;
                case VK_MEDIA_STOP:          scanCode = 0x24 + EXT; break;
                case VK_MEDIA_PREV_TRACK:    scanCode = 0x10 + EXT; break;
                case VK_MEDIA_NEXT_TRACK:    scanCode = 0x19 + EXT; break;
                // Volume
                case VK_VOLUME_MUTE:         scanCode = 0x20 + EXT; break;
                case VK_VOLUME_DOWN:         scanCode = 0x2E + EXT; break;
                case VK_VOLUME_UP:           scanCode = 0x30 + EXT; break;
                // Launchers
                case VK_LAUNCH_MAIL:         scanCode = 0x6C + EXT; break;
                case VK_LAUNCH_APP1:         scanCode = 0x6B + EXT; break; // My Computer / My PC
                case VK_LAUNCH_APP2:         scanCode = 0x21 + EXT; break; // Calculator
                case VK_LAUNCH_MEDIA_SELECT: scanCode = 0x6D + EXT; break; // Media-player launcher
                // System control
                case VK_SLEEP:               scanCode = 0x5F + EXT; break;
                }
            }
            emit KeyHook::instance()->keyEvent(scanCode, press);
        }
        return 1;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

KeyHook::KeyHook(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

KeyHook::~KeyHook()
{
    uninstall();
    if (s_instance == this) s_instance = nullptr;
}

bool KeyHook::install()
{
    if (m_hook) return true;
    HHOOK h = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, GetModuleHandle(nullptr), 0);
    if (!h) {
        qWarning() << "[KeyHook] SetWindowsHookEx failed:" << GetLastError();
        return false;
    }
    m_hook = h;
    qDebug() << "[KeyHook] installed";
    return true;
}

void KeyHook::uninstall()
{
    if (!m_hook) return;
    UnhookWindowsHookEx(reinterpret_cast<HHOOK>(m_hook));
    m_hook = nullptr;
    qDebug() << "[KeyHook] uninstalled";
}
