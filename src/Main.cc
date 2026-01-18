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
#include "core/utils/StyleLoader.h"

int main( int argc, char *argv[] ) {
    qputenv( "LIBGL_ALWAYS_SOFTWARE", "1" );
    QApplication app( argc, argv );

    StyleLoader::setGlobalStyle();

    // Load language from settings
    QSettings settings( "ZPR", "LearningApp" );
    QString lang = settings.value( "language", "en" ).toString();

    LanguageManager::loadLanguage( lang );

    // database setup
    DatabaseManager db_manager;
    if ( !db_manager.connect() || !db_manager.createTables() ) return -1;

    ViewFactory view_factory( db_manager );
    MainWindow main_window( view_factory );
    main_window.show();
    return app.exec();
}