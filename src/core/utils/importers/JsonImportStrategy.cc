/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of JSON parsing logic to import study sets from files.
 */
#include "JsonImportStrategy.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool JsonImportStrategy::import( const QString& filepath, DatabaseManager& db,
                                 QString& error_msg ) {
    QFile file( filepath );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        error_msg = "Nie można otworzyć pliku: " + filepath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_err;
    QJsonDocument doc = QJsonDocument::fromJson( data, &parse_err );

    if ( parse_err.error != QJsonParseError::NoError ) {
        error_msg = "Błąd parsowania JSON: " + parse_err.errorString();
        return false;
    }

    if ( !doc.isObject() ) {
        error_msg = "JSON musi być obiektem.";
        return false;
    }

    QJsonObject root = doc.object();

    if ( !root.contains( "name" ) || !root["name"].isString() ) {
        error_msg = "Brak pola 'name'.";
        return false;
    }
    std::string set_name = root["name"].toString().toStdString();

    if ( !root.contains( "cards" ) || !root["cards"].isArray() ) {
        error_msg = "Brak pola 'cards'.";
        return false;
    }

    std::vector<DraftCard> cards_to_import;
    QJsonArray cards_array = root["cards"].toArray();

    for ( const auto& val : cards_array ) {
        if ( !val.isObject() ) continue;
        QJsonObject card_obj = val.toObject();

        if ( !card_obj.contains( "question" ) || !card_obj.contains( "correct" ) ) continue;

        std::string q = card_obj["question"].toString().toStdString();
        std::string c = card_obj["correct"].toString().toStdString();

        std::vector<std::string> wrongs;
        if ( card_obj.contains( "wrong" ) && card_obj["wrong"].isArray() ) {
            QJsonArray wrong_arr = card_obj["wrong"].toArray();
            for ( const auto& w : wrong_arr ) {
                wrongs.push_back( w.toString().toStdString() );
            }
        }
        cards_to_import.push_back( { q, c, wrongs } );
    }

    if ( cards_to_import.empty() ) {
        error_msg = "Brak poprawnych kart do zaimportowania.";
        return false;
    }

    if ( !db.createSet( set_name, cards_to_import ) ) {
        error_msg = "Błąd bazy danych.";
        return false;
    }

    return true;
}