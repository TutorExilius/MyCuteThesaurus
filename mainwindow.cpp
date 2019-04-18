#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMap>
#include <QMessageBox>

#include "mytextedit.h"
#include "customaboutdialog.h"
#include "log.h"
#include "translationdialog.h"

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, dbManager{ nullptr }
, dbConnectionCheckTimer{ nullptr }
, analysed{ false }
, knownWords{ 0 }
, unknownWords{ 0 }
{
    this->ui->setupUi( this );

    this->dbManager = new DB_Manager{ this, "mycutethesaurus.db" };

    this->ui->statusBar->showMessage( "Current native language: " + this->dbManager->getCurrentNativeLang() );

    this->dbConnectionCheckTimer = new QTimer{ this };

    QObject::connect( this->dbConnectionCheckTimer, &QTimer::timeout,
                      this, &MainWindow::onDbConnectionTimeOut,
                      Qt::UniqueConnection );

    QObject::connect( this->ui->textEdit, &MyTextEdit::doubleClicked,
                      this, &MainWindow::onDoubleClicked,
                      Qt::UniqueConnection );

    dbConnectionCheckTimer->start( 1000 );
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getSelectedText() const
{
    return  this->ui->textEdit->textCursor().selectedText();
}

QString MainWindow::getNativeLang() const
{
    return this->dbManager->getCurrentNativeLang();
}

void MainWindow::onDbConnectionTimeOut()
{
    if( !this->dbManager->isOk() )
    {
        ::logInfo( "Close application causing of db connection failure." );
        this->close();
    }
    else
    {
        this->dbConnectionCheckTimer->stop();

        QObject::disconnect( this->dbConnectionCheckTimer, &QTimer::timeout,
                             this, &MainWindow::onDbConnectionTimeOut );

        this->fillComboBox();
    }
}

void MainWindow::fillComboBox()
{
    const QVector<QString> langs = this->dbManager->getLangs();
    this->ui->comboBox_langs->addItem( "Select Language:" );

    for( const QString &lang : langs )
    {
        this->ui->comboBox_langs->addItem( lang.toUpper() );
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt( this );
}

void MainWindow::on_action_Exit_triggered()
{
    this->close();
}

void MainWindow::on_actionAbout_My_Cute_Thesaurus_triggered()
{
    QMap<AboutSection,QString> contents{
        { AboutSection::TITLE, "About My Cute Thesaurus" },
        { AboutSection::VERSION, "v0.0.1 Beta" },
        { AboutSection::DATE, "09.03.2019" },
        { AboutSection::SHORT_INFO, "A Let's Try [Qt] project created on <a href=http://twitch.tv/tutorexilius>"
        "Twitch.tv/TutorExilius</a>." },
        { AboutSection::DESCRIPTION, "Description here..."},
        { AboutSection::RESOURCES, "Yon can download source code on <a href=https://github.com/tutorexilius/MyCuteThesaurus>GitHub:MyCuteThesaurus</a>" },
        { AboutSection::LOGO, ":/tutor_exilius_logo_64x64.png" },
        { AboutSection::AUTHOR, "Tutor Exilius" },
        { AboutSection::LICENCE, "LGPL v3" }
    };

    CustomAboutDialog::about( this, contents );
}

void MainWindow::on_pushButton_analyse_clicked()
{
    const QString selectedLang{ this->ui->comboBox_langs->currentText() };

    const QString text{ this->ui->textEdit->toPlainText() };
    QString newText;

    // build word
    QString word;

    for( const QChar &ch : text )
    {
        if( !ch.isSpace() && !MyTextEdit::isWordDelim( ch ) )
        {
            word.append( ch );
        }
        else
        {
            if( !word.isEmpty() )
            {
                newText.append( this->colorizeWord( word ) );
                word.clear();
            }

            newText.append( this->maskHtml( ch ) );
        }
    }

    newText.append( this->colorizeWord( word ) );

    // clear current textEdit-content and reset cursors position to 0
    this->ui->textEdit->clear();

    // refill textEdit with html text by using textCursor().insertHtml(...),
    // so the cursor can track the end of text
    this->ui->textEdit->textCursor().insertHtml( newText );

    this->ui->comboBox_langs->setCurrentText( selectedLang );

    int sum = this->knownWords + this->unknownWords;

    double knownPerc = ( sum > 0 ) ? this->knownWords * 100.0 / sum : 0.0;
    double unknownPerc = ( sum > 0 ) ? this->unknownWords * 100.0 / sum : 0.0;


    const QString statistics{
        QString{ "%1 Words: Known %2 <span style=\"color: green;\">(%3%)</span> "
                 "Unknown %4 <span style=\"color: red;\">(%5%)</span>" }
                 .arg( sum )
                 .arg( this->knownWords )
                 .arg( knownPerc, 0, 'f', 2 )
                 .arg( this->unknownWords )
                 .arg( unknownPerc, 0, 'f', 2 )
    };


    this->ui->label_statistics->setText( statistics );

    this->analysed = true;
}

QString MainWindow::maskHtml( const QChar &ch )
{
    if( ch == '\t' )
    {
        return "&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    else if( ch == '\n' )
    {
        return "<br>";
    }
    else if( ch.isSpace() )
    {
        return "&nbsp;";
    }
    else
    {
        return QString{ ch };
    }
}

QString MainWindow::colorizeWord( QString foreignWord )
{
    if( foreignWord.isEmpty() )
    {
        return "";
    }

    const QString foreignLangTag{ this->ui->comboBox_langs->currentText().toLower() };
    const QString nativeLangTag{ this->getNativeLang().toLower() };

    // get lang ids
    int foreignLangId = this->dbManager->getLangId( foreignLangTag );
    int nativeLangId = this->dbManager->getLangId( nativeLangTag );

    // check if word is known -> Database
    bool isKnownWord = this->dbManager->isTranslatedWord( foreignWord, foreignLangId );

    if( isKnownWord )
    {
        ++this->knownWords;
    }
    else
    {
        ++this->unknownWords;
    }

    // escape brackets <..>
    foreignWord = foreignWord.toHtmlEscaped();

    return QString( "<span style=color:%1>%2</span>" ).
            arg( ((isKnownWord) ? "#37e790;" : "red;") ).
            arg( foreignWord );
}

void MainWindow::onDoubleClicked()
{
    if( !analysed )
        return;

    const QString doubleClickedWord{ this->ui->textEdit->textCursor().selectedText() };

    if( !doubleClickedWord.isEmpty() )
    {
        this->resetStatistic();

        TranslationDialog *dialog = new TranslationDialog{ this, this->dbManager };

        dialog->setUnknownWordLabelText( doubleClickedWord );
        dialog->updateUnknownWordTitle( this->ui->comboBox_langs->currentText() );
        dialog->updateTranslateToLangTitle( this->getNativeLang() );

        dialog->setForeignLangId( this->dbManager->getLangId( this->ui->comboBox_langs->currentText().toLower() ) );
        dialog->setNativeLangId( this->dbManager->getLangId( this->getNativeLang().toLower() ) );

        dialog->exec();

        this->on_pushButton_analyse_clicked();
    }
}

void MainWindow::resetStatistic()
{
    this->knownWords = 0;
    this->unknownWords = 0;
    this->analysed = false;
}

void MainWindow::on_comboBox_langs_currentTextChanged( const QString &section )
{
    if( section == "Select Language:" )
    {
        this->ui->pushButton_analyse->setEnabled( false );
    }
    else
    {
        this->ui->pushButton_analyse->setEnabled( true );
    }
}

void MainWindow::on_textEdit_textChanged()
{
    this->check();
}

void MainWindow::check()
{
    // qDebug() << "TEXTCHANGED";
     this->ui->comboBox_langs->setCurrentText( "Select Language:" );

     const QString redStyleText{ "<span style=\" color:#ff0000" };
     const QString greenStyleText{ "<span style=\" color:#37e790" };
     const QString blackStyleText{ "<span style=\" color:#000000" };

     if( this->analysed )
     {
         QString htmlText{ this->ui->textEdit->toHtml() };
         htmlText.replace( redStyleText, blackStyleText );
         htmlText.replace( greenStyleText, blackStyleText );

         qDebug() << htmlText;

         if( this->ui->textEdit->toHtml() != htmlText )
         {
             QTextCursor cursor = this->ui->textEdit->textCursor();
             const int cursorsPos =  cursor.position();

             this->ui->textEdit->setHtml( htmlText );

             cursor.setPosition( cursorsPos );
             this->ui->textEdit->setTextCursor( cursor );
         }

         this->analysed = false;
         this->knownWords = 0;
         this->unknownWords = 0;
         this->ui->label_statistics->setText( "" );
     }
}

