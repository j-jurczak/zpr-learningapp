/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Factory class for creating different views in the application - header file.
 */

#pragma once
#include <QWidget>

#include "ViewType.h"

class ViewFactory {
public:
    static QWidget* create( ViewType type, QWidget* parent = nullptr );
};