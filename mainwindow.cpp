#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMap>
#include <QMessageBox>

#include "customaboutdialog.h"
#include "log.h"

MainWindow::MainWindow( QWidget *parent )
: QMainWindow{ parent }
, ui{ new Ui::MainWindow }
, dbManager{ nullptr }
, dbConnectionCheckTimer{ nullptr }
{
    this->ui->setupUi( this );

    this->dbManager = new DB_Manager{ this, "mycutethesaurus.db" };

    this->dbConnectionCheckTimer = new QTimer{ this };

    QObject::connect( this->dbConnectionCheckTimer, &QTimer::timeout,
                      this, &MainWindow::onDbConnectionTimeOut,
                      Qt::UniqueConnection );

    dbConnectionCheckTimer->start( 1000 );
}

MainWindow::~MainWindow()
{
    delete ui;
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

    for( const QString &lang : langs )
    {
        this->ui->comboBox_langs->addItem( lang );
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
