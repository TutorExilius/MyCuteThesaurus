#ifndef TRANSLATIONDIALOG_H
#define TRANSLATIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

// remove! ( DEBUG )
#include <QListWidget>

#include "mainwindow.h"
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

    void fillTranslationTable( const QVector<std::pair<QString,int>> &list );

protected:
    void keyPressEvent(QKeyEvent * e) override;

private slots:
    void on_pushButton_ok_clicked();
    void onItemChanged( QTableWidgetItem *item );
    void on_lineEdit_translateToLang_textChanged( const QString &text );
    void on_tableWidget_translations_itemDoubleClicked(QTableWidgetItem *item);

signals:
    void translationDeleted( QString foreignWord, QString translation );
    void translationAdded( QString foreignWord, QString translation );

private:
    void deleteItem( QTableWidgetItem *item );

    Ui::TranslationDialog *ui;
    int foreignLangId;
    int nativeLangId;
    DB_Manager *db_manager;
    QMap<int,QString> toDeleteTranslations;
    QString rememberedWordInSelectedItemWidget;
};

#endif // TRANSLATIONDIALOG_H
