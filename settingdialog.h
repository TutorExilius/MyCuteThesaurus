#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

#include "db_manager.h"

namespace Ui {
    class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent, DB_Manager *db_manager );

    ~SettingDialog();

private slots:
    void on_pushButton_save_clicked();

private:
    Ui::SettingDialog *ui;

    DB_Manager *db_manager;
};

#endif // SETTINGDIALOG_H
