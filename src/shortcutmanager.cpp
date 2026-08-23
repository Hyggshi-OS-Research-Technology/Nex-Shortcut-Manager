#include "shortcutmanager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>
#include <QDirIterator>
#include <QRegularExpression>

ShortcutManager::ShortcutManager(QObject *parent)
    : QObject(parent)
    , m_settings("ShortcutManager", "ShortcutManager")
{
    // Prefer XDG_DESKTOP_DIR, fallback to ~/Desktop
    QString xdgDesktop = qgetenv("XDG_DESKTOP_DIR");
    if (!xdgDesktop.isEmpty() && QDir(xdgDesktop).exists()) {
        m_desktopPath = xdgDesktop;
    } else {
        m_desktopPath = QDir::homePath() + "/Desktop";
        QDir().mkpath(m_desktopPath);
    }
}

QString ShortcutManager::desktopPath() const {
    return m_desktopPath;
}

QString ShortcutManager::sanitizeFilename(const QString &name) const {
    QString safe = name;
    safe.replace(QRegularExpression("[/\\\\:*?\"<>|]"), "_");
    safe = safe.trimmed();
    if (safe.isEmpty()) safe = "shortcut";
    return safe;
}

QString ShortcutManager::desktopFilePath(const DesktopEntry &entry) const {
    // 1. If the entry itself is from the desktop folder
    if (!entry.filePath().isEmpty() && entry.filePath().startsWith(m_desktopPath) && QFile::exists(entry.filePath())) {
        return entry.filePath();
    }

    // 2. Check sanitized name desktop file
    QString byName = m_desktopPath + "/" + sanitizeFilename(entry.name()) + ".desktop";
    if (QFile::exists(byName)) {
        return byName;
    }

    // 3. Check original filename
    if (!entry.filePath().isEmpty()) {
        QString origName = QFileInfo(entry.filePath()).fileName();
        QString byOrigName = m_desktopPath + "/" + origName;
        if (QFile::exists(byOrigName)) {
            return byOrigName;
        }
    }

    // 4. Scan all .desktop files in Desktop to match Name or Exec
    QDirIterator it(m_desktopPath, QStringList() << "*.desktop", QDir::Files);
    while (it.hasNext()) {
        QString path = it.next();
        DesktopEntry d(path);
        if (d.name().compare(entry.name(), Qt::CaseInsensitive) == 0) {
            return path;
        }
        if (!entry.exec().isEmpty() && !d.exec().isEmpty()) {
            QString exec1 = entry.exec().section(' ', 0, 0);
            QString exec2 = d.exec().section(' ', 0, 0);
            if (!exec1.isEmpty() && exec1 == exec2) {
                return path;
            }
        }
    }

    return byName;
}

bool ShortcutManager::isOnDesktop(const DesktopEntry &entry) const {
    // 1. If entry itself is from Desktop folder
    if (!entry.filePath().isEmpty() && entry.filePath().startsWith(m_desktopPath) && QFile::exists(entry.filePath())) {
        return true;
    }
    QString path = desktopFilePath(entry);
    return QFile::exists(path);
}

bool ShortcutManager::addToDesktop(const DesktopEntry &entry) {
    QString path = desktopFilePath(entry);
    if (!entry.save(path))
        return false;

    // Make executable (required for some DEs)
    QFile::setPermissions(path,
        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
        QFile::ReadGroup | QFile::ExeGroup |
        QFile::ReadOther | QFile::ExeOther);

    // Mark as trusted (for GNOME/KDE)
    QProcess::startDetached("gio", QStringList() << "set" << path
                             << "metadata::trusted" << "true");

    emit shortcutAdded(path);
    return true;
}

bool ShortcutManager::removeFromDesktop(const QString &desktopFilePath) {
    if (QFile::exists(desktopFilePath) && QFile::remove(desktopFilePath)) {
        emit shortcutRemoved(desktopFilePath);
        return true;
    }
    return false;
}

bool ShortcutManager::saveEntry(const DesktopEntry &entry, const QString &targetPath) {
    if (!entry.save(targetPath))
        return false;

    QFile::setPermissions(targetPath,
        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
        QFile::ReadGroup | QFile::ExeGroup |
        QFile::ReadOther | QFile::ExeOther);
    return true;
}

bool ShortcutManager::editDesktopShortcut(const DesktopEntry &entry) {
    QString path = desktopFilePath(entry);
    if (!QFile::exists(path)) return false;
    return saveEntry(entry, path);
}

void ShortcutManager::pinApp(const QString &appId) {
    QStringList pinned = pinnedApps();
    if (!pinned.contains(appId)) {
        pinned << appId;
        m_settings.setValue("pinned", pinned);
        emit pinnedChanged();
    }
}

void ShortcutManager::unpinApp(const QString &appId) {
    QStringList pinned = pinnedApps();
    if (pinned.removeAll(appId) > 0) {
        m_settings.setValue("pinned", pinned);
        emit pinnedChanged();
    }
}

bool ShortcutManager::isPinned(const QString &appId) const {
    return pinnedApps().contains(appId);
}

QStringList ShortcutManager::pinnedApps() const {
    return m_settings.value("pinned", QStringList()).toStringList();
}

QList<DesktopEntry> ShortcutManager::desktopShortcuts() const {
    QList<DesktopEntry> shortcuts;
    QDirIterator it(m_desktopPath, QStringList() << "*.desktop",
                    QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        shortcuts << DesktopEntry(it.next());
    }
    return shortcuts;
}

void ShortcutManager::refreshDesktop() {
    // Trigger DE to refresh desktop icons
    QProcess::startDetached("xdg-user-dirs-update", {});
}

QStringList ShortcutManager::validateEntry(const DesktopEntry &entry) const {
    return entry.validate();
}
