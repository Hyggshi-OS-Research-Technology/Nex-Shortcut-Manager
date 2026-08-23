#include "editdialog.h"
#include "iconpickerdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QIcon>

EditDialog::EditDialog(const DesktopEntry &entry, QWidget *parent)
    : QDialog(parent)
    , m_entry(entry)
{
    setupUi();
    populateFields();
    setWindowTitle("Edit Shortcut - " + entry.name());
    setMinimumWidth(550);
}

void EditDialog::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Icon row
    auto *iconRow = new QHBoxLayout;
    m_iconPreview = new QLabel;
    m_iconPreview->setFixedSize(64, 64);
    m_iconPreview->setAlignment(Qt::AlignCenter);
    m_iconPreview->setObjectName("iconPreview");

    m_iconEdit = new QLineEdit;
    m_iconEdit->setPlaceholderText("Icon name or path...");
    m_chooseIconBtn = new QPushButton("Browse Icon");
    m_chooseIconBtn->setObjectName("secondaryBtn");

    iconRow->addWidget(m_iconPreview);
    auto *iconRight = new QVBoxLayout;
    iconRight->addWidget(m_iconEdit);
    iconRight->addWidget(m_chooseIconBtn);
    iconRow->addLayout(iconRight);
    mainLayout->addLayout(iconRow);

    // Form fields
    auto *form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("Application display name");
    form->addRow("Name *:", m_nameEdit);

    m_execEdit = new QLineEdit;
    m_execEdit->setPlaceholderText("e.g. /usr/bin/firefox %u");
    form->addRow("Exec *:", m_execEdit);

    m_categoriesEdit = new QLineEdit;
    m_categoriesEdit->setPlaceholderText("e.g. Network;WebBrowser;");
    form->addRow("Categories:", m_categoriesEdit);

    m_commentEdit = new QLineEdit;
    m_commentEdit->setPlaceholderText("Short description");
    form->addRow("Comment:", m_commentEdit);

    m_pathEdit = new QLineEdit;
    m_pathEdit->setPlaceholderText("Working directory (optional)");
    form->addRow("Path:", m_pathEdit);

    m_terminalCheck = new QCheckBox("Run in terminal");
    m_noDisplayCheck = new QCheckBox("Hide from app menu (NoDisplay)");
    form->addRow("", m_terminalCheck);
    form->addRow("", m_noDisplayCheck);

    mainLayout->addLayout(form);

    // Validation label
    m_validationLabel = new QLabel;
    m_validationLabel->setObjectName("validationLabel");
    m_validationLabel->setWordWrap(true);
    m_validationLabel->hide();
    mainLayout->addWidget(m_validationLabel);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    auto *validateBtn = new QPushButton("Validate");
    validateBtn->setObjectName("secondaryBtn");
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnLayout->addWidget(validateBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(buttons);
    mainLayout->addLayout(btnLayout);

    connect(m_chooseIconBtn, &QPushButton::clicked, this, &EditDialog::onChooseIcon);
    connect(validateBtn, &QPushButton::clicked, this, &EditDialog::onValidate);
    connect(buttons, &QDialogButtonBox::accepted, this, &EditDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_iconEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        DesktopEntry temp = result();
        m_iconPreview->setPixmap(temp.resolveIcon().pixmap(60, 60));
    });
}

void EditDialog::populateFields() {
    m_nameEdit->setText(m_entry.name());
    m_execEdit->setText(m_entry.exec());
    m_iconEdit->setText(m_entry.icon());
    m_categoriesEdit->setText(m_entry.categories());
    m_commentEdit->setText(m_entry.comment());
    m_pathEdit->setText(m_entry.path());
    m_terminalCheck->setChecked(m_entry.terminal());
    m_noDisplayCheck->setChecked(m_entry.noDisplay());
    m_iconPreview->setPixmap(m_entry.resolveIcon().pixmap(60, 60));
}

void EditDialog::onChooseIcon() {
    IconPickerDialog dlg(m_iconEdit->text(), this);
    if (dlg.exec() == QDialog::Accepted) {
        m_iconEdit->setText(dlg.selectedIcon());
    }
}

void EditDialog::onValidate() {
    DesktopEntry temp = result();
    QStringList errors = temp.validate();
    if (errors.isEmpty()) {
        m_validationLabel->setText("✓ Valid .desktop entry");
        m_validationLabel->setStyleSheet("color: #4ade80;");
    } else {
        m_validationLabel->setText("✗ Errors:\n• " + errors.join("\n• "));
        m_validationLabel->setStyleSheet("color: #f87171;");
    }
    m_validationLabel->show();
}

void EditDialog::onAccept() {
    DesktopEntry temp = result();
    QStringList errors = temp.validate();
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Please fix the following:\n• " + errors.join("\n• "));
        return;
    }
    accept();
}

DesktopEntry EditDialog::result() const {
    DesktopEntry e = m_entry;
    e.setName(m_nameEdit->text().trimmed());
    e.setExec(m_execEdit->text().trimmed());
    e.setIcon(m_iconEdit->text().trimmed());
    e.setCategories(m_categoriesEdit->text().trimmed());
    e.setComment(m_commentEdit->text().trimmed());
    e.setPath(m_pathEdit->text().trimmed());
    e.setTerminal(m_terminalCheck->isChecked());
    e.setNoDisplay(m_noDisplayCheck->isChecked());
    return e;
}
