/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Home view of the application - source file.
 */
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "HomeView.h"

HomeView::HomeView( QWidget* parent ) : QWidget( parent ) {
    this->setObjectName( "content" );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setAlignment( Qt::AlignCenter );
    layout->setSpacing( 20 );

    QLabel* title_label = new QLabel( "Witamy w LearningApp!", this );
    title_label->setObjectName( "title" );
    title_label->setAlignment( Qt::AlignCenter );

    QLabel* subtitle_label = new QLabel( "Czego się chcesz dziś nauczyć?", this );
    subtitle_label->setObjectName( "subtitle" );
    subtitle_label->setAlignment( Qt::AlignCenter );

    QHBoxLayout* buttons_layout = new QHBoxLayout();
    buttons_layout->setSpacing( 30 );
    buttons_layout->setAlignment( Qt::AlignCenter );

    btn_new_set_ = new QPushButton( "Nowy zestaw", this );
    btn_new_set_->setFixedSize( 160, 50 );
    btn_new_set_->setObjectName( "action_btn" );

    btn_import_ = new QPushButton( "Importuj", this );
    btn_import_->setFixedSize( 160, 50 );
    btn_import_->setObjectName( "action_btn" );

    buttons_layout->addWidget( btn_new_set_ );
    buttons_layout->addWidget( btn_import_ );

    layout->addStretch();
    layout->addWidget( title_label );
    layout->addWidget( subtitle_label );
    layout->addLayout( buttons_layout );
    layout->addStretch();
}