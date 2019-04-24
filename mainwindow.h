#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <list>

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
    QString getNativeLang() const
    ;
public slots:
    void onDbConnectionTimeOut();

private slots:
    void onDoubleClicked();

    void on_actionAbout_Qt_triggered();
    void on_action_Exit_triggered();
    void on_actionAbout_My_Cute_Thesaurus_triggered();
    void on_pushButton_analyse_clicked();

    void on_comboBox_langs_currentTextChanged( const QString &arg1 );

    void on_textEdit_textChanged();

    void on_pushButton_edit_clicked();

private:
    void fillComboBox();
    QString colorizeWord( QString foreignWord );
    QString maskHtml( const QChar &ch );
    void resetHighlighting();
    void resetStatistic();
    void reorganiseDataStructure( const std::vector<Word> &foreign_words );
    QString newText();

    Ui::MainWindow *ui;
    std::list<Word> native_words;
    std::list<Word> foreign_words;

    DB_Manager *dbManager;
    QTimer *dbConnectionCheckTimer;
    bool analysed;
    int knownWords;
    int unknownWords;
};

#endif // MAINWINDOW_H
