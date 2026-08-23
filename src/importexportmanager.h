#pragma once
#include "desktopentry.h"
#include <QObject>
#include <QList>

class ImportExportManager : public QObject {
    Q_OBJECT
public:
    explicit ImportExportManager(QObject *parent = nullptr);

    bool exportToJson(const QList<DesktopEntry> &entries, const QString &filePath) const;
    QList<DesktopEntry> importFromJson(const QString &filePath) const;

    QString lastError() const { return m_lastError; }

private:
    mutable QString m_lastError;
};
