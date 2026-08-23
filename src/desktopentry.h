#pragma once
#include <QString>
#include <QStringList>
#include <QMap>
#include <QIcon>

class DesktopEntry {
public:
    DesktopEntry() = default;
    explicit DesktopEntry(const QString &filePath);

    bool load(const QString &filePath);
    bool save(const QString &filePath) const;
    bool isValid() const;
    QStringList validate() const; // returns list of errors

    // Getters
    QString filePath() const { return m_filePath; }
    QString name() const { return m_name; }
    QString genericName() const { return m_genericName; }
    QString comment() const { return m_comment; }
    QString exec() const { return m_exec; }
    QString icon() const { return m_icon; }
    QString categories() const { return m_categories; }
    QString type() const { return m_type; }
    bool noDisplay() const { return m_noDisplay; }
    bool hidden() const { return m_hidden; }
    bool terminal() const { return m_terminal; }
    QString tryExec() const { return m_tryExec; }
    QString path() const { return m_path; }

    // Setters
    void setName(const QString &v) { m_name = v; }
    void setGenericName(const QString &v) { m_genericName = v; }
    void setComment(const QString &v) { m_comment = v; }
    void setExec(const QString &v) { m_exec = v; }
    void setIcon(const QString &v) { m_icon = v; }
    void setCategories(const QString &v) { m_categories = v; }
    void setType(const QString &v) { m_type = v; }
    void setNoDisplay(bool v) { m_noDisplay = v; }
    void setTerminal(bool v) { m_terminal = v; }
    void setPath(const QString &v) { m_path = v; }
    void setFilePath(const QString &v) { m_filePath = v; }

    QIcon resolveIcon() const;

    // For file/folder shortcuts
    static DesktopEntry createFileShortcut(const QString &filePath);
    static DesktopEntry createFolderShortcut(const QString &folderPath);
    static DesktopEntry createCustomShortcut(const QString &name,
                                              const QString &exec,
                                              const QString &icon,
                                              const QString &comment = QString(),
                                              bool terminal = false);

    QMap<QString, QString> extraFields() const { return m_extraFields; }
    void setExtraField(const QString &key, const QString &value) { m_extraFields[key] = value; }

private:
    QString m_filePath;
    QString m_name;
    QString m_genericName;
    QString m_comment;
    QString m_exec;
    QString m_icon;
    QString m_categories;
    QString m_type = "Application";
    QString m_tryExec;
    QString m_path;
    bool m_noDisplay = false;
    bool m_hidden = false;
    bool m_terminal = false;
    QMap<QString, QString> m_extraFields;
};
