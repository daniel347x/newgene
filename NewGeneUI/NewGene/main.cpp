#include "newgenemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "..\..\NewGeneBackEnd\test.h"
#include "uistatusmanager.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    NewGeneMainWindow w;

    try
    {

        w.statusManager().PostStatus("Main window created.");

        w.show();
        return a.exec();

    }
    catch (boost::exception & e)
    {
        if( std::string const * error_desc = boost::get_error_info<newgene_error_description>(e) )
        {
            boost::format msg(error_desc->c_str());
            QMessageBox msgBox;
            msgBox.setText(msg.str().c_str());
            msgBox.exec();
        }
        else
        {
            boost::format msg("Unknown exception thrown");
            QMessageBox msgBox;
            msgBox.setText(msg.str().c_str());
            msgBox.exec();
        }
        return -1;
    }
    catch (std::exception & e)
    {
        boost::format msg("Exception thrown: %1%");
        msg % e.what();
        return -1;
    }

}
