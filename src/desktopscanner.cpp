#include "desktopscanner.h"
#include <QDir>
#include <QDirIterator>
#include <algorithm>

DesktopScanner::DesktopScanner(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    m_scanPaths << "/usr/share/applications"
                << QDir::homePath() + "/.local/share/applications";

    QString xdgDesktop = qgetenv("XDG_DESKTOP_DIR");
    QString desk = (!xdgDesktop.isEmpty() && QDir(xdgDesktop).exists())
                       ? xdgDesktop
                       : (QDir::homePath() + "/Desktop");
    m_scanPaths << desk;

    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &DesktopScanner::directoryChanged);
}

void DesktopScanner::addScanPath(const QString &path) {
    if (!m_scanPaths.contains(path))
        m_scanPaths << path;
}

void DesktopScanner::scan() {
    m_entries.clear();
    QStringList seenNames;

    for (const QString &dir : m_scanPaths) {
        if (!QDir(dir).exists()) continue;
        m_watcher->addPath(dir);

        QDirIterator it(dir, QStringList() << "*.desktop",
                        QDir::Files, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString path = it.next();
            DesktopEntry entry(path);
            // Skip hidden/nodisplay
            if (entry.hidden() || entry.noDisplay()) continue;
            if (entry.name().isEmpty()) continue;
            // Deduplicate by name (local overrides system)
            if (seenNames.contains(entry.name())) continue;
            seenNames << entry.name();
            m_entries << entry;
        }
    }

    // Sort alphabetically
    std::sort(m_entries.begin(), m_entries.end(),
              [](const DesktopEntry &a, const DesktopEntry &b) {
                  return a.name().toLower() < b.name().toLower();
              });

    emit scanFinished(m_entries);
}
