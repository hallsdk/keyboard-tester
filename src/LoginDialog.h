#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QTabWidget;
class ApiClient;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(ApiClient* api, QWidget* parent = nullptr);

private slots:
    void onLogin();
    void onRegister();
    void onWorkOffline();
    void onEditServer();

private:
    ApiClient*   m_api      = nullptr;
    QLabel*      m_server   = nullptr;
    QTabWidget*  m_tabs     = nullptr;
    QLineEdit*   m_userL    = nullptr;
    QLineEdit*   m_pwL      = nullptr;
    QLineEdit*   m_userR    = nullptr;
    QLineEdit*   m_pwR      = nullptr;
    QLineEdit*   m_pw2R     = nullptr;
    QPushButton* m_btnLogin = nullptr;
    QPushButton* m_btnReg   = nullptr;
    QLabel*      m_msg      = nullptr;
};

#endif // LOGIN_DIALOG_H
