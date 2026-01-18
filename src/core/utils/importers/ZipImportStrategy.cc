/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of ZIP parsing logic to import study sets from archives.
 */
#include <QTemporaryDir>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <memory>

#include "ZipImportStrategy.h"
#include "JsonImportStrategy.h"

using namespace std;

bool ZipImportStrategy::import( const QString& filepath, DatabaseManager& db, QString& error_msg ) {
    QTemporaryDir temp_dir;
    if ( !temp_dir.isValid() ) {
        error_msg = "Could not create temporary directory for extraction.";
        return false;
    }

    qDebug() << "Extracting ZIP to:" << temp_dir.path();

    QProcess unzip;
    QString program = "unzip";
    QStringList arguments;
    arguments << "-o" << filepath << "-d" << temp_dir.path();

    unzip.start( program, arguments );
    if ( !unzip.waitForFinished() || unzip.exitCode() != 0 ) {
        error_msg = "Failed to unzip archive. Ensure 'unzip' is installed.";
        return false;
    }

    QString json_path = QDir( temp_dir.path() ).filePath( "data.json" );
    if ( !QFile::exists( json_path ) ) {
        error_msg = "Invalid archive: data.json not found.";
        return false;
    }

    JsonImportStrategy jsonStrategy( temp_dir.path() );
    return jsonStrategy.import( json_path, db, error_msg );
}
