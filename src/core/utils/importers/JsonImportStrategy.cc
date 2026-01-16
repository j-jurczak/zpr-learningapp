/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of JSON parsing logic to import study sets from files.
 */
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "JsonImportStrategy.h"

using namespace std;

bool JsonImportStrategy::import( const QString& filepath, DatabaseManager& db,
                                 QString& error_msg ) {
    QFile file( filepath );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        error_msg = "Cannot open file: " + filepath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_err;
    QJsonDocument doc = QJsonDocument::fromJson( data, &parse_err );

    if ( parse_err.error != QJsonParseError::NoError ) {
        error_msg = "Error parsing JSON: " + parse_err.errorString();
        return false;
    }

    if ( !doc.isObject() ) {
        error_msg = "JSON must be an object.";
        return false;
    }

    QJsonObject root = doc.object();

    if ( !root.contains( "name" ) || !root["name"].isString() ) {
        error_msg = "Missing field 'name'.";
        return false;
    }
    string set_name = root["name"].toString().toStdString();

    if ( !root.contains( "cards" ) || !root["cards"].isArray() ) {
        error_msg = "Missing field 'cards'.";
        return false;
    }

    vector<DraftCard> cards_to_import;
    QJsonArray cards_array = root["cards"].toArray();

    for ( const auto& val : cards_array ) {
        if ( !val.isObject() ) continue;
        QJsonObject card_obj = val.toObject();

        if ( !card_obj.contains( "question" ) || !card_obj.contains( "correct" ) ) continue;

        string q = card_obj["question"].toString().toStdString();
        string c = card_obj["correct"].toString().toStdString();

        vector<string> wrongs;
        if ( card_obj.contains( "wrong" ) && card_obj["wrong"].isArray() ) {
            QJsonArray wrong_arr = card_obj["wrong"].toArray();
            for ( const auto& w : wrong_arr ) {
                wrongs.push_back( w.toString().toStdString() );
            }
        }
        cards_to_import.push_back( { q, c, wrongs } );
    }

    if ( cards_to_import.empty() ) {
        error_msg = "No valid cards to import.";
        return false;
    }

    if ( !db.createSet( set_name, cards_to_import ) ) {
        error_msg = "Database error.";
        return false;
    }

    return true;
}