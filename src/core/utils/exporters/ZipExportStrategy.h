/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for ZIP export strategy.
 */
#pragma once
#include "IExportStrategy.h"

class ZipExportStrategy : public IExportStrategy {
public:
    bool exportSet( int set_id, const DatabaseManager& db, const QString& dest_path ) override;

private:
    static bool createZipArchive( const QString& source_dir, const QString& dest_zip );
};
