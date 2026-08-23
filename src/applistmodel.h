#pragma once
#include "desktopentry.h"
#include <QAbstractTableModel>
#include <QList>
#include <QSet>

class AppListModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { ColIcon = 0, ColName, ColCategory, ColOnDesktop, ColPinned, ColCount };

    explicit AppListModel(QObject *parent = nullptr);

    void setEntries(const QList<DesktopEntry> &entries);
    void setDesktopEntries(const QList<DesktopEntry> &shortcuts);
    void setPinnedApps(const QStringList &pinned);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    DesktopEntry entryAt(int row) const;
    int findEntry(const QString &name) const;

private:
    QList<DesktopEntry> m_entries;
    QSet<QString> m_desktopNames;  // names of shortcuts on desktop
    QStringList m_pinnedApps;
};
