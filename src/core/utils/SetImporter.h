/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SetImporter class - header file.
 */
#pragma once
#include <QString>
#include <memory>

#include "../../db/DatabaseManager.h"

class SetImporter {
public:
    static bool importFile( const QString& filepath, DatabaseManager& db, QString& error_msg );
};