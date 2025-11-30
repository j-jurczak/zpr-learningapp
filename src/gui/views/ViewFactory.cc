/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary:  Factory class for creating different views in the application - source file.
 */
#include "ViewFactory.h"
#include "HomeView.h"
#include "SetsView.h"
#include "SettingsView.h"

ViewFactory::ViewFactory( DatabaseManager& db ) : db_( db ) {}

QWidget* ViewFactory::create( ViewType type, QWidget* parent ) {
    switch ( type ) {
        case ViewType::SETS:
            return new SetsView( db_, parent );

        case ViewType::HOME:
            return new HomeView( parent );

        case ViewType::SETTINGS:
            return new SettingsView( parent );

        default:
            return new QWidget( parent );
    }
}