/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SetImporter class - source file.
 */
#include <QFileInfo>

#include "SetImporter.h"
#include "importers/IImportStrategy.h"
#include "importers/ZipImportStrategy.h"
#include "importers/JsonImportStrategy.h"

#include <QTemporaryDir>
#include <QProcess>
#include <QDir>
#include <QDebug>

using namespace std;

bool SetImporter::importFile( const QString& filepath, DatabaseManager& db, QString& error_msg ) {
    QFileInfo fi( filepath );
    QString ext = fi.suffix().toLower();

    // Use unique_ptr for polymorphic strategy
    unique_ptr<IImportStrategy> strategy = nullptr;

    if ( ext == "zip" ) {
        strategy = make_unique<ZipImportStrategy>();
    } else if ( ext == "json" ) {
        strategy = make_unique<JsonImportStrategy>( "" );
    } else {
        error_msg = "Cannot process file format: " + ext;
        return false;
    }

    return strategy->import( filepath, db, error_msg );
}