/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Sets view of the application - source file.
 */
#include <QFile>
#include <QLabel>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include "SetsView.h"

SetsView::SetsView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_manager_( db ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 40, 40, 40, 40 );
    layout->setSpacing( 20 );

    QLabel* title = new QLabel( "Twoje Zestawy", this );
    title->setObjectName( "title" );
    title->setAlignment( Qt::AlignLeft );
    layout->addWidget( title );

    list_widget_ = new QListWidget( this );
    list_widget_->setObjectName( "list_widget" );
    layout->addWidget( list_widget_ );

    setupStyles();
    refreshSetsList();
}

void SetsView::refreshSetsList() {
    list_widget_->clear();
    std::vector<StudySet> sets = db_manager_.getAllSets();

    if ( sets.empty() ) {
        list_widget_->addItem( "Brak zestawów." );
        return;
    }

    for ( const auto& set : sets ) {
        QListWidgetItem* item = new QListWidgetItem( QString::fromStdString( set.name ) );
        item->setData( Qt::UserRole, set.id );
        list_widget_->addItem( item );
    }
}

void SetsView::setupStyles() {
    QFile file( ":/resources/SetsView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }
}