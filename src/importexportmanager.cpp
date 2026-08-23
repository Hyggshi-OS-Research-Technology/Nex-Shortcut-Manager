#include "importexportmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

ImportExportManager::ImportExportManager(QObject *parent)
    : QObject(parent) {}

bool ImportExportManager::exportToJson(const QList<DesktopEntry> &entries,
                                        const QString &filePath) const {
    QJsonArray arr;
    for (const DesktopEntry &e : entries) {
        QJsonObject obj;
        obj["name"]       = e.name();
        obj["exec"]       = e.exec();
        obj["icon"]       = e.icon();
        obj["categories"] = e.categories();
        obj["comment"]    = e.comment();
        obj["terminal"]   = e.terminal();
        obj["type"]       = e.type();
        obj["path"]       = e.path();
        obj["filePath"]   = e.filePath();
        arr.append(obj);
    }

    QJsonObject root;
    root["version"]   = 1;
    root["exported"]  = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["shortcuts"] = arr;

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Cannot write to: " + filePath;
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

QList<DesktopEntry> ImportExportManager::importFromJson(const QString &filePath) const {
    QList<DesktopEntry> result;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        m_lastError = "Cannot open: " + filePath;
        return result;
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        m_lastError = "JSON parse error: " + err.errorString();
        return result;
    }

    QJsonArray arr = doc.object()["shortcuts"].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        DesktopEntry e;
        e.setName(obj["name"].toString());
        e.setExec(obj["exec"].toString());
        e.setIcon(obj["icon"].toString());
        e.setCategories(obj["categories"].toString());
        e.setComment(obj["comment"].toString());
        e.setTerminal(obj["terminal"].toBool());
        e.setType(obj["type"].toString("Application"));
        e.setPath(obj["path"].toString());
        e.setFilePath(obj["filePath"].toString());
        result << e;
    }
    return result;
}
