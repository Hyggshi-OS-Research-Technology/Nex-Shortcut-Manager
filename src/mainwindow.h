#pragma once

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include "desktopentry.h"
#include "desktopscanner.h"
#include "shortcutmanager.h"
#include "applistmodel.h"
#include "importexportmanager.h"

class QTableView;
class QLineEdit;
class QLabel;
class QPushButton;
class QListWidget;
class QStackedWidget;
class QComboBox;

class AppFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum ViewFilter {
        FilterAll = 0,
        FilterPinned = 1,
        FilterDesktop = 2
    };

    explicit AppFilterProxyModel(QObject *parent = nullptr);

    void setViewFilter(ViewFilter filter);
    void setShortcutManager(ShortcutManager *manager);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    ViewFilter m_viewFilter = FilterAll;
    ShortcutManager *m_manager = nullptr;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onScanFinished(const QList<DesktopEntry> &entries);
    void onSearchTextChanged(const QString &text);
    void onCategoryFilterChanged(const QString &category);
    void onSidebarItemChanged(int row);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTableDoubleClicked(const QModelIndex &proxyIndex);
    void onContextMenuRequested(const QPoint &pos);

    // Core Actions
    void onAddToDesktop();
    void onRemoveFromDesktop();
    void onRefreshAll();
    void onRefreshDesktopIcons();
    void onEditShortcut();
    void onQuickRename();
    void onChangeIcon();
    void onTogglePin();
    void onLaunchApp();
    void onOpenLocation();
    void onValidateSelected();

    // Creation Actions
    void onCreateCustomShortcut();
    void onCreateFolderShortcut();
    void onCreateFileShortcut();

    // Import/Export
    void onExportShortcuts();
    void onImportShortcuts();

    // About
    void onShowAbout();

private:
    void setupUi();
    void setupConnections();
    void setupStyle();
    void updateStatusBar();
    void updateActionButtons();
    DesktopEntry getSelectedEntry(int *sourceRowOut = nullptr) const;

    // Components
    DesktopScanner *m_scanner;
    ShortcutManager *m_shortcutManager;
    ImportExportManager *m_ioManager;

    AppListModel *m_model;
    AppFilterProxyModel *m_proxyModel;

    // UI elements
    QTableView *m_tableView;
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryCombo;
    QListWidget *m_sidebar;
    QLabel *m_statusLabel;
    QLabel *m_badgeAll;
    QLabel *m_badgePinned;
    QLabel *m_badgeDesktop;

    // Bottom action buttons
    QPushButton *m_btnAddToDesktop;
    QPushButton *m_btnRemoveFromDesktop;
    QPushButton *m_btnEdit;
    QPushButton *m_btnLaunch;
    QPushButton *m_btnPin;
    QPushButton *m_btnCustom;
    QPushButton *m_btnFolder;
    QPushButton *m_btnFile;
};
