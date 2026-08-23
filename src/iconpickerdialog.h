#pragma once
#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>

class IconPickerDialog : public QDialog {
    Q_OBJECT
public:
    explicit IconPickerDialog(const QString &currentIcon = QString(), QWidget *parent = nullptr);

    QString selectedIcon() const;

private slots:
    void onSearch(const QString &query);
    void onBrowseFile();
    void onItemSelected(QListWidgetItem *item);

private:
    void setupUi();
    void populateIcons(const QString &filter = QString());

    QListWidget *m_iconList;
    QLineEdit   *m_searchEdit;
    QLabel      *m_previewLabel;
    QString      m_selectedIcon;
};
