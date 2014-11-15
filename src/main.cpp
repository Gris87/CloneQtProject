#include <QApplication>
#include <QMessageBox>

#include "main/mainwindow.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    switch (argc)
    {
        case 1:
        {
            // Nothing
            w.show();

            // For debug
            /* */
            //if (!w.batchClone("D:/Work/ePC_SIM/ePC_SIM.pro", "D:/Temp"))
            //if (!w.batchClone("D:/Projects/My/Qt/CloneQtProject/CloneQtProject.pro", "D:/Temp"))
            if (!w.batchClone("D:/Projects/My/Qt/CloneQtProject/CloneQtProject.pro", "E:/Temp"))
            {
                return 1;
            }
            /* */
        }
        break;
        case 3:
        {
            w.show();

            if (!w.batchClone(argv[1], argv[2]))
            {
                return 1;
            }
        }
        break;
        default:
        {
            QString text = QCoreApplication::translate("main", "Incorrect arguments are given:");

            for (int i=1; i<argc; ++i)
            {
                text.append("\n");
                text.append(argv[i]);
            }

            QMessageBox::information(0, QCoreApplication::translate("main", "Incorrect arguments"), text);

            return 2;
        }
        break;
    }

    return a.exec();
}
