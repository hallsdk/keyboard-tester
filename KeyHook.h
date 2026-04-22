#ifndef KEYHOOK_H
#define KEYHOOK_H

#include <QObject>

/**
 * KeyHook
 *  - 安装 Windows 低级键盘钩子 (WH_KEYBOARD_LL).
 *  - 按下/抬起事件通过 keyEvent(scanCode, pressed) 信号投递到主线程 (DirectConnection).
 *  - scanCode 规则与旧工程一致: Extended key 加 0x100, 特殊 VK 补偿常见 0 scanCode.
 *  - 同一时刻仅支持一个全局 KeyHook 实例.
 */
class KeyHook : public QObject
{
    Q_OBJECT
public:
    explicit KeyHook(QObject* parent = nullptr);
    ~KeyHook() override;

    bool install();
    void uninstall();

    static KeyHook* instance() { return s_instance; }

signals:
    void keyEvent(int scanCode, bool pressed);

private:
    static KeyHook* s_instance;
    void*           m_hook = nullptr;   // HHOOK
};

#endif // KEYHOOK_H
