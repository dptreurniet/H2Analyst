#include "h2analyst.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Forze");
    a.setOrganizationDomain("forzehydrogenracing.com");
    a.setApplicationName("H2Analyst");

    H2Analyst app;
    app.show();
    return a.exec();
}
