#pragma once
#include "desktopentry.h"
#include <QObject>
#include <QList>
#include <QFileSystemWatcher>

class DesktopScanner : public QObject {
    Q_OBJECT
public:
    explicit DesktopScanner(QObject *parent = nullptr);

    void scan();
    void addScanPath(const QString &path);

    QList<DesktopEntry> entries() const { return m_entries; }

signals:
    void scanFinished(const QList<DesktopEntry> &entries);
    void directoryChanged(const QString &path);

private:
    QList<QString> m_scanPaths;
    QList<DesktopEntry> m_entries;
    QFileSystemWatcher *m_watcher;
};
