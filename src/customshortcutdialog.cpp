#include "customshortcutdialog.h"
#include "iconpickerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>

CustomShortcutDialog::CustomShortcutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle("Create Custom Shortcut");
    setMinimumWidth(520);
}

void CustomShortcutDialog::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Type selection
    auto *typeGroup = new QGroupBox("Shortcut Type");
    auto *typeLayout = new QHBoxLayout(typeGroup);
    m_appRadio    = new QRadioButton("🚀 Application");
    m_fileRadio   = new QRadioButton("📄 File");
    m_folderRadio = new QRadioButton("📁 Folder");
    m_appRadio->setChecked(true);
    typeLayout->addWidget(m_appRadio);
    typeLayout->addWidget(m_fileRadio);
    typeLayout->addWidget(m_folderRadio);
    mainLayout->addWidget(typeGroup);

    // Icon row
    auto *iconRow = new QHBoxLayout;
    m_iconPreview = new QLabel;
    m_iconPreview->setFixedSize(64, 64);
    m_iconPreview->setAlignment(Qt::AlignCenter);
    m_iconPreview->setObjectName("iconPreview");
    m_iconPreview->setPixmap(QIcon::fromTheme("application-x-executable").pixmap(56, 56));

    m_iconEdit = new QLineEdit;
    m_iconEdit->setPlaceholderText("Icon name or full path...");
    m_chooseIconBtn = new QPushButton("Browse Icon");
    m_chooseIconBtn->setObjectName("secondaryBtn");
    iconRow->addWidget(m_iconPreview);
    auto *iconRight = new QVBoxLayout;
    iconRight->addWidget(m_iconEdit);
    iconRight->addWidget(m_chooseIconBtn);
    iconRow->addLayout(iconRight);
    mainLayout->addLayout(iconRow);

    // Form
    auto *form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("My Application");
    form->addRow("Name *:", m_nameEdit);

    auto *execRow = new QHBoxLayout;
    m_execEdit = new QLineEdit;
    m_execEdit->setPlaceholderText("/usr/bin/myapp or xdg-open /path/to/file");
    m_chooseExecBtn = new QPushButton("...");
    m_chooseExecBtn->setFixedWidth(36);
    m_chooseExecBtn->setObjectName("secondaryBtn");
    execRow->addWidget(m_execEdit);
    execRow->addWidget(m_chooseExecBtn);
    form->addRow("Command *:", execRow);

    m_commentEdit = new QLineEdit;
    m_commentEdit->setPlaceholderText("Short description");
    form->addRow("Description:", m_commentEdit);

    m_workingDirEdit = new QLineEdit;
    m_workingDirEdit->setPlaceholderText("Working directory (optional)");
    form->addRow("Working Dir:", m_workingDirEdit);

    m_terminalCheck = new QCheckBox("Run in terminal");
    form->addRow("", m_terminalCheck);

    mainLayout->addLayout(form);

    m_validationLabel = new QLabel;
    m_validationLabel->setObjectName("validationLabel");
    m_validationLabel->setWordWrap(true);
    m_validationLabel->hide();
    mainLayout->addWidget(m_validationLabel);

    // Buttons
    auto *btnRow = new QHBoxLayout;
    auto *validateBtn = new QPushButton("Validate");
    validateBtn->setObjectName("secondaryBtn");
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnRow->addWidget(validateBtn);
    btnRow->addStretch();
    btnRow->addWidget(buttons);
    mainLayout->addLayout(btnRow);

    connect(m_chooseIconBtn, &QPushButton::clicked, this, &CustomShortcutDialog::onChooseIcon);
    connect(m_chooseExecBtn, &QPushButton::clicked, this, &CustomShortcutDialog::onChooseExec);
    connect(validateBtn, &QPushButton::clicked, this, &CustomShortcutDialog::onValidate);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        onValidate();
        DesktopEntry e = result();
        if (e.isValid()) accept();
        else QMessageBox::warning(this, "Validation Error",
                                  "Please fix the errors shown before creating the shortcut.");
    });

    connect(m_appRadio,    &QRadioButton::toggled, this, &CustomShortcutDialog::onTypeChanged);
    connect(m_fileRadio,   &QRadioButton::toggled, this, &CustomShortcutDialog::onTypeChanged);
    connect(m_folderRadio, &QRadioButton::toggled, this, &CustomShortcutDialog::onTypeChanged);

    connect(m_iconEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QIcon icon = text.isEmpty()
            ? QIcon::fromTheme("application-x-executable")
            : DesktopEntry::createCustomShortcut("", "", text).resolveIcon();
        m_iconPreview->setPixmap(icon.pixmap(56, 56));
    });
}

void CustomShortcutDialog::onTypeChanged() {
    if (m_fileRadio->isChecked()) {
        m_iconEdit->setText("text-x-generic");
        m_terminalCheck->setEnabled(false);
    } else if (m_folderRadio->isChecked()) {
        m_iconEdit->setText("folder");
        m_terminalCheck->setEnabled(false);
    } else {
        m_terminalCheck->setEnabled(true);
    }
}

void CustomShortcutDialog::onChooseIcon() {
    IconPickerDialog dlg(m_iconEdit->text(), this);
    if (dlg.exec() == QDialog::Accepted)
        m_iconEdit->setText(dlg.selectedIcon());
}

void CustomShortcutDialog::onChooseExec() {
    if (m_folderRadio->isChecked()) {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Folder");
        if (!dir.isEmpty()) {
            m_execEdit->setText("xdg-open \"" + dir + "\"");
            if (m_nameEdit->text().isEmpty())
                m_nameEdit->setText(QFileInfo(dir).fileName());
        }
    } else {
        QString file = QFileDialog::getOpenFileName(this, "Select Executable or File");
        if (!file.isEmpty()) {
            if (m_fileRadio->isChecked())
                m_execEdit->setText("xdg-open \"" + file + "\"");
            else
                m_execEdit->setText(file);
            if (m_nameEdit->text().isEmpty())
                m_nameEdit->setText(QFileInfo(file).completeBaseName());
        }
    }
}

void CustomShortcutDialog::onChoosePath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Working Directory",
                                                     QDir::homePath());
    if (!dir.isEmpty())
        m_workingDirEdit->setText(dir);
}

void CustomShortcutDialog::onValidate() {
    DesktopEntry e = result();
    QStringList errors = e.validate();
    if (errors.isEmpty()) {
        m_validationLabel->setText("✓ Valid shortcut configuration");
        m_validationLabel->setStyleSheet("color: #4ade80;");
    } else {
        m_validationLabel->setText("✗ " + errors.join(" | "));
        m_validationLabel->setStyleSheet("color: #f87171;");
    }
    m_validationLabel->show();
}

DesktopEntry CustomShortcutDialog::result() const {
    return DesktopEntry::createCustomShortcut(
        m_nameEdit->text().trimmed(),
        m_execEdit->text().trimmed(),
        m_iconEdit->text().trimmed(),
        m_commentEdit->text().trimmed(),
        m_terminalCheck->isChecked()
    );
}
