#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QFileSystemWatcher>

#include "db_manager.h"
#include "word.h"

// Forward-Declarations
class TranslationDialog;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class Mode
    {
        EDIT_MODE,
        TRANSLATE_MODE
    };

    enum class TextTypeColor
    {
        FOREIGN_TEXT_KNOWN_COLOR,
        FOREIGN_TEXT_UNKNOWN_COLOR,
        NATIVE_TEXT_COLOR,
        STATISTIC_KNOWN_WORDS_COLOR,
        STATISTIC_UNKNOWN_WORDS_COLOR,
        HORIZONTAL_LINE_COLOR
    };

    static QString normalizeVersion( const QString &version );
    static QString revision( const QString &version );

    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow() override;

    QString getSelectedText() const;
    QString getNativeLang() const;

private slots:
    void onTranslationDeleted( QString foreignWord, QString translation );
    void onTranslationAdded( QString foreignWord, QString translation );

    void onOpenFileChanged();
    void onDoubleClicked();

    void on_actionAbout_Qt_triggered();
    void on_action_Exit_triggered();
    void on_actionAbout_My_Cute_Thesaurus_triggered();
    void on_pushButton_analyse_clicked();

    void on_comboBox_langs_currentTextChanged( const QString &arg1 );

    void on_pushButton_edit_clicked();

    void on_action_Settings_triggered();

    void on_textEdit_textChanged();

    void on_action_Open_triggered();

    void on_action_Save_triggered();

    void on_actionSave_As_triggered();

private:
    void initialiseFileChangeWatcher();
    void fillComboBox();
    QString colorizeWord( QString foreignWord, const bool isTranslated );
    QString maskHtml( const QChar &ch ) const;
    QString maskHtml( const QString &content ) const;
    void resetHighlighting();
    void resetStatistic();
    void buildTranslationStructure( const QVector<Word> &foreign_words );
    QString newText();
    QString mergeLanguages( const QString &foreignText,
                            const QString &nativeText,
                            const int textEditViewWidth ) const;
    QString htmlWord( QString word, const QString &styleColor = "black" ) const;
    QString cascadeHtmlSpace( const int count ) const;
    QString restoreForeignText() const;
    void analyse();
    void saveAsFile();
    void loadFromFile();
    void saveTo( const QString &fileName ) const;
    void reset();
    void switchMode();
    QVector<QString> getTanslations( const QString &word,
                                     const int foreignLangID,
                                     const int nativeLangId,
                                     bool useCache = true ) const;

    inline void cacheWord( const Word &word );
    void updateCachedWord( const QString &foreignWord, const QString &translation );
    QString removeSeperators( const QString &word ) const;

    Ui::MainWindow *ui;
    QVector<Word> foreign_words;

    DB_Manager *dbManager;
    bool analysed;
    int knownWords;
    int unknownWords;
    int monospaceCharactersWidth;
    QString openedFileName;
    QFileSystemWatcher *fileChangeWatcher;
    bool openFileChangedFromExtern;
    Mode mode;
    QMap<TextTypeColor, QString> textColors;

    // Word as String -> Word as Objcet (with Translations inside)
    QMap<QString, Word> chachedTranslations;

    QString originForeignText;
};

#endif // MAINWINDOW_H
