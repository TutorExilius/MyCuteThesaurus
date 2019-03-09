#include "customaboutdialog.h"

#include <QMessageBox>
#include <QWidget>

namespace CustomAboutDialog
{
    void about( QWidget *parent, const QMap<AboutSection,QString> &contents )
    {
        const QString logo = ( contents.contains(AboutSection::LOGO) )
                              ? contents[AboutSection::LOGO]
                              : "";

        const QString title = ( contents.contains(AboutSection::TITLE) )
                              ? contents[AboutSection::TITLE]
                              : "About";

        const QString version = ( contents.contains(AboutSection::VERSION) )
                              ? contents[AboutSection::VERSION]
                              : "";

        const QString date = ( contents.contains(AboutSection::DATE) )
                              ? contents[AboutSection::DATE]
                              : "";

        const QString short_info = ( contents.contains(AboutSection::SHORT_INFO) )
                              ? contents[AboutSection::SHORT_INFO]
                              : "";

        const QString description = ( contents.contains(AboutSection::DESCRIPTION) )
                              ? contents[AboutSection::DESCRIPTION]
                              : "";

        const QString resources = ( contents.contains(AboutSection::RESOURCES) )
                              ? contents[AboutSection::RESOURCES]
                              : "";

        const QString credits = ( contents.contains(AboutSection::CREDITS) )
                              ? contents[AboutSection::CREDITS]
                              : "";

        const QString author = ( contents.contains(AboutSection::AUTHOR) )
                              ? contents[AboutSection::AUTHOR]
                              : "";

        const QString licence = ( contents.contains(AboutSection::LICENCE) )
                              ? contents[AboutSection::LICENCE]
                              : "";

        #ifdef Q_OS_MAC
            static QPointer<QMessageBox> oldMsgBox;

            if (oldMsgBox) {
                oldMsgBox->show();
                oldMsgBox->raise();
                oldMsgBox->activateWindow();
                return;
            }
        #endif

        QString caption;
        QString content;

        caption = ( !title.isEmpty() )       // TITLE
                ? ("<h3>" + title + "</h3>")
                : "<h3>About</h3>";

        content = ( !version.isEmpty() )     // VERSION
              ? "" + version + ""
              : "";

        if( !date.isEmpty() )
        {
            if( !version.isEmpty() )
            {
                content += "<br>";
            }

            content += date;
        }

        content += ( !short_info.isEmpty() )     // SHORT_INFO
              ? "<p>" + short_info + "</p>"
              : "";
        content += ( !description.isEmpty() )     // DESCRIPTION
              ? "<p>" + description + "</p>"
              : "";
        content += ( !resources.isEmpty() )     // RESOURCES
              ? "<p>" + resources + "</p>"
              : "";
        content += ( !credits.isEmpty() )     // CREDITS
              ? "<p>" + credits + "</p>"
              : "";
        content += ( !author.isEmpty() )     // AUTHOR
              ? "<p><b>Author</b><br>" + author + "</p>"
              : "";
        content += ( !licence.isEmpty() )      // LICENCE
                ? "<p><b>Licence</b><br>" + licence + "</p>"
                : "";

        QMessageBox *msgBox = new QMessageBox{ parent };
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->setWindowTitle( contents[AboutSection::TITLE] );
        msgBox->setText(caption);
        msgBox->setInformativeText(content);

        QPixmap pm( logo );
        if (!pm.isNull())
            msgBox->setIconPixmap(pm);

            // should perhaps be a style hint
        #ifdef Q_OS_MAC
            oldMsgBox = msgBox;
        #if 0
            // ### doesn't work until close button is enabled in title bar
            msgBox->d_func()->autoAddOkButton = false;
        #else
            msgBox->d_func()->buttonBox->setCenterButtons(true);
        #endif
            msgBox->show();
        #else
            msgBox->exec();
        #endif
    }
}
