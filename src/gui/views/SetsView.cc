/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Sets view of the application - source file.
 */

#include "SetsView.h"

#include <QLabel>
#include <QVBoxLayout>

SetsView::SetsView( QWidget* parent ) : QWidget( parent ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    // to-do: implement actual sets UI
    QLabel* label = new QLabel( "<lista zestawow pobranych z BD>", this );
    label->setAlignment( Qt::AlignCenter );
    label->setStyleSheet( "font-size: 24px; color: #aaaaaa;" );

    layout->addWidget( label );
}