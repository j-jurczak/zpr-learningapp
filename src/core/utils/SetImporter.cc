/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SetImporter class - source file.
 */
#include "SetImporter.h"
#include "importers/IImportStrategy.h"
#include "importers/JsonImportStrategy.h"
#include <QFileInfo>

bool SetImporter::importFile( const QString& filepath, DatabaseManager& db, QString& error_msg ) {
    QFileInfo fi( filepath );
    QString ext = fi.suffix().toLower();

    std::unique_ptr<IImportStrategy> strategy = nullptr;

    if ( ext == "json" ) {
        strategy = std::make_unique<JsonImportStrategy>();
    } else {
        error_msg = "Nieobsługiwany format pliku: " + ext;
        return false;
    }

    return strategy->import( filepath, db, error_msg );
}