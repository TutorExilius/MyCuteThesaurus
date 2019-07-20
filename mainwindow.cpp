#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMap>
#include <QMessageBox>
#include <QFont>
#include <QFontMetrics>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <algorithm>

#include "word.h"
#include "mytextedit.h"
#include "customaboutdialog.h"
#include "log.h"
#include "translationdialog.h"
#include "settingdialog.h"

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, dbManager{ nullptr }
, analysed{ false }
, knownWords{ 0 }
, unknownWords{ 0 }
{
    this->ui->setupUi( this );

    this->dbManager = new DB_Manager{ this, "mycutethesaurus.db" };

    this->ui->statusBar->showMessage( "Current native language: " + this->dbManager->getCurrentNativeLang() );

    QObject::connect( this->ui->textEdit, &MyTextEdit::doubleClicked,
                      this, &MainWindow::onDoubleClicked,
                      Qt::UniqueConnection );

    this->fillComboBox();

    // Set Checkbox-Lang in Control-Panel to ForeignLang set in Settings ---
    const QString currentForeignLang{ this->dbManager->getCurrentForeignLang() };
    this->ui->comboBox_langs->setCurrentText( currentForeignLang.toUpper() );
    // ---
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

void MainWindow::fillComboBox()
{
    const QVector<QString> langs = this->dbManager->getLanguages();
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
        { AboutSection::VERSION, QString("%1").arg(GIT_VERSION) }, // "v0.1.0 Beta" },
        { AboutSection::DATE, "20.07.2019" },
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

void MainWindow::analyse()
{
    QVector<Word> tmp_foreign_words;
    const QString text{ this->ui->textEdit->toPlainText() };

    // build word
    QString word;
    QString link;

    //bool foreignLine = true;

    for( const QChar &ch : text )
    {
        //if( foreignLine )
        {
            if( !ch.isSpace() )
            {
                if( !link.isEmpty() )
                {
                    tmp_foreign_words.push_back( Word{ link, TYPE::LINK } );
                    link.clear();
                }

                word.append( ch );
            }
            else
            {
                if( !word.isEmpty() )
                {
                    tmp_foreign_words.push_back( Word{ word, TYPE::WORD } );
                    word.clear();
                }

                link.append( ch );
            }
        }

        if( ch == QChar{'\n'} )
        {
       //     foreignLine = !foreignLine;
        }
    }

    if( !word.isEmpty() )
    {
        tmp_foreign_words.push_back( Word{ word, TYPE::WORD } );
    }

    if( !link.isEmpty() )
    {
        tmp_foreign_words.push_back( Word{ link, TYPE::LINK } );
    }

    // reorganise std::vector<std::list> native_words / foreign_words;
    this->buildTranslationStructure( tmp_foreign_words );
}

void MainWindow::on_pushButton_analyse_clicked()
{
    this->ui->textEdit->setReadOnly( true );
    this->ui->comboBox_langs->setEnabled( false );
    this->ui->pushButton_analyse->setEnabled( false );
    this->ui->pushButton_edit->setEnabled( true );

    const QString selectedLang{ this->ui->comboBox_langs->currentText() };

    this->analyse();

    // clear current textEdit-content and reset cursors position to 0
    this->ui->textEdit->clear();

    const QString newContent = this->newText();

    // recalculate statistics
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

    this->analysed = true;
    this->ui->textEdit->setHtml( newContent );
    this->ui->label_statistics->setText( statistics );
    this->ui->comboBox_langs->setCurrentText( selectedLang );
}

void MainWindow::buildTranslationStructure( const QVector<Word> &foreign_words )
{
    this->foreign_words.clear();

    const QString foreignLangTag{ this->ui->comboBox_langs->currentText().toLower() };
    const QString nativeLangTag{ this->getNativeLang().toLower() };

    // get lang ids
    const int foreignLangId = this->dbManager->getLangId( foreignLangTag );
    const int nativeLangId = this->dbManager->getLangId( nativeLangTag );

    for( Word word : foreign_words )
    {
        if( word.isWordType() )
        {
            const QVector<QString> translations = this->dbManager->getTanslations(
                        word.getContent(), foreignLangId, nativeLangId );

            word.setTranslations( translations );
        }

        this->foreign_words.push_back( word );
    }
}

QString MainWindow::cascadeHtmlSpace( const int count ) const
{
    QString text;

    for( int i=0; i<count; ++i )
    {
        text.append( "&nbsp;" );
    }

    return text;
}

QString MainWindow::newText()
{
    // TODO erweiter words bzw translations an die wordbreite (nachfüllen mit leerzeichen)
    // this->updateWordWidth( this->foreign_words  );

    QString foreign_text;
    QString native_text;

    for( const Word &word : this->foreign_words )
    {
        const int wordLength = word.getContent().size();
        const QVector<QString> translations = word.getTranslations();

        if( word.isWordType() )
        {
            foreign_text.append( this->colorizeWord( word.getContent(), word.hasTranslations() ) );

            if( !translations.isEmpty() )
            {
                QString bestTranslation = translations.at( 0 );

                if( bestTranslation.size() < wordLength )
                {
                    bestTranslation.append( this->cascadeHtmlSpace( wordLength - bestTranslation.size() ) );
                }
                else
                {
                    foreign_text.append( this->cascadeHtmlSpace( bestTranslation.size() - wordLength ) );
                }

                native_text.append( this->htmlWord( bestTranslation ) );
            }
            else
            {
                native_text.append( this->htmlWord( this->cascadeHtmlSpace( wordLength ) ));
            }
        }
        else
        {
            if( word.getContent().contains( "\n" ) )
            {
                foreign_text.append( '\n' );
                native_text.append( '\n' );
            }
            else
            {
                foreign_text.append( this->maskHtml(' ') );
                native_text.append( this->maskHtml(' ') );
            }
        }
    }

    return this->mergeLanguages( foreign_text, native_text );
}

QString MainWindow::mergeLanguages( const QString &foreignText, const QString &nativeText ) const
{
    const QStringList foreignText_lines = foreignText.split( '\n' );
    const QStringList nativeText_lines = nativeText.split( '\n' );

    if( foreignText_lines.size() != nativeText_lines.size() )
        throw "Size mismatch";

    QString text;

    for( int i=0; i<foreignText_lines.size(); ++i )
    {
        text.append( foreignText_lines.at(i) );
        text.append( this->maskHtml( '\n' ) );
        text.append( nativeText_lines.at(i) );
        text.append( this->maskHtml( '\n' ) );
    }

    return text;
}

QString MainWindow::maskHtml( const QChar &ch ) const
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

QString MainWindow::colorizeWord( QString foreignWord, const bool isTranslated )
{
    if( foreignWord.isEmpty() )
    {
        return "";
    }

    if( isTranslated )
    {
        ++this->knownWords;
    }
    else
    {
        ++this->unknownWords;
    }

    return htmlWord( foreignWord, ((isTranslated) ? "#37e790;" : "red;") );
}

QString MainWindow::htmlWord( QString word, const QString &styleColor ) const
{
    return QString( "<span style=color:%1>%2</span>" ).
            arg( styleColor ).arg( word );
}

void MainWindow::onDoubleClicked()
{
    if( !this->analysed )
    {
        return;
    }

    const QString nativeWordBlackColored = this->ui->textEdit->textCursor().selection().toHtml();

    if( nativeWordBlackColored.contains("<!--StartFragment--><span style=\" color:#000000;\">") )
    {
        return;
    }

    const QString doubleClickedWord{ this->ui->textEdit->textCursor().selectedText().trimmed() };

    if( !doubleClickedWord.isEmpty() )
    {
        this->resetStatistic();

        const QString foreignLangTag = this->ui->comboBox_langs->currentText();
        const QString nativeLangTag = this->getNativeLang();

        TranslationDialog *dialog = new TranslationDialog{ this, this->dbManager };

        dialog->setUnknownWordLabelText( doubleClickedWord );
        dialog->updateUnknownWordTitle( this->ui->comboBox_langs->currentText() );
        dialog->updateTranslateToLangTitle( nativeLangTag );

        const int foreignLangID = this->dbManager->getLangId( foreignLangTag.toLower() );
        const int nativeLangID = this->dbManager->getLangId( nativeLangTag.toLower() );

        dialog->setForeignLangId( foreignLangID );
        dialog->setNativeLangId( nativeLangID );

        const QVector<QString> words = this->dbManager->getTanslations( doubleClickedWord, foreignLangID,  nativeLangID );

        QVector<std::pair<QString,int>> word_pairs;

        for( const QString &word : words )
        {
            const int wordID = this->dbManager->getWordId( word, nativeLangID );
            word_pairs.push_back( std::make_pair(word, wordID) );
        }

        dialog->fillTranslationTable( word_pairs );

        dialog->exec();

        this->restoreForeignText();
        this->on_pushButton_analyse_clicked();
    }
}

void MainWindow::resetStatistic()
{
    this->knownWords = 0;
    this->unknownWords = 0;
    this->analysed = false;
    this->ui->label_statistics->setText( "" );
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

void MainWindow::resetHighlighting()
{
    // qDebug() << "TEXTCHANGED";
    // this->ui->comboBox_langs->setCurrentText( "Select Language:" );

    const QString redStyleText{ "<span style=\" color:#ff0000" };
    const QString greenStyleText{ "<span style=\" color:#37e790" };
    const QString blackStyleText{ "<span style=\" color:#000000" };


    QString htmlText{ this->ui->textEdit->toHtml() };
    htmlText.replace( redStyleText, blackStyleText );
    htmlText.replace( greenStyleText, blackStyleText );

    if( this->ui->textEdit->toHtml() != htmlText )
    {
      QTextCursor cursor = this->ui->textEdit->textCursor();
      const int cursorsPos =  cursor.position();

      this->ui->textEdit->setHtml( htmlText );

      cursor.setPosition( cursorsPos );
      this->ui->textEdit->setTextCursor( cursor );
    }

    this->resetStatistic();
}

void MainWindow::restoreForeignText()
{
    QString foreignText;

    for( const Word &word : this->foreign_words )
    {
        foreignText.append( word.getContent() );
    }

    this->ui->textEdit->setText( foreignText );
}

void MainWindow::on_pushButton_edit_clicked()
{
    this->resetHighlighting();

    this->restoreForeignText();

    this->ui->textEdit->setReadOnly( false );
    this->ui->comboBox_langs->setEnabled( true );
    this->ui->pushButton_analyse->setEnabled( true );
    this->ui->pushButton_edit->setEnabled( false );
}

void MainWindow::on_action_Settings_triggered()
{
    SettingDialog *dialog = new SettingDialog{ this, this->dbManager };
    dialog->exec();
}

void MainWindow::on_textEdit_textChanged()
{
}
