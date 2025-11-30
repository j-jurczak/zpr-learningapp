/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary:  Factory class for creating different views in the application - source file.
 */

#include "ViewFactory.h"

#include <QWidget>

#include "views/HomeView.h"
#include "views/SetsView.h"
#include "views/SettingsView.h"

QWidget* ViewFactory::create( ViewType type, QWidget* parent ) {
    switch ( type ) {
        case ViewType::HOME:
            return new HomeView( parent );

        case ViewType::SETS:
            return new SetsView( parent );

        case ViewType::SETTINGS:
            return new SettingsView( parent );

        default:
            return new QWidget( parent );
    }
}