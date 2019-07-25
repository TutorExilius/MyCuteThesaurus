#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QFileDialog>
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

QString MainWindow::normalizeVersion( const QString &version )
{
    return version.mid( 0, version.lastIndexOf( '-' ) );
}

QString MainWindow::revision( const QString &version )
{
    return version.mid( version.lastIndexOf( '-' ) + 1 );
}

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, dbManager{ nullptr }
, analysed{ false }
, knownWords{ 0 }
, unknownWords{ 0 }
, fileChangeWatcher{ new QFileSystemWatcher( QApplication::instance() ) }
, openFileChangedFromExtern{ false }
, mode{ Mode::EDIT_MODE }
, textColors{ { TextTypeColor::FOREIGN_TEXT_KNOWN_COLOR, "#32ab32" },
              { TextTypeColor::FOREIGN_TEXT_UNKNOWN_COLOR, "#ff0000" },
              { TextTypeColor::NATIVE_UNMARKED_TEXT_COLOR, "#010101" },
              { TextTypeColor::NATIVE_MARKED_TEXT_COLOR, "#A0A0A0" },
              { TextTypeColor::STATISTIC_KNOWN_WORDS_COLOR, "#32ab32" },
              { TextTypeColor::STATISTIC_UNKNOWN_WORDS_COLOR, "#ff0000" },
              { TextTypeColor::HORIZONTAL_LINE_COLOR, "#bcbcbc" },
              { TextTypeColor::SEPERATOR_COLOR, "#999999" } }
{
    this->ui->setupUi( this );

    this->dbManager = new DB_Manager{ this, "mycutethesaurus.db" };

    QObject::connect( this->ui->textEdit, &MyTextEdit::doubleClicked,
                      this, &MainWindow::onDoubleClicked,
                      Qt::UniqueConnection );

    this->fillComboBox();

    // Set Checkbox-Lang in Control-Panel to ForeignLang set in Settings ---
    const QString currentForeignLang{ this->dbManager->getCurrentForeignLang() };
    this->ui->comboBox_langs->setCurrentText( currentForeignLang.toUpper() );
    // ---

    QObject::connect( this->fileChangeWatcher, &QFileSystemWatcher::fileChanged,
                      this, &MainWindow::onOpenFileChanged,
                      Qt::UniqueConnection );

    QObject::connect( this->ui->textEdit, &MyTextEdit::escapeTriggered,
                      this, &MainWindow::onEscape,
                      Qt::UniqueConnection );

    this->ui->statusBar->showMessage( "Current native language: " + this->dbManager->getCurrentNativeLang() );
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

void MainWindow::onOpenFileChanged()
{
    this->openFileChangedFromExtern = true;
}

void MainWindow::onLangChanged()
{
    this->chachedTranslations.clear();
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
    QString version = QString("%1").arg(GIT_VERSION);

    QMap<AboutSection,QString> contents{
        { AboutSection::TITLE, "About My Cute Thesaurus" },
        { AboutSection::VERSION, MainWindow::normalizeVersion(version) }, // "v0.1-beta-1" },
        { AboutSection::REVISION, MainWindow::revision(version) },
        { AboutSection::DATE, "24.07.2019" },
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

    for( const QChar &ch : text )
    {
        if( ch.isLetter() || this->ui->textEdit->isPartOfWordSeperators( ch ) )
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

    if( !word.isEmpty() )
    {
        tmp_foreign_words.push_back( Word{ word, TYPE::WORD } );
    }

    if( !link.isEmpty() )
    {
        tmp_foreign_words.push_back( Word{ link, TYPE::LINK } );
    }

    this->originForeignText = text;
    this->buildTranslationStructure( tmp_foreign_words );
}

void MainWindow::switchToEditMode()
{
    this->ui->pushButton_edit->setEnabled( false );
    this->ui->pushButton_analyse->setEnabled( true );

    this->ui->action_Save->setEnabled( true );
    this->ui->actionSave_As->setEnabled( true );
    this->mode = Mode::EDIT_MODE;
}

void MainWindow::switchToTranslationMode()
{
    this->ui->pushButton_edit->setEnabled( true );
    this->ui->pushButton_analyse->setEnabled( false );

    this->ui->action_Save->setEnabled( false );
    this->ui->actionSave_As->setEnabled( false );
    this->mode = Mode::TRANSLATE_MODE;
}

void MainWindow::switchMode()
{
    if( this->mode == Mode::TRANSLATE_MODE ) // Switch to Edit Mode
    {
        this->switchToEditMode();
    }
    else // Switch to Translation Mode
    {
        this->switchToTranslationMode();
    }
}

void MainWindow::on_pushButton_analyse_clicked()
{
    this->ui->textEdit->setReadOnly( true );
    this->ui->comboBox_langs->setEnabled( false );
    //this->ui->pushButton_analyse->setEnabled( false );
    //this->ui->pushButton_edit->setEnabled( true );

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
        QString{ "%1 Words: Known %2 <span style=\"color: %3;\">(%4%)</span> "
                 "Unknown %5 <span style=\"color: %6;\">(%7%)</span>" }
                 .arg( sum )
                 .arg( this->knownWords )
                 .arg( this->textColors[TextTypeColor::FOREIGN_TEXT_KNOWN_COLOR] )
                 .arg( knownPerc, 0, 'f', 2 )
                 .arg( this->unknownWords )
                 .arg( this->textColors[TextTypeColor::FOREIGN_TEXT_UNKNOWN_COLOR] )
                 .arg( unknownPerc, 0, 'f', 2 )
    };

    this->analysed = true;
    this->ui->textEdit->setHtml( newContent );
    this->ui->label_statistics->setText( statistics );
    this->ui->comboBox_langs->setCurrentText( selectedLang );

    if( this->mode == Mode::EDIT_MODE )
    {
        this->switchMode();
    }
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
            const QVector<QString> translations =
                    this->getTanslations( word.getContent(),
                                          foreignLangId,
                                          nativeLangId );

            word.setTranslations( translations );
        }
        else
        {
            QString content{ word.getContent() };

            if( !content.isEmpty() )
            {
                content.prepend( ' ' );
                content.append( ' ' );
            }

            word.setContent( content );
        }

        this->foreign_words.push_back( word );

        if( word.isWordType() )
        {
            this->cacheWord( word );
        }
    }
}

void MainWindow::cacheWord( const Word &word )
{
    if( !this->chachedTranslations.contains( word.getContent() ) )
    {
        this->chachedTranslations.insert( word.getContent(), word );
    }
}

void MainWindow::updateCachedWord( const QString &foreignWord, const QString &translation )
{
    auto word = this->chachedTranslations.find( foreignWord );

    if( word !=  this->chachedTranslations.end() )
    {
        Word updatedWord{ word.value() };
        updatedWord.addTranslation( translation );

        this->chachedTranslations.insert( foreignWord, updatedWord );
    }
}

QVector<QString> MainWindow::getTanslations( const QString &word,
                                             const int foreignLangID,
                                             const int nativeLangId,
                                             bool useCache ) const
{
    if( useCache )
    {
        if( this->chachedTranslations.contains( word ) )
        {
            return this->chachedTranslations.value( word ).getTranslations();
        }
    }

    return this->dbManager->getTanslations( word, foreignLangID, nativeLangId );
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
    QString foreign_text;
    QString native_text;

    QFont font{ this->ui->textEdit->font() };
    QFontMetrics fm{ font };

    int textEditViewWidth = 0;

    QString cleanForeignTextLine;
    QString cleanNativeTextLine;

    for( const Word &word : this->foreign_words )
    {
        const int wordLength = word.getContent().size();
        const QVector<QString> translations = word.getTranslations();

        if( word.isWordType() )
        {   
            cleanForeignTextLine.append( word.getContent() );
            foreign_text.append( this->colorizeWord( word.getContent(), word.hasTranslations() ) );

            if( !translations.isEmpty() )
            {
                QString bestTranslation = translations.at( 0 );
                cleanNativeTextLine.append( bestTranslation );

                if( bestTranslation.size() < wordLength )
                {
                    cleanNativeTextLine.append( QString{ wordLength - bestTranslation.size(), ' ' } );

                    bestTranslation.append( this->cascadeHtmlSpace( wordLength - bestTranslation.size() ) );
                }
                else
                {                   
                    cleanForeignTextLine.append( QString{ bestTranslation.size() - wordLength, ' ' } );
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
            const QString htmlMaskedContent = this->maskHtml( word.getContent() );

            if( word.getContent().contains( "\n" ) )
            {
                foreign_text.append( word.getContent() );
                native_text.append( word.getContent() );

                textEditViewWidth = std::max( textEditViewWidth, fm.width( cleanForeignTextLine ) );
                textEditViewWidth = std::max( textEditViewWidth, fm.width( cleanNativeTextLine ) );

                cleanNativeTextLine.clear();
                cleanForeignTextLine.clear();
            }
            else
            {
                cleanForeignTextLine.append( word.getContent() );
                cleanNativeTextLine.append( word.getContent() );

                foreign_text.append( htmlMaskedContent );
                native_text.append( htmlMaskedContent );
            }
        }
    }

    return this->mergeLanguages( foreign_text, native_text, textEditViewWidth );
}

QString MainWindow::mergeLanguages( const QString &foreignText,
                                    const QString &nativeText,
                                    const int textEditViewWidth ) const
{
    const QStringList foreignText_lines = foreignText.split( '\n' );
    const QStringList nativeText_lines = nativeText.split( '\n' );

    if( foreignText_lines.size() != nativeText_lines.size() )
    {
        throw "Size mismatch";
    }


    QFont font{ this->ui->textEdit->font() };
    QFontMetrics fm{ font };
    const QChar unicodeLine{ 0x23AF }; // 0x23AF = '⎯'

    const int maxCharWidth{ fm.width( unicodeLine ) };

    int countUnicodeLines{ 0 };

    // text available, no empty lines, countUnicodeLines can be set
    if( textEditViewWidth > 0 )
    {
       countUnicodeLines = static_cast<int>( textEditViewWidth / maxCharWidth ) + 1;
    }

    const QString extraLine{ countUnicodeLines, unicodeLine };

    QString text;

    for( int i=0; i<foreignText_lines.size(); ++i )
    {
        text.append( foreignText_lines.at(i) );
        text.append( this->maskHtml( '\n' ) );

        // make translation bold ---
        QString nativeText{ nativeText_lines.at(i) };
        nativeText.prepend( "<span style=\"font-weight: bold;\">" );
        nativeText.append( "</span>" );
        nativeText.replace( "<span style=color:black>",
                            QString{"<span style=color:%1>"}
                            .arg( this->textColors[TextTypeColor::NATIVE_UNMARKED_TEXT_COLOR] ) );

        text.append( nativeText );
        //text.append( nativeText_lines.at(i) );

        if( i != foreignText_lines.size()-1 )
        {
            text.append( QString{"<br><span style=\"color:%1\">%2</span><br>"}
                         .arg( this->textColors[TextTypeColor::HORIZONTAL_LINE_COLOR] )
                         .arg( extraLine ) );
        }
    }

    return text;
}

QString MainWindow::maskHtml( const QString &content ) const
{
    QString htmlMaskedString;

    for( const QChar &ch : content )
    {
        htmlMaskedString.append( this->maskHtml( ch ) );
    }

    return htmlMaskedString;
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

    return htmlWord( foreignWord, ((isTranslated)
                                   ? this->textColors[TextTypeColor::FOREIGN_TEXT_KNOWN_COLOR]
                                   : this->textColors[TextTypeColor::FOREIGN_TEXT_UNKNOWN_COLOR] ) );
}

QString MainWindow::htmlWord( QString word, const QString &styleColor ) const
{
    return QString( "<span style=color:%1>%2</span>" ).
            arg( styleColor ).arg( word );
}

// -> is meant as "seperator" trim() .. remove not accepted seperators from start and end
QString MainWindow::removeSeperators( const QString &word ) const
{
    if( word.isEmpty() )
    {
        return "";
    }

    QString newWord{ word };

    for( int i = 0; i<word.size(); ++i )
    {
        QChar ch{ word.at(i) };

        if( !ch.isLetter() && !this->ui->textEdit->isPartOfWordSeperators( ch ) )
        {
            newWord.remove( 0, 1 );
        }
        else
        {
            break;
        }
    }

    for( int i = word.size() - 1; i > 0; --i )
    {
        QChar ch{ word.at(i) };

        if( !ch.isLetter() && !this->ui->textEdit->isPartOfWordSeperators( ch ) )
        {
            newWord.remove( newWord.size() - 1, 1 );
        }
        else
        {
            break;
        }
    }

    return newWord;
}

void MainWindow::onDoubleClicked()
{
    if( !this->analysed )
    {
        return;
    }

    const QString doubleClickedWord_html = this->ui->textEdit->textCursor().selection().toHtml();

    if( doubleClickedWord_html.contains(
          QString{"<!--StartFragment--><span style=\" font-weight:600; color:%1;\">"}
            .arg( this->textColors[TextTypeColor::NATIVE_UNMARKED_TEXT_COLOR] ) )
        // TODO: check expected string-structure "<!--StartfRagemnt...<span ...color:..>" via REGEX


            || doubleClickedWord_html.contains(
          QString{"<!--StartFragment--><span style=\" color:%1;\">"}
            .arg( this->textColors[TextTypeColor::HORIZONTAL_LINE_COLOR] ) )
        // TODO: check expected string-structure "<!--StartfRagemnt...<span ...color:..>" via REGEX
      )
    {
        return;
    }

    QString doubleClickedWord{ this->ui->textEdit->textCursor().selectedText().trimmed() };
    doubleClickedWord = this->removeSeperators( doubleClickedWord );

    if( !doubleClickedWord.isEmpty() )
    {
        const QString foreignLangTag = this->ui->comboBox_langs->currentText();
        const QString nativeLangTag = this->getNativeLang();

        TranslationDialog *translationDialog = new TranslationDialog{ this, this->dbManager };

        translationDialog->setUnknownWordLabelText( doubleClickedWord );
        translationDialog->updateUnknownWordTitle( this->ui->comboBox_langs->currentText() );
        translationDialog->updateTranslateToLangTitle( nativeLangTag );

        const int foreignLangID = this->dbManager->getLangId( foreignLangTag.toLower() );
        const int nativeLangID = this->dbManager->getLangId( nativeLangTag.toLower() );

        translationDialog->setForeignLangId( foreignLangID );
        translationDialog->setNativeLangId( nativeLangID );

        const QVector<QString> words = this->dbManager->getTanslations( doubleClickedWord, foreignLangID,  nativeLangID );

        QVector<std::pair<QString,int>> word_pairs;

        for( const QString &word : words )
        {
            const int wordID = this->dbManager->getWordId( word, nativeLangID );
            word_pairs.push_back( std::make_pair(word, wordID) );
        }

        translationDialog->fillTranslationTable( word_pairs );

        QObject::connect( translationDialog, &TranslationDialog::translationDeleted,
                          this, &MainWindow::onTranslationDeleted,
                          Qt::UniqueConnection );

        QObject::connect( translationDialog, &TranslationDialog::translationAdded,
                          this, &MainWindow::onTranslationAdded,
                          Qt::UniqueConnection );

        translationDialog->exec();
    }
}

void MainWindow::onTranslationDeleted( QString foreignWord, QString translation )
{
    const int lastScrollPosition{ this->ui->textEdit->getScrollPosition() };

    this->chachedTranslations[foreignWord].removeTranslation( translation );

    if( !this->chachedTranslations[foreignWord].hasTranslations() )
    {
        this->chachedTranslations.remove( foreignWord );
    }

    this->resetStatistic();
    this->ui->textEdit->setText( this->originForeignText );
    this->on_pushButton_analyse_clicked();

    this->ui->textEdit->setScrollPosition( lastScrollPosition );
}


void MainWindow::onTranslationAdded( QString foreignWord, QString translation )
{
    const int lastScrollPosition{ this->ui->textEdit->getScrollPosition() };

    this->updateCachedWord( foreignWord, translation );

    this->resetStatistic();
    this->ui->textEdit->setText( this->originForeignText );
    this->on_pushButton_analyse_clicked();

    this->ui->textEdit->setScrollPosition( lastScrollPosition );
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
    const QString unknownWordStyleText{
        QString{"<span style=\" color:%1\">" }
        .arg( this->textColors[TextTypeColor::FOREIGN_TEXT_UNKNOWN_COLOR] ) };
    const QString knownWordStyleText{
        QString{"<span style=\" color:%1\">" }
        .arg( this->textColors[TextTypeColor::FOREIGN_TEXT_KNOWN_COLOR] ) };
    const QString nativeWordStyleText{
        QString{"<span style=\" color:%1\">" }
        .arg( this->textColors[TextTypeColor::NATIVE_UNMARKED_TEXT_COLOR] ) };

    QString htmlText{ this->ui->textEdit->toHtml() };
    htmlText.replace( unknownWordStyleText, nativeWordStyleText );
    htmlText.replace( knownWordStyleText, nativeWordStyleText );

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

QString MainWindow::restoreForeignText() const
{
    QString foreignText;

    for( const Word &word : this->foreign_words )
    {
        foreignText.append( word.getContent() );
    }

    return foreignText;
}

void MainWindow::reset()
{
    this->resetStatistic();
    this->resetHighlighting();

    this->ui->textEdit->setTextColor( QColor::fromRgb( 0,0,0 ) );
    this->ui->textEdit->setFontWeight( QFont::Weight::Normal );
    this->ui->textEdit->setReadOnly( false );
    this->ui->comboBox_langs->setEnabled( true );
    this->ui->pushButton_analyse->setEnabled( true );
    this->ui->pushButton_edit->setEnabled( false );
}

void MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                "",
                                tr("Text File (*.txt)"));

    if( !fileName.isEmpty() )
    {
        if( !this->fileChangeWatcher->files().isEmpty() )
        {
            this->fileChangeWatcher->removePath( this->openedFileName );
        }

        this->openedFileName = fileName;

        this->saveTo( this->openedFileName );

        this->fileChangeWatcher->addPath( this->openedFileName );

        this->ui->action_Save->setEnabled( true );
    }
    else
    {
        qDebug() << "Cancelled save as...";
    }
}

void MainWindow::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Text File"), "", tr("Text File (*.txt)") );

    if( fileName.isEmpty() )
    {
        return;
    }

    qDebug() << "Selected File: " << fileName;

    QFile f( fileName );
    f.open( QFile::ReadOnly | QFile::Text );

    QTextStream fileStream( &f );

    this->reset();

    this->ui->textEdit->setText( fileStream.readAll() );


    if( this->mode == Mode::TRANSLATE_MODE )
    {
        this->switchMode();
    }

    if( !this->fileChangeWatcher->files().isEmpty() )
    {
        this->fileChangeWatcher->removePath( this->openedFileName );
    }

    this->openedFileName = fileName;

    this->fileChangeWatcher->addPath( this->openedFileName );
    this->ui->action_Save->setEnabled( true );
}

void MainWindow::saveTo( const QString &fileName ) const
{
    QFile f( fileName );
    f.open( QFile::WriteOnly | QFile::Truncate | QFile::Text );

    QTextStream fileStream( &f );
    const QString content = this->ui->textEdit->toPlainText();

    fileStream << content ;

    f.close();
}

void MainWindow::on_pushButton_edit_clicked()
{
    this->reset();

    this->ui->textEdit->setText( this->originForeignText );

    this->switchMode();
}

void MainWindow::on_action_Settings_triggered()
{
    SettingDialog *dialog = new SettingDialog{ this, this->dbManager };

    QObject::connect( dialog, &SettingDialog::langChangedSignal,
                      this, &MainWindow::onLangChanged,
                      Qt::UniqueConnection );

    dialog->exec();
}

void MainWindow::on_textEdit_textChanged()
{
}

void MainWindow::on_action_Open_triggered()
{
    this->loadFromFile();
}

void MainWindow::on_action_Save_triggered()
{
    if( this->openedFileName.isEmpty() || this->mode == Mode::TRANSLATE_MODE )
    {
        return;
    }

    bool overrideFile = true;

    if( this->openFileChangedFromExtern )
    {
        if( QMessageBox::No == QMessageBox( QMessageBox::Warning,
                                            "File changed",
                                            "Current file is changed. Override file?",
                                            QMessageBox::No|QMessageBox::Yes).exec() )
        {
            overrideFile = false;
        }
        else
        {
            overrideFile = true;
        }
    }

    if( overrideFile )
    {
        this->fileChangeWatcher->removePath( this->openedFileName );

        this->saveTo( this->openedFileName );

        this->openFileChangedFromExtern = false;
        this->fileChangeWatcher->addPath( this->openedFileName );
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    this->saveAsFile();
}

void MainWindow::onEscape()
{
    if( this->mode == Mode::EDIT_MODE )
    {
        return;
    }

    this->reset();
    this->switchToEditMode();
    this->ui->textEdit->setText( this->originForeignText );
}
