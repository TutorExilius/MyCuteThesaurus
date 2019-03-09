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

public slots:
    void onDbConnectionTimeOut();

private slots:
    void on_actionAbout_Qt_triggered();

    void on_action_Exit_triggered();

    void on_actionAbout_My_Cute_Thesaurus_triggered();

private:
    void fillComboBox();

    Ui::MainWindow *ui;
    DB_Manager *dbManager;
    QTimer *dbConnectionCheckTimer;
};

#endif // MAINWINDOW_H
