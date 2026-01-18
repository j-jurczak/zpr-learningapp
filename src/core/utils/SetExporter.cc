/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of SetExporter using Strategy pattern.
 */
#include "SetExporter.h"
#include "exporters/ZipExportStrategy.h"
#include <QFileInfo>
#include <memory>

using namespace std;

bool SetExporter::exportSet( int set_id, const DatabaseManager& db, const QString& dest_path ) {
    QFileInfo fi( dest_path );
    QString ext = fi.suffix().toLower();

    unique_ptr<IExportStrategy> strategy = nullptr;

    if ( ext == "zip" ) {
        strategy = make_unique<ZipExportStrategy>();
    } else {
        strategy = make_unique<ZipExportStrategy>();
    }

    if ( strategy ) {
        return strategy->exportSet( set_id, db, dest_path );
    }

    return false;
}
