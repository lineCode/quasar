#pragma once

#include <QVariant>

#include <shared_mutex>

QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(WidgetRegistry)
QT_FORWARD_DECLARE_CLASS(DataServer)

struct AppLauncherData
{
    QString file;
    QString startpath;
    QString arguments;

    friend QDataStream& operator<<(QDataStream& s, const AppLauncherData& o)
    {
        s << o.file;
        s << o.startpath;
        s << o.arguments;
        return s;
    }

    friend QDataStream& operator>>(QDataStream& s, AppLauncherData& o)
    {
        s >> o.file;
        s >> o.startpath;
        s >> o.arguments;
        return s;
    }
};

Q_DECLARE_METATYPE(AppLauncherData);

class AppLauncher : public QObject
{
    friend class DataServices;

    Q_OBJECT

public:
    const QVariantMap* getMapForRead();
    void               releaseMap(const QVariantMap*& map);

    void writeMap(QVariantMap& newmap);

private:
    void handleCommand(const QJsonObject& req, QWebSocket* sender);

private:
    explicit AppLauncher(DataServer* s, WidgetRegistry* r, QObject* parent = Q_NULLPTR);
    AppLauncher(const AppLauncher&) = delete;
    AppLauncher(AppLauncher&&)      = delete;
    AppLauncher& operator=(const AppLauncher&) = delete;
    AppLauncher& operator=(AppLauncher&&) = delete;

    DataServer*       server;
    WidgetRegistry*   reg;
    QVariantMap       m_map;
    std::shared_mutex m_mutex;
};
