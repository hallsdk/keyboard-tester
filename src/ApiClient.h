#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <cstdint>

class QNetworkAccessManager;

struct ApiDevice {
    int     id          = 0;
    QString vid;            // "0xHHHH"
    QString pid;            // "0xHHHH"
    QString name;
    QString description;
    QString layoutFile;     // server-side filename, e.g. "wp75.c"
    QString updatedAt;
};

/// Thin synchronous client for the ktester-backend REST API.
/// Token + base URL are persisted via QSettings.
/// Blocking calls run a local event loop so they can be used from
/// straight-line code (response sizes are tiny: JSON metadata + small
/// layout files). All methods return false on transport/HTTP error and
/// fill *errMsg when given.
class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject* parent = nullptr);
    ~ApiClient() override;

    // ----- persisted config -----
    QString baseUrl() const;                // e.g. "https://ktester.hallsdk.com"
    void    setBaseUrl(const QString& url); // saved to QSettings
    QString token()   const;
    QString username()const;
    QString role()    const;                // "user" / "admin" / "super_admin"
    bool    isLoggedIn() const { return !token().isEmpty(); }
    void    clearAuth();

    // ----- auth -----
    bool login   (const QString& user, const QString& pw, QString* errMsg = nullptr);
    bool registerUser(const QString& user, const QString& pw, QString* errMsg = nullptr);
    bool fetchMe (QString* errMsg = nullptr); // refresh role/username

    // ----- devices -----
    bool listDevices(QList<ApiDevice>* out, QString* errMsg = nullptr);
    /// Download a layout file for a given VID/PID.
    /// On success fills *outBody with the raw file bytes and *outFilename
    /// with the server-suggested filename (from Content-Disposition or device record).
    bool fetchLayout(uint16_t vid, uint16_t pid,
                     QByteArray* outBody, QString* outFilename,
                     QString* errMsg = nullptr);

private:
    // ----- low-level -----
    struct Reply {
        int        httpStatus = 0;
        QByteArray body;
        QString    contentType;
        QString    contentDisposition;
        QString    networkError;
        bool ok() const { return networkError.isEmpty() && httpStatus >= 200 && httpStatus < 300; }
    };
    Reply request(const QByteArray& verb,
                  const QString& path,
                  const QByteArray& body,
                  const QByteArray& contentType,
                  int timeoutMs = 8000);
    static QString parseFilename(const QString& contentDisposition);
    static QString parseErrorMessage(const QByteArray& body, int httpStatus);

    QNetworkAccessManager* m_nam = nullptr;
};

#endif // API_CLIENT_H
