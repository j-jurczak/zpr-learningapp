/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of ZIP export strategy (logic moved from SetExporter).
 */
#include "ZipExportStrategy.h"
#include "../../learning/Card.h"
#include "../../utils/Overloaded.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>

using namespace std;

// Helper to copy file to export dir and return relative path
static QString copyMediaToExport( const string& relative_path, const QString& export_root ) {
    QString rel = QString::fromStdString( relative_path );

    // Determine source path
    QString source_full_path;
#ifdef PROJECT_ROOT
    QDir src_dir( QString( PROJECT_ROOT ) );
    source_full_path = src_dir.filePath( "data/media/" + rel );
#else
    source_full_path = QDir::current().filePath( "data/media/" + rel );
#endif

    QFileInfo src_info( source_full_path );
    if ( !src_info.exists() ) {
        qWarning() << "Media file missing:" << source_full_path;
        return "";
    }

    QString dest_full_path = QDir( export_root ).filePath( "media/" + rel );
    QFileInfo dest_info( dest_full_path );
    QDir dest_dir = dest_info.dir();

    if ( !dest_dir.exists() ) {
        dest_dir.mkpath( "." );
    }

    if ( QFile::copy( source_full_path, dest_full_path ) ) {
        return "media/" + rel;
    } else {
        qWarning() << "Failed to copy media file:" << source_full_path;
        return "";
    }
}

bool ZipExportStrategy::exportSet( int set_id, const DatabaseManager& db, const QString& dest_path ) {
    auto cards = db.getCardsForSet( set_id );

    QString set_name = "Exported Set";
    auto set_opt = db.getSet( set_id );
    if ( set_opt.has_value() ) {
        set_name = QString::fromStdString( set_opt->name );
    }

    // Create temporary directory for staging
    QTemporaryDir temp_dir;
    if ( !temp_dir.isValid() ) {
        qCritical() << "Could not create temporary directory.";
        return false;
    }
    QString temp_path = temp_dir.path();

    QJsonObject root;
    root["name"] = set_name;

    QJsonArray cards_arr;

    for ( const auto& card : cards ) {
        const auto& data = card.getData();
        QJsonObject c_obj;

        // Basic fields
        c_obj["correct_answer"] = QString::fromStdString( data.correct_answer );

        QJsonArray wrongs;
        for ( const auto& w : data.wrong_answers ) {
            wrongs.append( QString::fromStdString( w ) );
        }
        if ( !wrongs.empty() ) {
            c_obj["wrong_answers"] = wrongs;
        }

        // Question Payload & Media
        visit( overloaded{
            [&]( const TextContent& c ) {
                c_obj["question"] = QString::fromStdString( c.text );
                c_obj["media_type"] = "text";
            },
            [&]( const ImageContent& c ) {
                QString rel = copyMediaToExport( c.image_path, temp_path );
                c_obj["question"] = rel;
                c_obj["media_type"] = "image";
            },
            [&]( const SoundContent& c ) {
                QString rel = copyMediaToExport( c.sound_path, temp_path );
                c_obj["question"] = rel;
                c_obj["media_type"] = "sound";
            }
        }, data.question );

        cards_arr.append( c_obj );
    }

    root["cards"] = cards_arr;

    // Save JSON
    QJsonDocument doc( root );
    QString json_path = QDir( temp_path ).filePath( "data.json" );
    QFile file( json_path );
    if ( !file.open( QIODevice::WriteOnly ) ) {
        qCritical() << "Could not write data.json to temp dir.";
        return false;
    }
    file.write( doc.toJson() );
    file.close();

    // Create ZIP
    return createZipArchive( temp_path, dest_path );
}

bool ZipExportStrategy::createZipArchive( const QString& source_dir, const QString& dest_zip ) {
    // Remove existing file if any
    QFile::remove( dest_zip );

    QString program = "zip";
    QStringList arguments;
    arguments << "-r" << dest_zip << ".";

    QProcess zip_process;
    zip_process.setWorkingDirectory( source_dir );
    zip_process.start( program, arguments );

    if ( !zip_process.waitForStarted() || !zip_process.waitForFinished() ) {
        qCritical() << "Zip process failed to run.";
        return false;
    }

    if ( zip_process.exitCode() != 0 ) {
        qCritical() << "Zip process error:" << zip_process.readAllStandardError();
        return false;
    }

    return true;
}
