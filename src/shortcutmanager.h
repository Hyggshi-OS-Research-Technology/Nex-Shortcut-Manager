#pragma once
#include "desktopentry.h"
#include <QObject>
#include <QSet>
#include <QSettings>

class ShortcutManager : public QObject {
    Q_OBJECT
public:
    explicit ShortcutManager(QObject *parent = nullptr);

    // Desktop operations
    bool addToDesktop(const DesktopEntry &entry);
    bool removeFromDesktop(const QString &desktopFilePath);
    bool isOnDesktop(const DesktopEntry &entry) const;
    QString desktopFilePath(const DesktopEntry &entry) const;

    // Edit operations
    bool saveEntry(const DesktopEntry &entry, const QString &targetPath);
    bool editDesktopShortcut(const DesktopEntry &entry);

    // Starred/pinned
    void pinApp(const QString &appId);
    void unpinApp(const QString &appId);
    bool isPinned(const QString &appId) const;
    QStringList pinnedApps() const;

    // Paths
    QString desktopPath() const;
    QList<DesktopEntry> desktopShortcuts() const;

    // Refresh
    void refreshDesktop();

    // Validate
    QStringList validateEntry(const DesktopEntry &entry) const;

signals:
    void shortcutAdded(const QString &path);
    void shortcutRemoved(const QString &path);
    void pinnedChanged();

private:
    QString m_desktopPath;
    QSettings m_settings;

    QString sanitizeFilename(const QString &name) const;
};
