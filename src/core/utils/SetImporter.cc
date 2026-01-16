/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SetImporter class - source file.
 */
#include <QFileInfo>

#include "SetImporter.h"
#include "importers/IImportStrategy.h"
#include "importers/JsonImportStrategy.h"

using namespace std;

bool SetImporter::importFile( const QString& filepath, DatabaseManager& db, QString& error_msg ) {
    QFileInfo fi( filepath );
    QString ext = fi.suffix().toLower();

    unique_ptr<IImportStrategy> strategy = nullptr;

    if ( ext == "json" ) {
        strategy = make_unique<JsonImportStrategy>();
    } else {
        error_msg = "Cannot process file format: " + ext;
        return false;
    }

    return strategy->import( filepath, db, error_msg );
}