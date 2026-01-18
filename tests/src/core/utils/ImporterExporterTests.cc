#include <catch2/catch_test_macros.hpp>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "db/DatabaseManager.h"
#include "core/utils/SetImporter.h"
#include "core/utils/SetExporter.h"
#include "core/utils/importers/JsonImportStrategy.h"

using namespace std;

TEST_CASE( "Import Export Functionality", "[ImpExp]" ) {
    QString test_db_name = "impexp_test.db";
    DatabaseManager db( test_db_name );
    db.connect();
    db.createTables();
    db.flushData();

    QTemporaryDir temp_dir;
    REQUIRE( temp_dir.isValid() );

    SECTION( "JSON Import" ) {
        QJsonObject root;
        root["name"] = "Json Set";
        QJsonArray cards;
        QJsonObject c1;
        c1["question"] = "JQ1";
        c1["correct_answer"] = "JA1";
        cards.append( c1 );
        root["cards"] = cards;

        QFile file( temp_dir.path() + "/test.json" );
        REQUIRE( file.open( QIODevice::WriteOnly ) );
        file.write( QJsonDocument( root ).toJson() );
        file.close();

        QString error;
        REQUIRE( SetImporter::importFile( file.fileName(), db, error ) );

        vector<StudySet> sets = db.getAllSets();
        REQUIRE( sets.size() == 1 );
        REQUIRE( sets[0].name == "Json Set" );

        vector<Card> db_cards = db.getCardsForSet( sets[0].id );
        REQUIRE( db_cards.size() == 1 );
        REQUIRE( db_cards[0].getQuestion() == "JQ1" );
    }

    SECTION( "Invalid JSON Import" ) {
        QFile file( temp_dir.path() + "/bad.json" );
        REQUIRE( file.open( QIODevice::WriteOnly ) );
        file.write( "{ bad json " );
        file.close();

        QString error;
        REQUIRE_FALSE( SetImporter::importFile( file.fileName(), db, error ) );
    }

    SECTION( "JSON Media Restriction" ) {
        QJsonObject root;
        root["name"] = "Media Set";
        QJsonArray cards;
        QJsonObject c1;
        c1["question"] = "image.png";
        c1["correct_answer"] = "A";
        c1["media_type"] = "image";
        cards.append( c1 );
        root["cards"] = cards;

        QFile file( temp_dir.path() + "/media.json" );
        REQUIRE( file.open( QIODevice::WriteOnly ) );
        file.write( QJsonDocument( root ).toJson() );
        file.close();

        QString error;
        REQUIRE_FALSE( SetImporter::importFile( file.fileName(), db, error ) );
    }

    SECTION( "Zip Export" ) {
        vector<DraftCard> cards;
        DraftCard c1;
        c1.question = TextContent{ "EQ1" };
        c1.correct_answer = "EA1";
        c1.answer_type = AnswerType::FLASHCARD;
        cards.push_back( c1 );
        db.createSet( "Export Set", cards );

        int set_id = db.getAllSets()[0].id;
        QString export_path = temp_dir.path() + "/exported.zip";

        REQUIRE( SetExporter::exportSet( set_id, db, export_path ) );
        REQUIRE( QFile::exists( export_path ) );

        QString error;
        REQUIRE( SetImporter::importFile( export_path, db, error ) );

        vector<StudySet> sets = db.getAllSets();
        REQUIRE( sets.size() == 2 );
        REQUIRE( sets.back().name == "Export Set" );
    }

    SECTION( "Media Handling in Import/Export" ) {
        QString image_name = "test_image.png";
        QString media_root = temp_dir.path();
        QDir(media_root).mkpath(".");
        QFile image_file( media_root + "/" + image_name );
        REQUIRE( image_file.open(QIODevice::WriteOnly) );
        image_file.write("fake image content");
        image_file.close();

        QJsonObject root;
        root["name"] = "Media Import Set";
        QJsonArray cards;
        QJsonObject c1;
        c1["question"] = image_name;
        c1["correct_answer"] = "A";
        c1["media_type"] = "image";
        cards.append( c1 );
        root["cards"] = cards;

        QString json_path = temp_dir.path() + "/media_valid.json";
        QFile file( json_path );
        REQUIRE( file.open( QIODevice::WriteOnly ) );
        file.write( QJsonDocument( root ).toJson() );
        file.close();

        QString error;

        REQUIRE_FALSE( SetImporter::importFile( json_path, db, error ) );

        QString set_name;
        JsonImportStrategy strategy( temp_dir.path(), false );
        REQUIRE( strategy.import( json_path, db, set_name ) );

        vector<StudySet> sets = db.getAllSets();
        REQUIRE( sets.size() == 1 );
        REQUIRE( sets[0].name == "Media Import Set" );

        vector<Card> cards_db = db.getCardsForSet(sets[0].id);
        REQUIRE( cards_db.size() == 1 );
        REQUIRE( cards_db[0].getMediaType() == MediaType::IMAGE );

        QString zip_path = temp_dir.path() + "/export_media.zip";
        REQUIRE( SetExporter::exportSet( sets.back().id, db, zip_path ) );
        REQUIRE( QFile::exists(zip_path) );

        REQUIRE( SetImporter::importFile( zip_path, db, error ) );
        sets = db.getAllSets();
        REQUIRE( sets.size() >= 2 );
    }

    QFile::remove( QDir::current().filePath( "data/" + test_db_name ) );
}
