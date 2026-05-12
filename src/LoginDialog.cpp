#include "LoginDialog.h"
#include "ApiClient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QWidget>
#include <QInputDialog>
#include <QApplication>

LoginDialog::LoginDialog(ApiClient* api, QWidget* parent)
    : QDialog(parent), m_api(api)
{
    setWindowTitle("登录 — Keyboard Tester");
    setModal(true);
    resize(420, 360);

    auto* root = new QVBoxLayout(this);

    // ---- Server line ----
    auto* srvRow = new QHBoxLayout();
    m_server = new QLabel(this);
    m_server->setText(QString("服务器: %1").arg(m_api->baseUrl()));
    m_server->setStyleSheet("color:#888");
    auto* btnSrv = new QPushButton("修改…", this);
    btnSrv->setFlat(true);
    srvRow->addWidget(m_server, 1);
    srvRow->addWidget(btnSrv);
    root->addLayout(srvRow);
    connect(btnSrv, &QPushButton::clicked, this, &LoginDialog::onEditServer);

    // ---- Tabs ----
    m_tabs = new QTabWidget(this);

    // Login tab
    {
        auto* w = new QWidget(this);
        auto* f = new QFormLayout(w);
        m_userL = new QLineEdit(w);
        m_userL->setText(m_api->username()); // pre-fill last user
        m_pwL = new QLineEdit(w);
        m_pwL->setEchoMode(QLineEdit::Password);
        f->addRow("用户名", m_userL);
        f->addRow("密  码", m_pwL);
        m_btnLogin = new QPushButton("登录", w);
        m_btnLogin->setDefault(true);
        f->addRow(m_btnLogin);
        connect(m_btnLogin, &QPushButton::clicked, this, &LoginDialog::onLogin);
        m_tabs->addTab(w, "登 录");
    }
    // Register tab
    {
        auto* w = new QWidget(this);
        auto* f = new QFormLayout(w);
        m_userR = new QLineEdit(w);
        m_pwR = new QLineEdit(w);  m_pwR ->setEchoMode(QLineEdit::Password);
        m_pw2R = new QLineEdit(w); m_pw2R->setEchoMode(QLineEdit::Password);
        f->addRow("用户名 (3-32)", m_userR);
        f->addRow("密码 (≥6)", m_pwR);
        f->addRow("确认密码", m_pw2R);
        m_btnReg = new QPushButton("注册并登录", w);
        f->addRow(m_btnReg);
        connect(m_btnReg, &QPushButton::clicked, this, &LoginDialog::onRegister);
        m_tabs->addTab(w, "注 册");
    }
    root->addWidget(m_tabs, 1);

    m_msg = new QLabel(this);
    m_msg->setWordWrap(true);
    m_msg->setStyleSheet("color:#c0392b;");
    root->addWidget(m_msg);

    auto* bottom = new QHBoxLayout();
    auto* offline = new QPushButton("离线使用 (使用本地缓存的布局)", this);
    offline->setFlat(true);
    bottom->addStretch(1);
    bottom->addWidget(offline);
    root->addLayout(bottom);
    connect(offline, &QPushButton::clicked, this, &LoginDialog::onWorkOffline);
}

void LoginDialog::onEditServer()
{
    bool ok = false;
    const QString cur = m_api->baseUrl();
    const QString v = QInputDialog::getText(this, "服务器地址",
        "格式: https://host[:port]，无尾随斜杠",
        QLineEdit::Normal, cur, &ok);
    if (!ok || v.trimmed().isEmpty()) return;
    m_api->setBaseUrl(v.trimmed());
    m_server->setText(QString("服务器: %1").arg(m_api->baseUrl()));
}

void LoginDialog::onLogin()
{
    m_msg->clear();
    const QString u = m_userL->text().trimmed();
    const QString p = m_pwL->text();
    if (u.isEmpty() || p.isEmpty()) {
        m_msg->setText("请填写用户名和密码"); return;
    }
    m_btnLogin->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString err;
    const bool ok = m_api->login(u, p, &err);
    QApplication::restoreOverrideCursor();
    m_btnLogin->setEnabled(true);
    if (!ok) { m_msg->setText("登录失败: " + err); return; }
    accept();
}

void LoginDialog::onRegister()
{
    m_msg->clear();
    const QString u  = m_userR->text().trimmed();
    const QString p  = m_pwR->text();
    const QString p2 = m_pw2R->text();
    if (u.size() < 3) { m_msg->setText("用户名至少 3 个字符"); return; }
    if (p.size() < 6) { m_msg->setText("密码至少 6 个字符"); return; }
    if (p != p2)      { m_msg->setText("两次输入的密码不一致"); return; }
    m_btnReg->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString err;
    const bool ok = m_api->registerUser(u, p, &err);
    QApplication::restoreOverrideCursor();
    m_btnReg->setEnabled(true);
    if (!ok) { m_msg->setText("注册失败: " + err); return; }
    accept();
}

void LoginDialog::onWorkOffline()
{
    // Caller treats reject() as "skip login" — local cache will be used.
    reject();
}
