#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>
#include <QVector>
#include <QPair>
#include <QFileSystemWatcher>

#include "db_manager.h"
#include "word.h"

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

    static QString normalizeVersion( const QString &version );
    static QString revision( const QString &version );

    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow() override;

    QString getSelectedText() const;
    QString getNativeLang() const;

private slots:
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

private:
    void fillComboBox();
    QString colorizeWord( QString foreignWord, const bool isTranslated );
    QString maskHtml( const QChar &ch ) const;
    void resetHighlighting();
    void resetStatistic();
    void buildTranslationStructure( const QVector<Word> &foreign_words );
    QString newText();
    QString mergeLanguages( const QString &foreignText, const QString &nativeText ) const;
    QString htmlWord( QString word, const QString &styleColor = "black" ) const;
    QString cascadeHtmlSpace( const int count ) const;
    QString restoreForeignText() const;
    void analyse();
    void loadFromFile();
    void reset();
    void switchMode();

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
};

#endif // MAINWINDOW_H
