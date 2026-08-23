#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QApplication>
#include <QSysInfo>
#include <QDir>
#include <QIcon>
#include <QDialogButtonBox>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle("About Nex Shortcut Manager (NSM)");
    resize(580, 480);
    setFixedSize(580, 500);
}

void AboutDialog::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 20);

    // 1. Header with App Icon, Name, Version Badge
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    auto *iconLabel = new QLabel();
    QIcon appIcon = QIcon::fromTheme("preferences-desktop", QIcon::fromTheme("system-run"));
    iconLabel->setPixmap(appIcon.pixmap(64, 64));
    iconLabel->setFixedSize(64, 64);
    headerLayout->addWidget(iconLabel);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);

    auto *titleRow = new QHBoxLayout();
    auto *nameLabel = new QLabel("Nex Shortcut Manager (NSM)");
    nameLabel->setStyleSheet("font-size: 20px; font-weight: 800; color: #60a5fa;");
    titleRow->addWidget(nameLabel);

    auto *versionBadge = new QLabel("v1.0.0");
    versionBadge->setStyleSheet("background-color: #1e3a8a; color: #93c5fd; font-size: 11px; font-weight: 700; "
                                "padding: 2px 8px; border-radius: 10px; border: 1px solid #3b82f6;");
    titleRow->addWidget(versionBadge);
    titleRow->addStretch();
    titleLayout->addLayout(titleRow);

    auto *subtitleLabel = new QLabel("Modern Linux Desktop Shortcut & Launcher Manager");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #94a3b8; font-weight: 500;");
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addLayout(titleLayout, 1);
    mainLayout->addLayout(headerLayout);

    // 2. Tab Widget with Details
    auto *tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #334155;
            background-color: #1e293b;
            border-radius: 8px;
            padding: 12px;
        }
        QTabBar::tab {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 8px 16px;
            margin-right: 4px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-weight: 600;
        }
        QTabBar::tab:selected {
            background-color: #1e293b;
            color: #60a5fa;
            border: 1px solid #334155;
            border-bottom: none;
        }
        QTabBar::tab:hover:!selected {
            background-color: #1e293b;
            color: #e2e8f0;
        }
    )");

    // Tab 1: Features
    auto *featuresWidget = new QWidget();
    auto *featuresLayout = new QVBoxLayout(featuresWidget);
    featuresLayout->setContentsMargins(8, 8, 8, 8);
    auto *featuresBrowser = new QTextBrowser();
    featuresBrowser->setStyleSheet("background: transparent; border: none; color: #cbd5e1; font-size: 13px;");
    featuresBrowser->setHtml(R"(
        <p><b>✨ Key Capabilities:</b></p>
        <ul style="line-height: 150%;">
            <li>📦 <b>Auto Scan:</b> Scans system and user applications automatically</li>
            <li>🖥️ <b>Desktop Integration:</b> Place executable and trusted shortcuts onto Desktop</li>
            <li>🗑️ <b>Safe Removal:</b> Clean removal of shortcuts and custom desktop entries</li>
            <li>✏️ <b>Property Editor:</b> Edit Name, Exec, Categories, Icon, Working Dir, Terminal mode</li>
            <li>🖼️ <b>Icon Picker:</b> Searchable visual system theme icon selector</li>
            <li>📁 <b>File & Folder Links:</b> Quick shortcut generator for folders and documents</li>
            <li>⭐ <b>Pinned Favorites:</b> Star your most frequently used apps</li>
            <li>🛡️ <b>Validator:</b> Freedesktop Desktop Entry compliance validator</li>
            <li>📥 <b>JSON Backup:</b> Easy Import and Export of your shortcut lists</li>
        </ul>
    )");
    featuresLayout->addWidget(featuresBrowser);
    tabWidget->addTab(featuresWidget, "✨ Features");

    // Tab 2: System Info
    auto *sysWidget = new QWidget();
    auto *sysLayout = new QVBoxLayout(sysWidget);
    sysLayout->setContentsMargins(8, 8, 8, 8);

    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");
    if (desktopEnv.isEmpty()) desktopEnv = "Unknown Desktop";
    QString sessionType = qgetenv("XDG_SESSION_TYPE");
    if (sessionType.isEmpty()) sessionType = "Unknown";
    QString desktopPath = qgetenv("XDG_DESKTOP_DIR");
    if (desktopPath.isEmpty()) desktopPath = QDir::homePath() + "/Desktop";

    auto *sysBrowser = new QTextBrowser();
    sysBrowser->setStyleSheet("background: transparent; border: none; color: #cbd5e1; font-size: 13px;");
    sysBrowser->setHtml(QString(R"(
        <p><b>💻 System & Runtime Environment:</b></p>
        <table cellpadding="4" style="line-height: 140%;">
            <tr><td><b>OS:</b></td><td>%1 (%2)</td></tr>
            <tr><td><b>Kernel / CPU:</b></td><td>%3 (%4)</td></tr>
            <tr><td><b>Desktop Environment:</b></td><td>%5 (%6)</td></tr>
            <tr><td><b>Desktop Path:</b></td><td><code>%7</code></td></tr>
            <tr><td><b>Qt Version:</b></td><td>%8 (Built with Qt %9)</td></tr>
            <tr><td><b>C++ Standard:</b></td><td>C++17</td></tr>
        </table>
    )").arg(QSysInfo::prettyProductName(),
            QSysInfo::productVersion(),
            QSysInfo::kernelType() + " " + QSysInfo::kernelVersion(),
            QSysInfo::currentCpuArchitecture(),
            desktopEnv,
            sessionType,
            desktopPath,
            qVersion(),
            QT_VERSION_STR));
    sysLayout->addWidget(sysBrowser);
    tabWidget->addTab(sysWidget, "💻 System Info");

    // Tab 3: License & Credits
    auto *creditsWidget = new QWidget();
    auto *creditsLayout = new QVBoxLayout(creditsWidget);
    creditsLayout->setContentsMargins(8, 8, 8, 8);
    auto *creditsBrowser = new QTextBrowser();
    creditsBrowser->setStyleSheet("background: transparent; border: none; color: #cbd5e1; font-size: 13px;");
    creditsBrowser->setHtml(R"(
        <p><b>📜 License:</b></p>
        <p>Released under the open-source <b>MIT License</b>.</p>
        <p><b>👥 Technologies:</b></p>
        <ul>
            <li>Built with <b>C++17</b> and <b>Qt 6 Framework</b>.</li>
            <li>Designed for Ubuntu, Debian, Fedora, Arch Linux, and all Freedesktop compliant Linux distros.</li>
        </ul>
        <p style="color: #94a3b8; font-size: 12px;">© 2026 Shortcut Manager contributors.</p>
    )");
    creditsLayout->addWidget(creditsBrowser);
    tabWidget->addTab(creditsWidget, "📜 License");

    mainLayout->addWidget(tabWidget, 1);

    // 3. Bottom Button Row
    auto *bottomRow = new QHBoxLayout();

    auto *btnAboutQt = new QPushButton("About Qt");
    btnAboutQt->setObjectName("secondaryBtn");
    btnAboutQt->setStyleSheet("background-color: #1e293b; color: #e2e8f0; border: 1px solid #334155; padding: 6px 12px; border-radius: 6px;");
    connect(btnAboutQt, &QPushButton::clicked, this, [this]() {
        QApplication::aboutQt();
    });
    bottomRow->addWidget(btnAboutQt);

    bottomRow->addStretch();

    auto *btnClose = new QPushButton("Close");
    btnClose->setObjectName("actionBtnPrimary");
    btnClose->setStyleSheet("background-color: #3b82f6; color: #ffffff; padding: 6px 18px; border-radius: 6px; font-weight: bold;");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    bottomRow->addWidget(btnClose);

    mainLayout->addLayout(bottomRow);
}
