/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Helper class for exporting sets to ZIP archive with media.
 */
#pragma once
#include <QString>

#include "../../db/DatabaseManager.h"

class SetExporter {
public:
    static bool exportSet( int set_id, const DatabaseManager& db, const QString& dest_path );

};
