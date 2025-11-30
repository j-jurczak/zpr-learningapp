/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Main file for the LearningApp project
 */
#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include "db/DatabaseManager.h"
#include "gui/MainWindow.h"
#include "gui/views/ViewFactory.h"

int main( int argc, char *argv[] ) {
    qputenv( "LIBGL_ALWAYS_SOFTWARE", "1" );  // to fix my fucking wls
    QApplication app( argc, argv );

    // database setup
    DatabaseManager db_manager;
    if ( !db_manager.connect() || !db_manager.createTables() ) return -1;
    db_manager.flushData();
    db_manager.seedData();

    // run app
    ViewFactory view_factory( db_manager );
    MainWindow main_window( view_factory );
    main_window.show();
    return app.exec();
}