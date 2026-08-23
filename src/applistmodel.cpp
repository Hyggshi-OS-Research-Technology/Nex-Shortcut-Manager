#include "applistmodel.h"
#include <QIcon>
#include <QColor>
#include <QFont>

AppListModel::AppListModel(QObject *parent)
    : QAbstractTableModel(parent) {}

void AppListModel::setEntries(const QList<DesktopEntry> &entries) {
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

void AppListModel::setDesktopEntries(const QList<DesktopEntry> &shortcuts) {
    m_desktopNames.clear();
    for (const DesktopEntry &e : shortcuts) {
        m_desktopNames.insert(e.name().trimmed().toLower());
        if (!e.exec().isEmpty()) {
            m_desktopNames.insert(e.exec().section(' ', 0, 0).trimmed().toLower());
        }
    }
    if (rowCount() > 0) {
        emit dataChanged(index(0, ColOnDesktop), index(rowCount()-1, ColOnDesktop));
    }
}

void AppListModel::setPinnedApps(const QStringList &pinned) {
    m_pinnedApps = pinned;
    if (rowCount() > 0) {
        emit dataChanged(index(0, ColPinned), index(rowCount()-1, ColPinned));
    }
}

int AppListModel::rowCount(const QModelIndex &) const {
    return m_entries.size();
}

int AppListModel::columnCount(const QModelIndex &) const {
    return ColCount;
}

QVariant AppListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const DesktopEntry &e = m_entries[index.row()];
    bool isOnDesk = m_desktopNames.contains(e.name().trimmed().toLower()) ||
                    (!e.exec().isEmpty() && m_desktopNames.contains(e.exec().section(' ', 0, 0).trimmed().toLower()));

    if (role == Qt::DecorationRole && index.column() == ColIcon) {
        return e.resolveIcon().pixmap(32, 32);
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColName:      return e.name();
        case ColCategory: {
            QString cats = e.categories();
            // Show first non-empty category
            for (const QString &cat : cats.split(';', Qt::SkipEmptyParts)) {
                if (!cat.isEmpty()) return cat;
            }
            return QString("-");
        }
        case ColOnDesktop: return isOnDesk ? "✓" : "";
        case ColPinned:    return m_pinnedApps.contains(e.name()) ? "★" : "☆";
        default: break;
        }
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColOnDesktop && isOnDesk)
            return QColor("#4ade80"); // green
        if (index.column() == ColPinned) {
            if (m_pinnedApps.contains(e.name()))
                return QColor("#fbbf24"); // gold
            return QColor("#6b7280"); // gray
        }
    }

    if (role == Qt::FontRole && index.column() == ColPinned) {
        QFont f;
        f.setPointSize(14);
        return f;
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColOnDesktop || index.column() == ColPinned)
            return Qt::AlignCenter;
    }

    if (role == Qt::UserRole) {
        // Return full DesktopEntry as QVariant via pointer
        return QVariant::fromValue(index.row());
    }

    return {};
}

QVariant AppListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section) {
    case ColIcon:      return "";
    case ColName:      return "Application";
    case ColCategory:  return "Category";
    case ColOnDesktop: return "Desktop";
    case ColPinned:    return "Pin";
    default: return {};
    }
}

DesktopEntry AppListModel::entryAt(int row) const {
    if (row < 0 || row >= m_entries.size()) return {};
    return m_entries[row];
}

int AppListModel::findEntry(const QString &name) const {
    for (int i = 0; i < m_entries.size(); ++i)
        if (m_entries[i].name() == name) return i;
    return -1;
}
