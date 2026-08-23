#include "iconpickerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QIcon>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QPixmap>
#include <QTimer>

IconPickerDialog::IconPickerDialog(const QString &currentIcon, QWidget *parent)
    : QDialog(parent)
    , m_selectedIcon(currentIcon)
{
    setupUi();
    setWindowTitle("Choose Icon");
    resize(600, 450);

    // Populate after show (slight delay for smooth open)
    QTimer::singleShot(50, this, [this]() {
        populateIcons();
    });
}

void IconPickerDialog::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Search bar
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("🔍 Search icons...");
    m_searchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(m_searchEdit);

    // Preview + list
    auto *contentLayout = new QHBoxLayout;

    m_iconList = new QListWidget;
    m_iconList->setViewMode(QListWidget::IconMode);
    m_iconList->setIconSize(QSize(36, 36));
    m_iconList->setGridSize(QSize(60, 60));
    m_iconList->setSpacing(4);
    m_iconList->setResizeMode(QListWidget::Adjust);
    m_iconList->setUniformItemSizes(true);
    m_iconList->setObjectName("iconGrid");

    m_previewLabel = new QLabel;
    m_previewLabel->setFixedSize(100, 120);
    m_previewLabel->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    m_previewLabel->setObjectName("iconPreviewLarge");
    m_previewLabel->setWordWrap(true);

    contentLayout->addWidget(m_iconList, 1);
    contentLayout->addWidget(m_previewLabel);
    mainLayout->addLayout(contentLayout, 1);

    // Browse + buttons
    auto *btnRow = new QHBoxLayout;
    auto *browseBtn = new QPushButton("Browse File...");
    browseBtn->setObjectName("secondaryBtn");
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnRow->addWidget(browseBtn);
    btnRow->addStretch();
    btnRow->addWidget(buttons);
    mainLayout->addLayout(btnRow);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &IconPickerDialog::onSearch);
    connect(browseBtn, &QPushButton::clicked, this, &IconPickerDialog::onBrowseFile);
    connect(m_iconList, &QListWidget::itemClicked, this, &IconPickerDialog::onItemSelected);
    connect(m_iconList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        onItemSelected(item);
        accept();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void IconPickerDialog::populateIcons(const QString &filter) {
    m_iconList->clear();

    // Common system theme icons
    static const QStringList commonIcons = {
        "accessories-calculator", "accessories-text-editor", "address-book-new",
        "application-exit", "application-x-executable", "applications-games",
        "applications-graphics", "applications-internet", "applications-multimedia",
        "applications-office", "applications-other", "applications-system",
        "applications-utilities", "audio-card", "audio-headphones",
        "audio-input-microphone", "audio-speakers", "audio-volume-high",
        "battery", "bluetooth", "camera-photo", "camera-video",
        "computer", "dialog-error", "dialog-information", "dialog-warning",
        "document-new", "document-open", "document-print", "document-save",
        "drive-harddisk", "drive-optical", "drive-removable-media",
        "edit-copy", "edit-cut", "edit-delete", "edit-find", "edit-paste",
        "emblem-favorite", "emblem-important", "emblem-symbolic-link",
        "face-smile", "file-manager", "folder", "folder-download",
        "folder-home", "folder-music", "folder-pictures", "folder-videos",
        "format-text-bold", "go-home", "go-next", "go-previous", "go-up",
        "help-browser", "image-viewer", "input-keyboard", "input-mouse",
        "internet-news-reader", "internet-web-browser",
        "mail-send", "media-flash", "media-optical", "media-playback-start",
        "media-playback-pause", "media-playback-stop", "media-record",
        "multimedia-player", "network-idle", "network-offline", "network-wireless",
        "office-calendar", "package-x-generic", "phone", "preferences-desktop",
        "preferences-system", "printer", "process-stop", "scanner",
        "system-file-manager", "system-log-out", "system-run", "system-search",
        "system-shutdown", "system-software-install", "system-software-update",
        "system-users", "terminal", "text-editor", "text-x-generic",
        "trash-empty", "trash-full", "user-home", "utilities-terminal",
        "video-display", "view-refresh", "weather-clear", "web-browser",
        "window-close", "window-new", "zoom-fit-best", "zoom-in", "zoom-out"
    };

    QString lowerFilter = filter.toLower();
    int count = 0;

    for (const QString &iconName : commonIcons) {
        if (!lowerFilter.isEmpty() && !iconName.contains(lowerFilter))
            continue;

        QIcon icon = QIcon::fromTheme(iconName);
        if (icon.isNull()) continue;

        auto *item = new QListWidgetItem(icon, "");
        item->setData(Qt::UserRole, iconName);
        item->setToolTip(iconName);
        m_iconList->addItem(item);
        count++;
        if (count > 200) break; // limit for performance
    }

    // Also scan hicolor icon dirs if filter is set
    if (!lowerFilter.isEmpty()) {
        QStringList iconDirs = {
            "/usr/share/icons/hicolor/48x48/apps/",
            "/usr/share/pixmaps/"
        };
        for (const QString &dir : iconDirs) {
            QDirIterator it(dir, QStringList() << "*.png" << "*.svg",
                            QDir::Files);
            while (it.hasNext() && count < 200) {
                it.next();
                QString base = it.fileInfo().completeBaseName();
                if (!base.contains(lowerFilter)) continue;
                QIcon icon(it.filePath());
                if (icon.isNull()) continue;
                auto *item = new QListWidgetItem(icon, "");
                item->setData(Qt::UserRole, base);
                item->setToolTip(base);
                m_iconList->addItem(item);
                count++;
            }
        }
    }
}

void IconPickerDialog::onSearch(const QString &query) {
    populateIcons(query);
}

void IconPickerDialog::onBrowseFile() {
    QString file = QFileDialog::getOpenFileName(this, "Select Icon File",
                                                 "/usr/share/icons",
                                                 "Images (*.png *.svg *.xpm *.ico)");
    if (!file.isEmpty()) {
        m_selectedIcon = file;
        m_previewLabel->setPixmap(QIcon(file).pixmap(80, 80));
        m_previewLabel->setText("<br><small>" + QFileInfo(file).fileName() + "</small>");
        accept();
    }
}

void IconPickerDialog::onItemSelected(QListWidgetItem *item) {
    if (!item) return;
    m_selectedIcon = item->data(Qt::UserRole).toString();
    QIcon icon = QIcon::fromTheme(m_selectedIcon);
    if (icon.isNull()) icon = QIcon(m_selectedIcon);
    m_previewLabel->setPixmap(icon.pixmap(80, 80));
    m_previewLabel->setText("<br><small>" + m_selectedIcon + "</small>");
}

QString IconPickerDialog::selectedIcon() const {
    return m_selectedIcon;
}
