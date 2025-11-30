/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Main file for the LearningApp application
 */

#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>

#include "gui/MainWindow.h"

bool connectToDatabase() {
    QSqlDatabase db_instance = QSqlDatabase::addDatabase( "QSQLITE" );
    db_instance.setDatabaseName( "learning_app.db" );

    if ( !db_instance.open() ) {
        qCritical() << "Blad polaczenia z baza:" << db_instance.lastError().text();
        return false;
    }

    qDebug() << "Polaczono z baza danych!";
    return true;
}

int main( int argc, char *argv[] ) {
    qputenv( "LIBGL_ALWAYS_SOFTWARE", "1" );

    QApplication app( argc, argv );

    if ( !connectToDatabase() ) {
        return -1;
    }

    MainWindow main_window;
    main_window.show();

    return app.exec();
}