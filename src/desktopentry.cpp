#include "desktopentry.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include <QMimeDatabase>

DesktopEntry::DesktopEntry(const QString &filePath) {
    load(filePath);
}

bool DesktopEntry::load(const QString &filePath) {
    m_filePath = filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    bool inDesktopEntry = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        if (line == "[Desktop Entry]") {
            inDesktopEntry = true;
            continue;
        }
        if (line.startsWith('[') && inDesktopEntry)
            break; // stop at next section

        if (!inDesktopEntry) continue;

        int eq = line.indexOf('=');
        if (eq < 0) continue;

        QString key = line.left(eq).trimmed();
        QString val = line.mid(eq + 1).trimmed();

        if (key == "Name")             m_name = val;
        else if (key == "GenericName") m_genericName = val;
        else if (key == "Comment")     m_comment = val;
        else if (key == "Exec")        m_exec = val;
        else if (key == "Icon")        m_icon = val;
        else if (key == "Categories")  m_categories = val;
        else if (key == "Type")        m_type = val;
        else if (key == "TryExec")     m_tryExec = val;
        else if (key == "Path")        m_path = val;
        else if (key == "NoDisplay")   m_noDisplay = (val.toLower() == "true");
        else if (key == "Hidden")      m_hidden = (val.toLower() == "true");
        else if (key == "Terminal")    m_terminal = (val.toLower() == "true");
        else m_extraFields[key] = val;
    }
    return true;
}

bool DesktopEntry::save(const QString &filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "[Desktop Entry]\n";
    out << "Type=" << (m_type.isEmpty() ? "Application" : m_type) << "\n";
    if (!m_name.isEmpty())        out << "Name=" << m_name << "\n";
    if (!m_genericName.isEmpty()) out << "GenericName=" << m_genericName << "\n";
    if (!m_comment.isEmpty())     out << "Comment=" << m_comment << "\n";
    if (!m_exec.isEmpty())        out << "Exec=" << m_exec << "\n";
    if (!m_icon.isEmpty())        out << "Icon=" << m_icon << "\n";
    if (!m_categories.isEmpty())  out << "Categories=" << m_categories << "\n";
    if (!m_path.isEmpty())        out << "Path=" << m_path << "\n";
    if (!m_tryExec.isEmpty())     out << "TryExec=" << m_tryExec << "\n";
    if (m_terminal)               out << "Terminal=true\n";
    if (m_noDisplay)              out << "NoDisplay=true\n";
    for (auto it = m_extraFields.begin(); it != m_extraFields.end(); ++it)
        out << it.key() << "=" << it.value() << "\n";

    return true;
}

bool DesktopEntry::isValid() const {
    return validate().isEmpty();
}

QStringList DesktopEntry::validate() const {
    QStringList errors;
    if (m_name.trimmed().isEmpty())
        errors << "Name is required";
    if (m_type.trimmed().isEmpty())
        errors << "Type is required";
    if (m_type == "Application" && m_exec.trimmed().isEmpty())
        errors << "Exec is required for Application type";
    if (m_type == "Link") {
        QString url = m_extraFields.value("URL");
        if (url.isEmpty())
            errors << "URL is required for Link type";
    }
    return errors;
}

QIcon DesktopEntry::resolveIcon() const {
    if (m_icon.isEmpty()) {
        if (m_type == "Directory")
            return QIcon::fromTheme("folder");
        return QIcon::fromTheme("application-x-executable");
    }
    // Check if it's an absolute path to an image file
    if (QFileInfo(m_icon).isAbsolute() && QFile::exists(m_icon))
        return QIcon(m_icon);

    // Try theme icon
    QIcon themeIcon = QIcon::fromTheme(m_icon);
    if (!themeIcon.isNull())
        return themeIcon;

    // Try common icon paths
    const QStringList iconDirs = {
        "/usr/share/pixmaps/",
        "/usr/share/icons/hicolor/256x256/apps/",
        "/usr/share/icons/hicolor/128x128/apps/",
        "/usr/share/icons/hicolor/64x64/apps/",
        "/usr/share/icons/hicolor/48x48/apps/",
        QDir::homePath() + "/.local/share/icons/"
    };
    const QStringList exts = {"", ".png", ".svg", ".xpm"};
    for (const QString &dir : iconDirs) {
        for (const QString &ext : exts) {
            QString path = dir + m_icon + ext;
            if (QFile::exists(path))
                return QIcon(path);
        }
    }
    return QIcon::fromTheme("application-x-executable");
}

DesktopEntry DesktopEntry::createFileShortcut(const QString &filePath) {
    DesktopEntry entry;
    QFileInfo fi(filePath);
    entry.setType("Application");
    entry.setName(fi.completeBaseName());
    entry.setExec("xdg-open \"" + filePath + "\"");
    entry.setIcon("text-x-generic");
    entry.setComment("Shortcut to " + filePath);
    return entry;
}

DesktopEntry DesktopEntry::createFolderShortcut(const QString &folderPath) {
    DesktopEntry entry;
    QFileInfo fi(folderPath);
    entry.setType("Application");
    entry.setName(fi.fileName().isEmpty() ? folderPath : fi.fileName());
    entry.setExec("xdg-open \"" + folderPath + "\"");
    entry.setIcon("folder");
    entry.setComment("Shortcut to folder: " + folderPath);
    return entry;
}

DesktopEntry DesktopEntry::createCustomShortcut(const QString &name,
                                                  const QString &exec,
                                                  const QString &icon,
                                                  const QString &comment,
                                                  bool terminal) {
    DesktopEntry entry;
    entry.setType("Application");
    entry.setName(name);
    entry.setExec(exec);
    entry.setIcon(icon);
    entry.setComment(comment);
    entry.setTerminal(terminal);
    return entry;
}
