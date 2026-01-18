/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of JSON parsing logic to import study sets from files.
 */
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "JsonImportStrategy.h"
#include "../../learning/Card.h"

using namespace std;

JsonImportStrategy::JsonImportStrategy( const QString& media_root, bool strict_text_only )
    : media_root_( media_root ), strict_text_only_( strict_text_only ) {}

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
    if( doc.isNull() ) {
        qCritical() << "Invalid JSON file.";
        return false;
    }

    QJsonObject root = doc.object();

    if( root.contains( "name" ) && root["name"].isString() ) {
        set_name_out = root["name"].toString();
    } else {
        set_name_out = "Importowany Zestaw";
    }

    if ( !root.contains( "cards" ) || !root["cards"].isArray() ) {
        qCritical() << "JSON does not contain 'cards' array.";
        return false;
    }

    QJsonArray cards_array = root["cards"].toArray();

    if( strict_text_only_ ) {
        for( const auto& val : cards_array ) {
            QJsonObject obj = val.toObject();
            if( obj.contains( "media_type" ) ) {
                QString type = obj["media_type"].toString();
                if( type == "image" || type == "sound" ) {
                    set_name_out = "Import plików JSON obsługuje tylko pytania tekstowe.";
                    return false;
                }
            }
        }
    }
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

        if( !media_root_.isEmpty() &&
             ( obj["media_type"].toString() == "image" || obj["media_type"].toString() == "sound" ) ) {

            QString rel_path = QString::fromStdString( q_str );
            QString source_path = QDir( media_root_ ).filePath( rel_path );

            QFileInfo src_info( source_path );
            if ( src_info.exists() ) {
                QString ext = src_info.suffix();
                QString new_name = QString::number( QDateTime::currentMSecsSinceEpoch() ) +
                                   QString::number( qHash( rel_path ) ) + "." + ext;

                QString subfolder = ( obj["media_type"].toString() == "sound" ) ? "sounds" : "images";

                QString app_data_path;
                #ifdef PROJECT_ROOT
                    app_data_path = QString( PROJECT_ROOT );
                #else
                    app_data_path = QDir::currentPath();
                #endif

                QDir dest_dir( app_data_path );
                dest_dir.mkpath( "data/media/" + subfolder );
                QString dest_full = dest_dir.filePath( "data/media/" + subfolder + "/" + new_name );

                if ( QFile::copy( source_path, dest_full ) ) {
                    q_str = ( subfolder + "/" + new_name ).toStdString();
                } else {
                    qWarning() << "Failed to copy import media:" << source_path;
                }
            }
        }

        DraftCard draft;

        bool is_image = false;
        bool is_sound = false;

        if ( obj.contains( "media_type" ) ) {
            QString type = obj["media_type"].toString();
            if ( type == "image" ) is_image = true;
            else if ( type == "sound" ) is_sound = true;
        }

        if( is_image ) {
            draft.question = ImageContent{ q_str };
        } else if( is_sound ) {
            draft.question = SoundContent{ q_str };
        } else {
             draft.question = TextContent{ q_str };
        }

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