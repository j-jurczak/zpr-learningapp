/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary:  Factory class for creating different views in the application - source file.
 */
#include "ViewFactory.h"
#include "HomeView.h"
#include "SetsView.h"
#include "SettingsView.h"
#include "SetView.h"
#include "AddSetView.h"
#include "LearningView.h"

ViewFactory::ViewFactory( DatabaseManager& db ) : db_( db ) {}

QWidget* ViewFactory::create( ViewType type, QVariant data, QWidget* parent ) {
    switch ( type ) {
        case ViewType::SETS:
            return new SetsView( db_, parent );

        case ViewType::HOME:
            return new HomeView( parent );

        case ViewType::SETTINGS:
            return new SettingsView( parent );

        case ViewType::SET_VIEW: {
            bool ok = false;
            int set_id = data.toInt( &ok );
            if ( !ok ) return new SetsView( db_, parent );
            return new SetView( set_id, db_, parent );
        }

        case ViewType::ADD_SET:
            return new AddSetView( db_, parent );

        case ViewType::LEARNING:
            return new LearningView( db_, parent );

        default:
            return new QWidget( parent );
    }
}