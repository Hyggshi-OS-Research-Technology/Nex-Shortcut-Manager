#include "mainwindow.h"
#include "editdialog.h"
#include "customshortcutdialog.h"
#include "iconpickerdialog.h"
#include "aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QToolBar>
#include <QApplication>
#include <QShortcut>
#include <QKeySequence>

// -------------------------------------------------------------
// AppFilterProxyModel
// -------------------------------------------------------------
AppFilterProxyModel::AppFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void AppFilterProxyModel::setViewFilter(ViewFilter filter) {
    m_viewFilter = filter;
    invalidate();
}

void AppFilterProxyModel::setShortcutManager(ShortcutManager *manager) {
    m_manager = manager;
    invalidate();
}

bool AppFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    auto *model = qobject_cast<AppListModel *>(sourceModel());
    if (!model) return true;

    DesktopEntry entry = model->entryAt(sourceRow);

    // Sidebar filter
    if (m_manager) {
        if (m_viewFilter == FilterPinned && !m_manager->isPinned(entry.name()))
            return false;
        if (m_viewFilter == FilterDesktop && !m_manager->isOnDesktop(entry))
            return false;
    }

    // Search query filter across Name, Exec, Categories, Comment
    QString query = filterRegularExpression().pattern();
    if (!query.isEmpty()) {
        bool match = entry.name().contains(query, Qt::CaseInsensitive)
                  || entry.exec().contains(query, Qt::CaseInsensitive)
                  || entry.categories().contains(query, Qt::CaseInsensitive)
                  || entry.comment().contains(query, Qt::CaseInsensitive);
        if (!match) return false;
    }

    return true;
}

// -------------------------------------------------------------
// MainWindow
// -------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scanner(new DesktopScanner(this))
    , m_shortcutManager(new ShortcutManager(this))
    , m_ioManager(new ImportExportManager(this))
    , m_model(new AppListModel(this))
    , m_proxyModel(new AppFilterProxyModel(this))
{
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setShortcutManager(m_shortcutManager);

    setupUi();
    setupConnections();
    setupStyle();

    // Start initial scan
    m_scanner->scan();
    updateStatusBar();
}

void MainWindow::setupUi() {
    setWindowTitle("Nex Shortcut Manager (NSM) — Desktop App & Launcher Manager");
    resize(1100, 700);
    setMinimumSize(850, 500);

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // 1. Top Header Bar
    auto *topBarLayout = new QHBoxLayout();
    topBarLayout->setSpacing(10);

    auto *appTitle = new QLabel("⚡ Nex Shortcut Manager (NSM)");
    appTitle->setObjectName("appTitle");
    appTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #60a5fa;");
    topBarLayout->addWidget(appTitle);

    topBarLayout->addSpacing(16);

    // Search Bar
    m_searchEdit = new QLineEdit();
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("🔍 Search apps by name, command or category...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(320);
    topBarLayout->addWidget(m_searchEdit, 1);

    // Quick Action Buttons in Top Bar
    auto *btnScan = new QPushButton("🔄 Refresh");
    btnScan->setObjectName("secondaryBtn");
    btnScan->setToolTip("Rescan /usr/share/applications and desktop shortcuts (F5)");
    connect(btnScan, &QPushButton::clicked, this, &MainWindow::onRefreshAll);
    topBarLayout->addWidget(btnScan);

    auto *btnImport = new QPushButton("📥 Import");
    btnImport->setObjectName("secondaryBtn");
    btnImport->setToolTip("Import shortcut list from JSON");
    connect(btnImport, &QPushButton::clicked, this, &MainWindow::onImportShortcuts);
    topBarLayout->addWidget(btnImport);

    auto *btnExport = new QPushButton("📤 Export");
    btnExport->setObjectName("secondaryBtn");
    btnExport->setToolTip("Export shortcuts list to JSON file");
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportShortcuts);
    topBarLayout->addWidget(btnExport);

    auto *btnAbout = new QPushButton("ℹ️ About");
    btnAbout->setObjectName("secondaryBtn");
    btnAbout->setToolTip("About Nex Shortcut Manager (NSM) & System Info (F1)");
    connect(btnAbout, &QPushButton::clicked, this, &MainWindow::onShowAbout);
    topBarLayout->addWidget(btnAbout);

    // Keyboard shortcuts
    auto *f1Shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect(f1Shortcut, &QShortcut::activated, this, &MainWindow::onShowAbout);

    auto *f5Shortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(f5Shortcut, &QShortcut::activated, this, &MainWindow::onRefreshAll);

    mainLayout->addLayout(topBarLayout);

    // 2. Main Content Splitter (Sidebar + Table)
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(2);

    // Left Sidebar
    auto *sidebarContainer = new QWidget();
    sidebarContainer->setFixedWidth(230);
    auto *sidebarLayout = new QVBoxLayout(sidebarContainer);
    sidebarLayout->setContentsMargins(0, 0, 8, 0);
    sidebarLayout->setSpacing(8);

    auto *lblViews = new QLabel("CATEGORIES");
    lblViews->setStyleSheet("font-size: 11px; font-weight: 700; color: #9ca3af; letter-spacing: 1px;");
    sidebarLayout->addWidget(lblViews);

    m_sidebar = new QListWidget();
    m_sidebar->setObjectName("sidebarList");

    auto *itemAll = new QListWidgetItem("📦  All Applications");
    auto *itemPinned = new QListWidgetItem("⭐  Pinned Apps");
    auto *itemDesktop = new QListWidgetItem("🖥️  Desktop Shortcuts");

    m_sidebar->addItem(itemAll);
    m_sidebar->addItem(itemPinned);
    m_sidebar->addItem(itemDesktop);
    m_sidebar->setCurrentRow(0);
    sidebarLayout->addWidget(m_sidebar);

    // Quick creation panel in sidebar
    auto *lblCreate = new QLabel("QUICK CREATE");
    lblCreate->setStyleSheet("font-size: 11px; font-weight: 700; color: #9ca3af; letter-spacing: 1px;");
    sidebarLayout->addWidget(lblCreate);

    m_btnCustom = new QPushButton("✨ Custom App Shortcut");
    m_btnCustom->setObjectName("actionBtnPrimary");
    connect(m_btnCustom, &QPushButton::clicked, this, &MainWindow::onCreateCustomShortcut);
    sidebarLayout->addWidget(m_btnCustom);

    m_btnFolder = new QPushButton("📁 Folder Shortcut");
    m_btnFolder->setObjectName("secondaryBtn");
    connect(m_btnFolder, &QPushButton::clicked, this, &MainWindow::onCreateFolderShortcut);
    sidebarLayout->addWidget(m_btnFolder);

    m_btnFile = new QPushButton("📄 File Shortcut");
    m_btnFile->setObjectName("secondaryBtn");
    connect(m_btnFile, &QPushButton::clicked, this, &MainWindow::onCreateFileShortcut);
    sidebarLayout->addWidget(m_btnFile);

    sidebarLayout->addStretch();
    splitter->addWidget(sidebarContainer);

    // Right Table View
    auto *tableContainer = new QWidget();
    auto *tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(8, 0, 0, 0);
    tableLayout->setSpacing(10);

    m_tableView = new QTableView();
    m_tableView->setModel(m_proxyModel);
    m_tableView->setObjectName("appTable");
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setShowGrid(false);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(44);
    m_tableView->horizontalHeader()->setStretchLastSection(false);
    m_tableView->setSortingEnabled(true);

    // Column widths
    m_tableView->setColumnWidth(AppListModel::ColIcon, 56);
    m_tableView->horizontalHeader()->setSectionResizeMode(AppListModel::ColIcon, QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setSectionResizeMode(AppListModel::ColName, QHeaderView::Stretch);
    m_tableView->setColumnWidth(AppListModel::ColCategory, 180);
    m_tableView->setColumnWidth(AppListModel::ColOnDesktop, 100);
    m_tableView->setColumnWidth(AppListModel::ColPinned, 70);

    tableLayout->addWidget(m_tableView);

    // 3. Bottom Action Bar
    auto *bottomBarLayout = new QHBoxLayout();
    bottomBarLayout->setSpacing(8);

    m_btnAddToDesktop = new QPushButton("🖥️ Add to Desktop");
    m_btnAddToDesktop->setObjectName("successBtn");
    m_btnAddToDesktop->setEnabled(false);
    bottomBarLayout->addWidget(m_btnAddToDesktop);

    m_btnRemoveFromDesktop = new QPushButton("🗑️ Remove from Desktop");
    m_btnRemoveFromDesktop->setObjectName("dangerBtn");
    m_btnRemoveFromDesktop->setEnabled(false);
    bottomBarLayout->addWidget(m_btnRemoveFromDesktop);

    m_btnEdit = new QPushButton("✏️ Edit Shortcut");
    m_btnEdit->setObjectName("secondaryBtn");
    m_btnEdit->setEnabled(false);
    bottomBarLayout->addWidget(m_btnEdit);

    m_btnPin = new QPushButton("⭐ Pin / Unpin");
    m_btnPin->setObjectName("secondaryBtn");
    m_btnPin->setEnabled(false);
    bottomBarLayout->addWidget(m_btnPin);

    m_btnLaunch = new QPushButton("🚀 Launch App");
    m_btnLaunch->setObjectName("secondaryBtn");
    m_btnLaunch->setEnabled(false);
    bottomBarLayout->addWidget(m_btnLaunch);

    bottomBarLayout->addStretch();

    auto *btnRefreshDesktop = new QPushButton("🔄 Refresh Desktop");
    btnRefreshDesktop->setObjectName("secondaryBtn");
    btnRefreshDesktop->setToolTip("Tell Desktop Environment to reload desktop shortcuts");
    connect(btnRefreshDesktop, &QPushButton::clicked, this, &MainWindow::onRefreshDesktopIcons);
    bottomBarLayout->addWidget(btnRefreshDesktop);

    tableLayout->addLayout(bottomBarLayout);
    splitter->addWidget(tableContainer);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);

    // Status Bar
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #9ca3af; font-size: 12px;");
    statusBar()->addWidget(m_statusLabel, 1);
}

void MainWindow::setupConnections() {
    // Scanner
    connect(m_scanner, &DesktopScanner::scanFinished, this, &MainWindow::onScanFinished);

    // Search & Sidebar
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_sidebar, &QListWidget::currentRowChanged, this, &MainWindow::onSidebarItemChanged);

    // Table view
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onTableDoubleClicked);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onContextMenuRequested);

    // Action buttons
    connect(m_btnAddToDesktop, &QPushButton::clicked, this, &MainWindow::onAddToDesktop);
    connect(m_btnRemoveFromDesktop, &QPushButton::clicked, this, &MainWindow::onRemoveFromDesktop);
    connect(m_btnEdit, &QPushButton::clicked, this, &MainWindow::onEditShortcut);
    connect(m_btnPin, &QPushButton::clicked, this, &MainWindow::onTogglePin);
    connect(m_btnLaunch, &QPushButton::clicked, this, &MainWindow::onLaunchApp);

    // Shortcut Manager events
    connect(m_shortcutManager, &ShortcutManager::shortcutAdded, this, [this](const QString &) {
        m_model->setDesktopEntries(m_shortcutManager->desktopShortcuts());
        m_proxyModel->invalidate();
        updateStatusBar();
        updateActionButtons();
    });
    connect(m_shortcutManager, &ShortcutManager::shortcutRemoved, this, [this](const QString &) {
        m_model->setDesktopEntries(m_shortcutManager->desktopShortcuts());
        m_proxyModel->invalidate();
        updateStatusBar();
        updateActionButtons();
    });
    connect(m_shortcutManager, &ShortcutManager::pinnedChanged, this, [this]() {
        m_model->setPinnedApps(m_shortcutManager->pinnedApps());
        m_proxyModel->invalidate();
        updateStatusBar();
        updateActionButtons();
    });
}

void MainWindow::setupStyle() {
    // Set rich dark mode stylesheet with smooth UI
    QString qss = R"(
        QMainWindow, QWidget {
            background-color: #0f172a;
            color: #f1f5f9;
            font-family: 'Segoe UI', 'Inter', 'Ubuntu', 'Sans-Serif';
            font-size: 13px;
        }
        QLineEdit {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 8px 12px;
            selection-background-color: #3b82f6;
        }
        QLineEdit:focus {
            border: 1px solid #60a5fa;
            background-color: #1e293b;
        }
        QPushButton {
            border-radius: 6px;
            padding: 7px 14px;
            font-weight: 600;
        }
        QPushButton#actionBtnPrimary {
            background-color: #3b82f6;
            color: #ffffff;
            border: none;
        }
        QPushButton#actionBtnPrimary:hover {
            background-color: #2563eb;
        }
        QPushButton#secondaryBtn {
            background-color: #1e293b;
            color: #e2e8f0;
            border: 1px solid #334155;
        }
        QPushButton#secondaryBtn:hover {
            background-color: #334155;
            color: #ffffff;
        }
        QPushButton#successBtn {
            background-color: #10b981;
            color: #ffffff;
            border: none;
        }
        QPushButton#successBtn:hover {
            background-color: #059669;
        }
        QPushButton#dangerBtn {
            background-color: #ef4444;
            color: #ffffff;
            border: none;
        }
        QPushButton#dangerBtn:hover {
            background-color: #dc2626;
        }
        QPushButton:disabled {
            background-color: #1e293b;
            color: #475569;
            border: 1px solid #1e293b;
        }
        QListWidget#sidebarList {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 6px;
        }
        QListWidget#sidebarList::item {
            padding: 10px 12px;
            border-radius: 6px;
            margin-bottom: 3px;
            font-weight: 500;
        }
        QListWidget#sidebarList::item:hover {
            background-color: #334155;
        }
        QListWidget#sidebarList::item:selected {
            background-color: #2563eb;
            color: #ffffff;
            font-weight: 600;
        }
        QTableView#appTable {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 8px;
            gridline-color: transparent;
            selection-background-color: #1d4ed8;
            selection-color: #ffffff;
            outline: none;
        }
        QTableView#appTable::item {
            padding: 6px 10px;
            border-bottom: 1px solid #334155;
        }
        QTableView#appTable::item:selected {
            background-color: #1d4ed8;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 8px 10px;
            border: none;
            border-bottom: 2px solid #334155;
            font-weight: 700;
            text-transform: uppercase;
            font-size: 11px;
        }
        QMenu {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: #3b82f6;
            color: #ffffff;
        }
        QMenu::separator {
            height: 1px;
            background-color: #334155;
            margin: 4px 8px;
        }
        QStatusBar {
            background-color: #0f172a;
            border-top: 1px solid #1e293b;
        }
        QDialog {
            background-color: #0f172a;
            color: #f1f5f9;
        }
        QGroupBox {
            border: 1px solid #334155;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 12px;
            font-weight: bold;
            color: #60a5fa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QListWidget#iconGrid {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 6px;
        }
        QListWidget#iconGrid::item {
            border-radius: 6px;
            padding: 4px;
        }
        QListWidget#iconGrid::item:hover {
            background-color: #334155;
        }
        QListWidget#iconGrid::item:selected {
            background-color: #2563eb;
        }
    )";
    qApp->setStyleSheet(qss);
}

void MainWindow::onScanFinished(const QList<DesktopEntry> &entries) {
    m_model->setEntries(entries);
    m_model->setDesktopEntries(m_shortcutManager->desktopShortcuts());
    m_model->setPinnedApps(m_shortcutManager->pinnedApps());
    m_proxyModel->invalidate();

    updateStatusBar();
}

void MainWindow::onSearchTextChanged(const QString &text) {
    m_proxyModel->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(text),
                                                                 QRegularExpression::CaseInsensitiveOption));
    updateStatusBar();
}

void MainWindow::onCategoryFilterChanged(const QString &category) {
    // Reserved for dropdown if needed
}

void MainWindow::onSidebarItemChanged(int row) {
    m_proxyModel->setViewFilter(static_cast<AppFilterProxyModel::ViewFilter>(row));
    updateStatusBar();
}

void MainWindow::onSelectionChanged(const QItemSelection &, const QItemSelection &) {
    updateActionButtons();
}

DesktopEntry MainWindow::getSelectedEntry(int *sourceRowOut) const {
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return {};

    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

    if (sourceRowOut) *sourceRowOut = sourceIndex.row();
    return m_model->entryAt(sourceIndex.row());
}

void MainWindow::updateActionButtons() {
    DesktopEntry entry = getSelectedEntry();
    bool hasSelection = !entry.name().isEmpty();

    m_btnEdit->setEnabled(hasSelection);
    m_btnPin->setEnabled(hasSelection);
    m_btnLaunch->setEnabled(hasSelection);
    m_btnAddToDesktop->setEnabled(hasSelection);
    m_btnRemoveFromDesktop->setEnabled(hasSelection);

    if (hasSelection) {
        bool onDesktop = m_shortcutManager->isOnDesktop(entry);
        m_btnAddToDesktop->setText(onDesktop ? "🖥️ Re-add to Desktop" : "🖥️ Add to Desktop");

        if (onDesktop) {
            m_btnRemoveFromDesktop->setText("🗑️ Remove from Desktop");
            m_btnRemoveFromDesktop->setToolTip("Remove " + entry.name() + " from Desktop");
        } else if (entry.filePath().contains("/.local/share/applications")) {
            m_btnRemoveFromDesktop->setText("🗑️ Delete Custom App");
            m_btnRemoveFromDesktop->setToolTip("Delete custom shortcut from ~/.local/share/applications");
        } else {
            m_btnRemoveFromDesktop->setText("🗑️ Remove from Desktop");
            m_btnRemoveFromDesktop->setToolTip("Remove shortcut from Desktop");
        }

        bool isPinned = m_shortcutManager->isPinned(entry.name());
        m_btnPin->setText(isPinned ? "★ Unpin" : "☆ Pin");
    } else {
        m_btnAddToDesktop->setEnabled(false);
        m_btnRemoveFromDesktop->setEnabled(false);
    }
}

void MainWindow::updateStatusBar() {
    int total = m_model->rowCount();
    int visible = m_proxyModel->rowCount();
    int desktopCount = m_shortcutManager->desktopShortcuts().size();
    int pinnedCount = m_shortcutManager->pinnedApps().size();

    m_statusLabel->setText(QString("Showing %1 of %2 applications | 🖥️ %3 on Desktop | ⭐ %4 Pinned | Desktop: %5")
                               .arg(visible)
                               .arg(total)
                               .arg(desktopCount)
                               .arg(pinnedCount)
                               .arg(m_shortcutManager->desktopPath()));
}

void MainWindow::onTableDoubleClicked(const QModelIndex &proxyIndex) {
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    DesktopEntry entry = m_model->entryAt(sourceIndex.row());
    if (entry.name().isEmpty()) return;

    if (!m_shortcutManager->isOnDesktop(entry)) {
        onAddToDesktop();
    } else {
        onLaunchApp();
    }
}

void MainWindow::onContextMenuRequested(const QPoint &pos) {
    QModelIndex index = m_tableView->indexAt(pos);
    if (!index.isValid()) return;

    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    bool onDesktop = m_shortcutManager->isOnDesktop(entry);
    bool isPinned = m_shortcutManager->isPinned(entry.name());

    QMenu menu(this);

    auto *actLaunch = menu.addAction("🚀 Launch Application");
    connect(actLaunch, &QAction::triggered, this, &MainWindow::onLaunchApp);

    menu.addSeparator();

    auto *actAdd = menu.addAction(onDesktop ? "🖥️ Re-add to Desktop" : "🖥️ Add to Desktop");
    connect(actAdd, &QAction::triggered, this, &MainWindow::onAddToDesktop);

    if (onDesktop) {
        auto *actRemove = menu.addAction("🗑️ Remove from Desktop");
        connect(actRemove, &QAction::triggered, this, &MainWindow::onRemoveFromDesktop);
    } else if (entry.filePath().contains("/.local/share/applications")) {
        auto *actRemove = menu.addAction("🗑️ Delete Custom Shortcut");
        connect(actRemove, &QAction::triggered, this, &MainWindow::onRemoveFromDesktop);
    } else {
        auto *actRemove = menu.addAction("🗑️ Remove from Desktop");
        connect(actRemove, &QAction::triggered, this, &MainWindow::onRemoveFromDesktop);
    }

    auto *actPin = menu.addAction(isPinned ? "★ Unpin from Favorites" : "☆ Pin to Favorites");
    connect(actPin, &QAction::triggered, this, &MainWindow::onTogglePin);

    menu.addSeparator();

    auto *actEdit = menu.addAction("⚙️ Full Edit Properties...");
    connect(actEdit, &QAction::triggered, this, &MainWindow::onEditShortcut);

    auto *actRename = menu.addAction("✏️ Quick Rename...");
    connect(actRename, &QAction::triggered, this, &MainWindow::onQuickRename);

    auto *actChangeIcon = menu.addAction("🖼️ Change Icon...");
    connect(actChangeIcon, &QAction::triggered, this, &MainWindow::onChangeIcon);

    auto *actValidate = menu.addAction("🛡️ Validate .desktop");
    connect(actValidate, &QAction::triggered, this, &MainWindow::onValidateSelected);

    menu.addSeparator();

    auto *actOpenLoc = menu.addAction("📂 Open File Location");
    connect(actOpenLoc, &QAction::triggered, this, &MainWindow::onOpenLocation);

    menu.exec(m_tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onAddToDesktop() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    if (m_shortcutManager->addToDesktop(entry)) {
        m_scanner->scan();
        statusBar()->showMessage("Added shortcut to Desktop: " + entry.name(), 4000);
    } else {
        QMessageBox::critical(this, "Error", "Failed to create desktop shortcut for " + entry.name());
    }
}

void MainWindow::onRemoveFromDesktop() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    QString path = m_shortcutManager->desktopFilePath(entry);
    bool onDesktop = QFile::exists(path);

    if (onDesktop) {
        auto reply = QMessageBox::question(this, "Confirm Removal",
                                           "Are you sure you want to remove the shortcut for '" + entry.name() + "' from Desktop?\n\nFile: " + path,
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (m_shortcutManager->removeFromDesktop(path)) {
                m_scanner->scan();
                statusBar()->showMessage("Removed shortcut from Desktop: " + entry.name(), 4000);
            } else {
                QMessageBox::critical(this, "Error", "Failed to delete desktop file:\n" + path);
            }
        }
        return;
    }

    // Check if it's a custom shortcut in user apps folder
    if (!entry.filePath().isEmpty() && entry.filePath().contains("/.local/share/applications") && QFile::exists(entry.filePath())) {
        auto reply = QMessageBox::question(this, "Delete Custom Shortcut",
                                           "This shortcut is in your user applications folder (~/.local/share/applications).\n\n"
                                           "Do you want to permanently delete '" + entry.name() + "'?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile::remove(entry.filePath());
            m_scanner->scan();
            statusBar()->showMessage("Deleted custom shortcut: " + entry.name(), 4000);
        }
        return;
    }

    // System application not on Desktop
    QMessageBox::information(this, "Not on Desktop",
                             "'" + entry.name() + "' does not currently have a shortcut on your Desktop.\n\n"
                             "Click 'Add to Desktop' if you want to place a shortcut on your Desktop.");
}

void MainWindow::onRefreshAll() {
    m_scanner->scan();
    statusBar()->showMessage("Refreshed applications list.", 3000);
}

void MainWindow::onRefreshDesktopIcons() {
    m_shortcutManager->refreshDesktop();
    m_model->setDesktopEntries(m_shortcutManager->desktopShortcuts());
    m_proxyModel->invalidate();
    updateStatusBar();
    statusBar()->showMessage("Desktop refreshed.", 3000);
}

void MainWindow::onEditShortcut() {
    int sourceRow = -1;
    DesktopEntry entry = getSelectedEntry(&sourceRow);
    if (entry.name().isEmpty()) return;

    EditDialog dlg(entry, this);
    if (dlg.exec() == QDialog::Accepted) {
        DesktopEntry updated = dlg.result();
        // If it's already on desktop, update the desktop file directly
        if (m_shortcutManager->isOnDesktop(entry)) {
            m_shortcutManager->removeFromDesktop(m_shortcutManager->desktopFilePath(entry));
            m_shortcutManager->addToDesktop(updated);
        }
        // Save to user applications folder if user wants or save back
        QString userAppDir = QDir::homePath() + "/.local/share/applications";
        QDir().mkpath(userAppDir);
        QString userFilePath = userAppDir + "/" + QFileInfo(entry.filePath()).fileName();
        if (userFilePath.endsWith(".desktop")) {
            updated.save(userFilePath);
        }

        m_scanner->scan();
        statusBar()->showMessage("Updated shortcut: " + updated.name(), 4000);
    }
}

void MainWindow::onQuickRename() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    bool ok = false;
    QString newName = QInputDialog::getText(this, "Rename Shortcut",
                                            "Enter new name for '" + entry.name() + "':",
                                            QLineEdit::Normal, entry.name(), &ok);
    if (ok && !newName.trimmed().isEmpty() && newName != entry.name()) {
        bool wasOnDesktop = m_shortcutManager->isOnDesktop(entry);
        if (wasOnDesktop) {
            m_shortcutManager->removeFromDesktop(m_shortcutManager->desktopFilePath(entry));
        }

        entry.setName(newName.trimmed());
        if (wasOnDesktop) {
            m_shortcutManager->addToDesktop(entry);
        }

        QString userAppDir = QDir::homePath() + "/.local/share/applications";
        QDir().mkpath(userAppDir);
        entry.save(userAppDir + "/" + newName.trimmed() + ".desktop");

        m_scanner->scan();
        statusBar()->showMessage("Renamed to: " + newName, 4000);
    }
}

void MainWindow::onChangeIcon() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    IconPickerDialog dlg(entry.icon(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString newIcon = dlg.selectedIcon();
        if (!newIcon.isEmpty() && newIcon != entry.icon()) {
            bool wasOnDesktop = m_shortcutManager->isOnDesktop(entry);
            entry.setIcon(newIcon);

            if (wasOnDesktop) {
                m_shortcutManager->addToDesktop(entry);
            }

            QString userAppDir = QDir::homePath() + "/.local/share/applications";
            QDir().mkpath(userAppDir);
            entry.save(userAppDir + "/" + entry.name() + ".desktop");

            m_scanner->scan();
            statusBar()->showMessage("Updated icon for: " + entry.name(), 4000);
        }
    }
}

void MainWindow::onTogglePin() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    if (m_shortcutManager->isPinned(entry.name())) {
        m_shortcutManager->unpinApp(entry.name());
        statusBar()->showMessage("Unpinned: " + entry.name(), 3000);
    } else {
        m_shortcutManager->pinApp(entry.name());
        statusBar()->showMessage("Pinned to favorites: " + entry.name(), 3000);
    }
}

void MainWindow::onLaunchApp() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    QString exec = entry.exec();
    // Clean desktop spec field codes (%f, %F, %u, %U, etc.)
    exec.remove(QRegularExpression("%[fFuUdDnNickvm]"));
    exec = exec.trimmed();

    if (exec.isEmpty()) {
        QMessageBox::warning(this, "Launch Error", "No executable command defined.");
        return;
    }

    if (entry.terminal()) {
        // Try launching inside terminal emulator
        QStringList terminals = {"gnome-terminal", "konsole", "xfce4-terminal", "xterm", "kitty", "alacritty"};
        bool launched = false;
        for (const QString &term : terminals) {
            if (QProcess::startDetached(term, QStringList() << "--" << "bash" << "-c" << exec + "; exec bash")) {
                launched = true;
                break;
            }
        }
        if (!launched) {
            QProcess::startDetached("sh", QStringList() << "-c" << exec);
        }
    } else {
        QProcess::startDetached("/bin/sh", QStringList() << "-c" << exec);
    }
    statusBar()->showMessage("Launched: " + entry.name(), 3000);
}

void MainWindow::onOpenLocation() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    QString path = entry.filePath();
    if (m_shortcutManager->isOnDesktop(entry)) {
        path = m_shortcutManager->desktopFilePath(entry);
    }

    if (QFile::exists(path)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
    }
}

void MainWindow::onValidateSelected() {
    DesktopEntry entry = getSelectedEntry();
    if (entry.name().isEmpty()) return;

    QStringList errors = entry.validate();
    if (errors.isEmpty()) {
        QMessageBox::information(this, "Validation Passed",
                                 "✓ Valid .desktop entry!\n\n"
                                 "• Name: " + entry.name() + "\n"
                                 "• Type: " + entry.type() + "\n"
                                 "• Exec: " + entry.exec() + "\n"
                                 "• Icon: " + entry.icon());
    } else {
        QMessageBox::warning(this, "Validation Issues Found",
                             "✗ The desktop entry has issues:\n\n• " + errors.join("\n• "));
    }
}

void MainWindow::onCreateCustomShortcut() {
    CustomShortcutDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        DesktopEntry entry = dlg.result();
        // Ask if add to desktop directly
        auto reply = QMessageBox::question(this, "Add to Desktop?",
                                           "Would you like to place this shortcut directly onto your Desktop?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            m_shortcutManager->addToDesktop(entry);
        }

        // Also save to user applications
        QString userAppDir = QDir::homePath() + "/.local/share/applications";
        QDir().mkpath(userAppDir);
        entry.save(userAppDir + "/" + entry.name() + ".desktop");

        m_scanner->scan();
        statusBar()->showMessage("Created custom shortcut: " + entry.name(), 4000);
    }
}

void MainWindow::onCreateFolderShortcut() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder for Shortcut", QDir::homePath());
    if (dir.isEmpty()) return;

    DesktopEntry entry = DesktopEntry::createFolderShortcut(dir);
    if (m_shortcutManager->addToDesktop(entry)) {
        statusBar()->showMessage("Created folder shortcut on Desktop: " + entry.name(), 4000);
    }
}

void MainWindow::onCreateFileShortcut() {
    QString file = QFileDialog::getOpenFileName(this, "Select File for Shortcut", QDir::homePath());
    if (file.isEmpty()) return;

    DesktopEntry entry = DesktopEntry::createFileShortcut(file);
    if (m_shortcutManager->addToDesktop(entry)) {
        statusBar()->showMessage("Created file shortcut on Desktop: " + entry.name(), 4000);
    }
}

void MainWindow::onExportShortcuts() {
    QString savePath = QFileDialog::getSaveFileName(this, "Export Shortcuts",
                                                    QDir::homePath() + "/shortcuts_backup.json",
                                                    "JSON files (*.json)");
    if (savePath.isEmpty()) return;

    QList<DesktopEntry> desktopShortcuts = m_shortcutManager->desktopShortcuts();
    if (desktopShortcuts.isEmpty()) {
        desktopShortcuts = m_scanner->entries();
    }

    if (m_ioManager->exportToJson(desktopShortcuts, savePath)) {
        QMessageBox::information(this, "Export Successful",
                                 QString("Successfully exported %1 shortcuts to:\n%2")
                                     .arg(desktopShortcuts.size())
                                     .arg(savePath));
    } else {
        QMessageBox::critical(this, "Export Failed", m_ioManager->lastError());
    }
}

void MainWindow::onImportShortcuts() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Shortcuts JSON",
                                                    QDir::homePath(),
                                                    "JSON files (*.json)");
    if (filePath.isEmpty()) return;

    QList<DesktopEntry> imported = m_ioManager->importFromJson(filePath);
    if (imported.isEmpty()) {
        QMessageBox::warning(this, "Import Result",
                             "No valid shortcuts found in JSON file or file was empty.");
        return;
    }

    int count = 0;
    for (const DesktopEntry &e : imported) {
        if (e.isValid()) {
            m_shortcutManager->addToDesktop(e);
            count++;
        }
    }

    m_scanner->scan();
    QMessageBox::information(this, "Import Complete",
                             QString("Successfully restored %1 shortcuts to Desktop!").arg(count));
}

void MainWindow::onShowAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}
