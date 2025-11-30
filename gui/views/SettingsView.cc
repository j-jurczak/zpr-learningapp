/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Settings view of the application - source file.
 */
#include "SettingsView.h"

#include <QLabel>
#include <QVBoxLayout>

SettingsView::SettingsView( QWidget* parent ) : QWidget( parent ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    // to-do: implement actual settings UI
    QLabel* label = new QLabel( "<ustawienia aplikacji>", this );
    label->setAlignment( Qt::AlignCenter );
    label->setStyleSheet( "font-size: 24px; color: #aaaaaa;" );

    layout->addWidget( label );
}