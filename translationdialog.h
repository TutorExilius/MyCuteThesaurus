#ifndef TRANSLATIONDIALOG_H
#define TRANSLATIONDIALOG_H

#include <QDialog>

#include "db_manager.h"

namespace Ui {
    class TranslationDialog;
}

class TranslationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TranslationDialog( QWidget *parent, DB_Manager *db_manager );
    ~TranslationDialog() override;

    void setUnknownWordLabelText( const QString &unknownWord );
    void updateUnknownWordTitle( const QString &lang );
    void updateTranslateToLangTitle( const QString &lang );
    void setForeignLangId( const int &foreignLangId );
    void setNativeLangId( const int &nativeLangId );

private slots:
    void on_pushButton_add_clicked();

private:
    Ui::TranslationDialog *ui;

    int foreignLangId;
    int nativeLangId;
    DB_Manager *db_manager;
};

#endif // TRANSLATIONDIALOG_H
