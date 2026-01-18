#include <catch2/catch_test_macros.hpp>
#include <QCoreApplication>
#include <QFile>

#include "core/utils/LanguageManager.h"

TEST_CASE( "Language Manager", "[Utils]" ) {

    SECTION( "Load English" ) {
        LanguageManager::loadLanguage( "en" );
        REQUIRE( true );
    }

    SECTION( "Load Polish" ) {
        LanguageManager::loadLanguage( "pl" );
        REQUIRE( true );
    }

    SECTION( "Load Invalid" ) {
        LanguageManager::loadLanguage( "xx" );
        REQUIRE( true );
    }
}
