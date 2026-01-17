/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the JSON specific import strategy implementation.
 */
#pragma once
#include "IImportStrategy.h"

class JsonImportStrategy : public IImportStrategy {
public:
    explicit JsonImportStrategy( const QString& media_root = "" );
    bool import( const QString& file_path, DatabaseManager& db, QString& set_name_out ) override;

private:
    QString media_root_;
};