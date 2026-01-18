/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of the single card addition logic within an overlay.
 */
#include <QPalette>
#include <QResizeEvent>

#include "OverlayContainer.h"

OverlayContainer::OverlayContainer( QWidget* parent ) : QWidget( parent ) {
    this->hide();

    this->setAutoFillBackground( true );
    QPalette pal = palette();
    pal.setColor( QPalette::Window, QColor( 0, 0, 0, 180 ) );
    this->setPalette( pal );

    main_layout_ = new QVBoxLayout( this );
    main_layout_->setAlignment( Qt::AlignCenter );
    main_layout_->setContentsMargins( 20, 20, 20, 20 );
}

void OverlayContainer::setContent( QWidget* widget ) {
    clearContent();

    if ( widget ) {
        current_content_ = widget;

        main_layout_->addWidget( current_content_ );

        current_content_->show();

        if ( parentWidget() ) {
            this->resize( parentWidget()->size() );
        }

        this->raise();
        this->show();
    }
}

void OverlayContainer::clearContent() {
    if ( current_content_ ) {
        main_layout_->removeWidget( current_content_ );
        current_content_->setParent( nullptr );
        current_content_->hide();
        current_content_ = nullptr;
    }
    this->lower();
    this->hide();
}

void OverlayContainer::resizeEvent( QResizeEvent* event ) { QWidget::resizeEvent( event ); }