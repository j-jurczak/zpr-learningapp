/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Factory class for creating different views in the application - header file.
 */

#pragma once
#include <QWidget>

#include "../../db/DatabaseManager.h"
#include "ViewType.h"

class ViewFactory {
public:
    explicit ViewFactory( DatabaseManager& db );

    QWidget* create( ViewType type, QWidget* parent = nullptr );

private:
    DatabaseManager& db_;
};