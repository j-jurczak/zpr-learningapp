/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the ZIP specific import strategy implementation.
 */
#pragma once
#include "IImportStrategy.h"

class ZipImportStrategy : public IImportStrategy {
public:
    bool import( const QString& filepath, DatabaseManager& db, QString& error_msg ) override;
};
