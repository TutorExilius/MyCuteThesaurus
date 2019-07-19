#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>
#include <QVector>
#include <QPair>

#include "db_manager.h"
#include "word.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow() override;

    QString getSelectedText() const;
    QString getNativeLang() const;

public slots:
    void onDbConnectionTimeOut();

private slots:
    void onDoubleClicked();

    void on_actionAbout_Qt_triggered();
    void on_action_Exit_triggered();
    void on_actionAbout_My_Cute_Thesaurus_triggered();
    void on_pushButton_analyse_clicked();

    void on_comboBox_langs_currentTextChanged( const QString &arg1 );

    void on_pushButton_edit_clicked();

    void on_action_Settings_triggered();

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

    Ui::MainWindow *ui;
    QVector<Word> foreign_words;

    DB_Manager *dbManager;
    QTimer *dbConnectionCheckTimer;
    bool analysed;
    int knownWords;
    int unknownWords;
};

#endif // MAINWINDOW_H
