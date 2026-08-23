#pragma once
#include "desktopentry.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class EditDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditDialog(const DesktopEntry &entry, QWidget *parent = nullptr);

    DesktopEntry result() const;

private slots:
    void onChooseIcon();
    void onValidate();
    void onAccept();

private:
    void setupUi();
    void populateFields();

    DesktopEntry m_entry;

    QLineEdit *m_nameEdit;
    QLineEdit *m_execEdit;
    QLineEdit *m_iconEdit;
    QLineEdit *m_categoriesEdit;
    QLineEdit *m_commentEdit;
    QLineEdit *m_pathEdit;
    QCheckBox *m_terminalCheck;
    QCheckBox *m_noDisplayCheck;
    QLabel    *m_iconPreview;
    QLabel    *m_validationLabel;
    QPushButton *m_chooseIconBtn;
};
