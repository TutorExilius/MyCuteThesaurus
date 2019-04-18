#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "db_manager.h"

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

private:
    void fillComboBox();
    QString colorizeWord( QString foreignWord );
    QString maskHtml( const QChar &ch );
    void check();
    void resetStatistic();

    Ui::MainWindow *ui;
    DB_Manager *dbManager;
    QTimer *dbConnectionCheckTimer;
    bool analysed;
    int knownWords;
    int unknownWords;
};

#endif // MAINWINDOW_H
