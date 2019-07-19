#include "settingdialog.h"
#include "ui_settingdialog.h"

#include <QDebug>

SettingDialog::SettingDialog( QWidget *parent, DB_Manager *db_manager )
: QDialog{ parent }
, ui{ new Ui::SettingDialog }
, db_manager{ db_manager }
{
    this->ui->setupUi( this );

    if( this->db_manager == nullptr )
    {
        throw "db_manager is null";
    }

    const QVector<QString> supportedLanguages = this->db_manager->getLanguages();
    const QString currentNativeLang = this->db_manager->getCurrentNativeLang().toUpper();
    const QString currentForeignLang = this->db_manager->getCurrentForeignLang().toUpper();

    for( const QString &lang : supportedLanguages )
    {
        this->ui->comboBox_nativeLanguages->addItem( lang.toUpper() );
        this->ui->comboBox_foreignLanguages->addItem( lang.toUpper() );
    }

    this->ui->comboBox_nativeLanguages->setCurrentText( currentNativeLang );
    this->ui->comboBox_foreignLanguages->setCurrentText( currentForeignLang );
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::on_pushButton_save_clicked()
{
    const QString currentNativeLang = this->ui->comboBox_nativeLanguages->currentText();
    const QString currentForeignLang = this->ui->comboBox_foreignLanguages->currentText();

    this->db_manager->updateCurrentNativeLang( currentNativeLang );
    this->db_manager->updateCurrentForeignLang( currentForeignLang );

    qDebug() << "saved";

    this->close();
}
