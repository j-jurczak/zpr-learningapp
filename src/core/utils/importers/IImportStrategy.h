/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Interface definition for file import strategies.
 */
#pragma once
#include <QString>
#include "../../../db/DatabaseManager.h"

class IImportStrategy {
public:
    virtual ~IImportStrategy() = default;

    virtual bool import( const QString& filepath, DatabaseManager& db, QString& error_msg ) = 0;
};