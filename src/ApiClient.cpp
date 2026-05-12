#include "ApiClient.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>

namespace {
constexpr const char* kOrg = "Hallsdk";
constexpr const char* kApp = "KeyboardTesterV2";
constexpr const char* kKeyBaseUrl  = "api/baseUrl";
constexpr const char* kKeyToken    = "api/token";
constexpr const char* kKeyUsername = "api/username";
constexpr const char* kKeyRole     = "api/role";
}

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

ApiClient::~ApiClient() = default;

// ---------------------------------------------------------------------------
// Persisted config
// ---------------------------------------------------------------------------
QString ApiClient::baseUrl() const
{
    QSettings s(kOrg, kApp);
    return s.value(kKeyBaseUrl, "https://ktester.hallsdk.com").toString();
}

void ApiClient::setBaseUrl(const QString& url)
{
    QString u = url.trimmed();
    while (u.endsWith('/')) u.chop(1);
    QSettings s(kOrg, kApp);
    s.setValue(kKeyBaseUrl, u);
}

QString ApiClient::token()    const { return QSettings(kOrg, kApp).value(kKeyToken).toString(); }
QString ApiClient::username() const { return QSettings(kOrg, kApp).value(kKeyUsername).toString(); }
QString ApiClient::role()     const { return QSettings(kOrg, kApp).value(kKeyRole).toString(); }

void ApiClient::clearAuth()
{
    QSettings s(kOrg, kApp);
    s.remove(kKeyToken);
    s.remove(kKeyUsername);
    s.remove(kKeyRole);
}

// ---------------------------------------------------------------------------
// Low-level synchronous request (uses local event loop)
// ---------------------------------------------------------------------------
ApiClient::Reply ApiClient::request(const QByteArray& verb,
                                    const QString& path,
                                    const QByteArray& body,
                                    const QByteArray& contentType,
                                    int timeoutMs)
{
    Reply r;
    const QString base = baseUrl();
    if (base.isEmpty()) { r.networkError = "未配置服务器地址"; return r; }

    QUrl url(base + path);
    if (!url.isValid()) { r.networkError = "URL 无效: " + url.toString(); return r; }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("KeyboardTesterV2/Qt"));
    if (!contentType.isEmpty())
        req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    const QString t = token();
    if (!t.isEmpty())
        req.setRawHeader("Authorization", QByteArray("Bearer ") + t.toUtf8());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = m_nam->sendCustomRequest(req, verb, body);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    bool timedOut = false;
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]{
        timedOut = true;
        reply->abort();
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    timer.stop();

    r.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    r.body = reply->readAll();
    r.contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    r.contentDisposition = QString::fromUtf8(reply->rawHeader("Content-Disposition"));

    if (timedOut) {
        r.networkError = "请求超时";
    } else if (reply->error() != QNetworkReply::NoError &&
               (r.httpStatus < 400 || r.httpStatus == 0)) {
        // Real network error (DNS/TCP/TLS). HTTP-level errors keep going so
        // the caller can read the body.
        r.networkError = reply->errorString();
    }
    reply->deleteLater();
    return r;
}

QString ApiClient::parseFilename(const QString& cd)
{
    // RFC6266: Content-Disposition: attachment; filename="xxx.c"; filename*=UTF-8''xxx.c
    static const QRegularExpression re1(R"#(filename\*\s*=\s*[^']*''([^;]+))#",
                                        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression re2(R"#(filename\s*=\s*"?([^";]+)"?)#",
                                        QRegularExpression::CaseInsensitiveOption);
    auto m = re1.match(cd);
    if (m.hasMatch()) return QUrl::fromPercentEncoding(m.captured(1).toUtf8());
    m = re2.match(cd);
    if (m.hasMatch()) return m.captured(1).trimmed();
    return QString();
}

QString ApiClient::parseErrorMessage(const QByteArray& body, int httpStatus)
{
    if (!body.isEmpty()) {
        QJsonParseError pe;
        const auto doc = QJsonDocument::fromJson(body, &pe);
        if (pe.error == QJsonParseError::NoError && doc.isObject()) {
            const auto err = doc.object().value("error").toString();
            if (!err.isEmpty()) return err;
        }
    }
    return QString("HTTP %1").arg(httpStatus);
}

// ---------------------------------------------------------------------------
// Auth
// ---------------------------------------------------------------------------
bool ApiClient::login(const QString& user, const QString& pw, QString* errMsg)
{
    QJsonObject body{ {"username", user}, {"password", pw} };
    const auto r = request("POST", "/api/auth/login",
                           QJsonDocument(body).toJson(QJsonDocument::Compact),
                           "application/json");
    if (!r.ok()) {
        if (errMsg) *errMsg = r.networkError.isEmpty()
            ? parseErrorMessage(r.body, r.httpStatus)
            : r.networkError;
        return false;
    }
    QJsonParseError pe;
    const auto doc = QJsonDocument::fromJson(r.body, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errMsg) *errMsg = "登录响应解析失败"; return false;
    }
    const auto obj = doc.object();
    QSettings s(kOrg, kApp);
    s.setValue(kKeyToken,    obj.value("token").toString());
    s.setValue(kKeyUsername, user);
    s.setValue(kKeyRole,     obj.value("role").toString());
    return true;
}

bool ApiClient::registerUser(const QString& user, const QString& pw, QString* errMsg)
{
    QJsonObject body{ {"username", user}, {"password", pw} };
    const auto r = request("POST", "/api/auth/register",
                           QJsonDocument(body).toJson(QJsonDocument::Compact),
                           "application/json");
    if (!r.ok()) {
        if (errMsg) *errMsg = r.networkError.isEmpty()
            ? parseErrorMessage(r.body, r.httpStatus)
            : r.networkError;
        return false;
    }
    const auto obj = QJsonDocument::fromJson(r.body).object();
    QSettings s(kOrg, kApp);
    s.setValue(kKeyToken,    obj.value("token").toString());
    s.setValue(kKeyUsername, user);
    s.setValue(kKeyRole,     obj.value("role").toString());
    return true;
}

bool ApiClient::fetchMe(QString* errMsg)
{
    const auto r = request("GET", "/api/auth/me", {}, {});
    if (!r.ok()) {
        if (errMsg) *errMsg = r.networkError.isEmpty()
            ? parseErrorMessage(r.body, r.httpStatus) : r.networkError;
        return false;
    }
    const auto obj = QJsonDocument::fromJson(r.body).object();
    QSettings s(kOrg, kApp);
    s.setValue(kKeyUsername, obj.value("username").toString());
    s.setValue(kKeyRole,     obj.value("role").toString());
    return true;
}

// ---------------------------------------------------------------------------
// Devices
// ---------------------------------------------------------------------------
bool ApiClient::listDevices(QList<ApiDevice>* out, QString* errMsg)
{
    const auto r = request("GET", "/api/devices", {}, {});
    if (!r.ok()) {
        if (errMsg) *errMsg = r.networkError.isEmpty()
            ? parseErrorMessage(r.body, r.httpStatus) : r.networkError;
        return false;
    }
    const auto arr = QJsonDocument::fromJson(r.body).array();
    out->clear();
    out->reserve(arr.size());
    for (const auto& v : arr) {
        const auto o = v.toObject();
        ApiDevice d;
        d.id          = o.value("id").toInt();
        d.vid         = o.value("vid").toString();
        d.pid         = o.value("pid").toString();
        d.name        = o.value("name").toString();
        d.description = o.value("description").toString();
        d.layoutFile  = o.value("layout_file").toString();
        d.updatedAt   = o.value("updated_at").toString();
        out->push_back(d);
    }
    return true;
}

bool ApiClient::fetchLayout(uint16_t vid, uint16_t pid,
                            QByteArray* outBody, QString* outFilename,
                            QString* errMsg)
{
    const QString key = QString("0x%1-0x%2")
        .arg(vid, 4, 16, QChar('0'))
        .arg(pid, 4, 16, QChar('0'))
        .toUpper();
    const auto r = request("GET",
                           QString("/api/devices/%1/layout").arg(key),
                           {}, {}, /*timeoutMs=*/10000);
    if (!r.ok()) {
        if (errMsg) *errMsg = r.networkError.isEmpty()
            ? parseErrorMessage(r.body, r.httpStatus) : r.networkError;
        return false;
    }
    if (outBody) *outBody = r.body;
    if (outFilename) {
        QString fn = parseFilename(r.contentDisposition);
        if (fn.isEmpty()) {
            // Fallback: synthesize from VID-PID.
            fn = key + (r.contentType.contains("json") ? ".json" : ".c");
        }
        *outFilename = fn;
    }
    return true;
}
