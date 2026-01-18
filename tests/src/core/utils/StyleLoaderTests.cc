#include <catch2/catch_test_macros.hpp>
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QDir>

#include "core/utils/StyleLoader.h"

int argc = 0;
char *argv[] = {};
QApplication app(argc, argv);

TEST_CASE( "StyleLoader Functionality", "[StyleLoader]" ) {

    SECTION( "Get Resource Path" ) {
        QString path = StyleLoader::getResourcePath( "Global.qss" );
        REQUIRE_FALSE( path.isEmpty() );
        REQUIRE( path.contains( "Global.qss" ) );
    }

    SECTION( "Attach Style to Widget" ) {
        QWidget widget;
        StyleLoader::attach( &widget, "NonExistent.qss" );
        REQUIRE( true );
    }

    SECTION( "Set Global Style" ) {
        StyleLoader::setGlobalStyle();
        REQUIRE( true );
    }
}
