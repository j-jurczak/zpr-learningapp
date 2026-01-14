/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the JSON specific import strategy implementation.
 */
#pragma once
#include "IImportStrategy.h"

class JsonImportStrategy : public IImportStrategy {
public:
    bool import( const QString& filepath, DatabaseManager& db, QString& error_msg ) override;
};