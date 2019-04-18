#include "translationdialog.h"
#include "ui_translationdialog.h"

#include <QLineEdit>

TranslationDialog::TranslationDialog( QWidget *parent, DB_Manager *db_manager )
: QDialog{ parent }
, ui{ new Ui::TranslationDialog }
, foreignLangId{ 0 }
, nativeLangId{ 0 }
, db_manager{ db_manager }
{
    this->ui->setupUi( this );

    // remove help button from task bar
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & ( ~helpFlag );
    this->setWindowFlags( flags );
}

TranslationDialog::~TranslationDialog()
{
    delete this->ui;
}

void TranslationDialog::setForeignLangId( const int &foreignLangId )
{
    this->foreignLangId = foreignLangId;
}

void TranslationDialog::setNativeLangId( const int &nativeLangId )
{
   this->nativeLangId = nativeLangId;
}

void TranslationDialog::setUnknownWordLabelText( const QString &unknownWord )
{
    this->ui->label_word->setText( unknownWord );
}

void TranslationDialog::updateUnknownWordTitle( const QString &lang )
{
    this->ui->label_unknownWord_title->setText(
          QString( "Unknown word (%1):").arg( lang ) );
}

void TranslationDialog::updateTranslateToLangTitle( const QString &lang )
{
    this->ui->label_translateToLang_title->setText(
          QString( "Translate word (%1):").arg( lang ) );
}

void TranslationDialog::on_pushButton_add_clicked()
{
    const QString foreignWord{ this->ui->label_word->text() };
    const QString nativeWord{ this->ui->lineEdit_translateToLang->text() };

    // translate from foreign to native
    this->db_manager->translate( nativeWord, this->nativeLangId,
                                 foreignWord, this->foreignLangId );

    // translate from native to foreign
    this->db_manager->translate( foreignWord, this->foreignLangId,
                                 nativeWord, this->nativeLangId );

    this->close();
}
