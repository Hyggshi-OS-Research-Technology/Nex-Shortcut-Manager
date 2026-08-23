#pragma once
#include "desktopentry.h"
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>

class CustomShortcutDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomShortcutDialog(QWidget *parent = nullptr);

    DesktopEntry result() const;

private slots:
    void onChooseIcon();
    void onChooseExec();
    void onChoosePath();
    void onValidate();
    void onTypeChanged();

private:
    void setupUi();

    QRadioButton *m_appRadio;
    QRadioButton *m_fileRadio;
    QRadioButton *m_folderRadio;

    QLineEdit *m_nameEdit;
    QLineEdit *m_execEdit;
    QLineEdit *m_iconEdit;
    QLineEdit *m_commentEdit;
    QLineEdit *m_workingDirEdit;
    QCheckBox *m_terminalCheck;
    QLabel    *m_iconPreview;
    QLabel    *m_validationLabel;

    QPushButton *m_chooseExecBtn;
    QPushButton *m_chooseIconBtn;
};
