/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Main file for the LearningApp project
 */
#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include "db/DatabaseManager.h"
#include "gui/MainWindow.h"
#include "gui/views/ViewFactory.h"
#include "core/utils/LanguageManager.h"

int main( int argc, char *argv[] ) {
    qputenv( "LIBGL_ALWAYS_SOFTWARE", "1" );
    QApplication app( argc, argv );

    // Load language from settings, default to English ("en")
    QSettings settings( "ZPR", "LearningApp" );
    QString lang = settings.value( "language", "en" ).toString();

    // Load language using new modular manager
    LanguageManager::loadLanguage( lang );

    // database setup
    DatabaseManager db_manager;
    if ( !db_manager.connect() || !db_manager.createTables() ) return -1;
    // db_manager.seedData();

    // run app
    ViewFactory view_factory( db_manager );
    MainWindow main_window( view_factory );
    main_window.show();
    return app.exec();
}