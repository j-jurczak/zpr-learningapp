/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Interface definition for Set export strategies.
 */
#pragma once
#include <QString>
#include <vector>
#include "../../../db/DatabaseManager.h"

class IExportStrategy {
public:
    virtual ~IExportStrategy() = default;

    virtual bool exportSet( int set_id, const DatabaseManager& db, const QString& dest_path ) = 0;
};
