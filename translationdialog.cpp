#include "translationdialog.h"
#include "ui_translationdialog.h"

#include <QLineEdit>
#include <QStringList>
#include <QString>
#include <QTableWidget>
#include <QVariant>
#include <QKeyEvent>
#include <QDebug>

TranslationDialog::TranslationDialog( QWidget *parent, DB_Manager *db_manager )
: QDialog{ parent }
, ui{ new Ui::TranslationDialog }
, foreignLangId{ 0 }
, nativeLangId{ 0 }
, db_manager{ db_manager }
{
    this->ui->setupUi( this );

    QHeaderView *headerView = this->ui->tableWidget_translations->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Stretch);

    this->ui->tableWidget_translations->verticalHeader()->hide();

    QFont font = this->ui->tableWidget_translations->horizontalHeader()->font();
    font.setBold(true);

    this->ui->tableWidget_translations->horizontalHeader()->setFont( font );

    // remove help button from task bar
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & ( ~helpFlag );
    this->setWindowFlags( flags );

    QObject::connect( this->ui->tableWidget_translations, &QTableWidget::itemChanged,
                      this, &TranslationDialog::onItemChanged,
                      Qt::UniqueConnection );
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

// DEBUG only - REMOVE later!
void TranslationDialog::fillTranslationTable( const QVector<std::pair<QString,int>> &list )
{
    if( list.empty() )
    {
     //   this->ui->tableWidget_translations->setVisible( false );
        return;
    }

    this->ui->tableWidget_translations->setVisible( true );

    this->ui->tableWidget_translations->setColumnCount( 1 );
    this->ui->tableWidget_translations->setRowCount( list.size() );
    this->ui->tableWidget_translations->setHorizontalHeaderLabels( QStringList("Word") );

    for( int i=0; i<list.size(); ++i )
    {
        QTableWidgetItem *item = new QTableWidgetItem(list[i].first);
        item->setData( Qt::UserRole, QVariant( list[i].second ) );

        this->ui->tableWidget_translations->setItem( i, 0, item );
    }
}

void TranslationDialog::keyPressEvent( QKeyEvent *e )
{
    QDialog::keyPressEvent(e);

    switch( e->type() )
    {
      case QEvent::KeyPress:
        if( e->key() == Qt::Key_Delete )
        {
            this->deleteItem( this->ui->tableWidget_translations->currentItem() );
        }

        break;
      case QEvent::KeyRelease:
        // fall through
      default:
        break;
    }
}

void TranslationDialog::deleteItem( QTableWidgetItem *item )
{
    if( item == nullptr )
        return;

    bool ok = false;
    const int wordID = item->data(Qt::UserRole).toInt( &ok );

    if( !item->text().isEmpty() )
    {
        this->rememberedWordInSelectedItemWidget = item->text();
    }

    if( ok )
    {
        this->toDeleteTranslations.insert( wordID, this->rememberedWordInSelectedItemWidget );
        this->ui->tableWidget_translations->removeRow( item->row() );
        this->ui->pushButton_ok->setEnabled( true );
    }
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

void TranslationDialog::on_pushButton_ok_clicked()
{
    const QString foreignWord{ this->ui->label_word->text() };

    // delete removed Words from DB
    for( int wordID : this->toDeleteTranslations.keys() )
    {
        this->db_manager->remove( wordID );
        emit translationDeleted( foreignWord, this->toDeleteTranslations.value( wordID ) );
    }

    const QString nativeWord{ this->ui->lineEdit_translateToLang->text().trimmed() };

    if( !nativeWord.isEmpty() )
    {
        // translate from foreign to native
        this->db_manager->translate( nativeWord, this->nativeLangId,
                                     foreignWord, this->foreignLangId );

        // translate from native to foreign
        this->db_manager->translate( foreignWord, this->foreignLangId,
                                     nativeWord, this->nativeLangId );

        emit translationAdded( foreignWord, nativeWord );
    }

    this->close();
}

void TranslationDialog::onItemChanged( QTableWidgetItem *item )
{
    const QString foreignWord{ this->ui->label_word->text() };
    const QString word = item->text().trimmed();

    if( word.isEmpty() )
    {
        this->deleteItem( item );
    }
    else
    {
        const int wordID = item->data( Qt::UserRole ).toInt();
        this->db_manager->update( wordID, word );
        emit translationDeleted( foreignWord, this->rememberedWordInSelectedItemWidget );
        emit translationAdded( foreignWord, item->text() );
    }
}

void TranslationDialog::on_lineEdit_translateToLang_textChanged( const QString &text )
{
    const QString _text{ text.trimmed() };

    if( _text.size() > 0 )
    {
        this->ui->pushButton_ok->setEnabled( true );
    }
    else
    {
        this->ui->pushButton_ok->setEnabled( false );
    }
}

void TranslationDialog::on_tableWidget_translations_itemDoubleClicked( QTableWidgetItem *item )
{
    this->rememberedWordInSelectedItemWidget = item->text();
}
