/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of JSON parsing logic to import study sets from files.
 */
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "JsonImportStrategy.h"
#include "../../learning/Card.h"

using namespace std;

bool JsonImportStrategy::import( const QString& file_path, DatabaseManager& db,
                                 QString& set_name_out ) {
    QFile file( file_path );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qCritical() << "Could not open file:" << file_path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson( data );
    if ( doc.isNull() ) {
        qCritical() << "Invalid JSON file.";
        return false;
    }

    QJsonObject root = doc.object();

    if ( root.contains( "name" ) && root["name"].isString() ) {
        set_name_out = root["name"].toString();
    } else {
        set_name_out = "Importowany Zestaw";
    }

    if ( !root.contains( "cards" ) || !root["cards"].isArray() ) {
        qCritical() << "JSON does not contain 'cards' array.";
        return false;
    }

    QJsonArray cards_array = root["cards"].toArray();
    vector<DraftCard> cards_to_import;

    for ( const auto& val : cards_array ) {
        QJsonObject obj = val.toObject();

        string q_str = obj["question"].toString().toStdString();
        string c_str = obj["correct_answer"].toString().toStdString();

        vector<string> wrongs;
        if ( obj.contains( "wrong_answers" ) && obj["wrong_answers"].isArray() ) {
            QJsonArray w_arr = obj["wrong_answers"].toArray();
            for ( const auto& w : w_arr ) {
                wrongs.push_back( w.toString().toStdString() );
            }
        }

        DraftCard draft;

        draft.question = TextContent{ q_str };

        draft.correct_answer = c_str;
        draft.wrong_answers = wrongs;

        if ( !wrongs.empty() ) {
            draft.answer_type = AnswerType::TEXT_CHOICE;
        } else {
            draft.answer_type = AnswerType::FLASHCARD;
        }

        cards_to_import.push_back( draft );
    }

    if ( cards_to_import.empty() ) {
        return false;
    }
    return db.createSet( set_name_out.toStdString(), cards_to_import );
}