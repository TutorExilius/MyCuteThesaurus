#ifndef CUSTOMABOUTDIALOG_H
#define CUSTOMABOUTDIALOG_H

#include <QMap>
#include <QString>

// forward-declarations
class QWidget;

enum class AboutSection
{
    TITLE,
    VERSION,
    DATE,
    SHORT_INFO,
    DESCRIPTION,
    RESOURCES,
    CREDITS,
    AUTHOR,
    LICENCE,
    LOGO
};

namespace CustomAboutDialog
{  
    void about( QWidget *parent, const QMap<AboutSection,QString> &contents );
};

#endif // CUSTOMABOUTDIALOG_H
