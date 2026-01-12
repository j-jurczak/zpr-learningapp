/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - source file.
 */
#include "CardPreviewOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>

CardPreviewOverlay::CardPreviewOverlay( QWidget* parent ) : QWidget( parent ) {
    this->hide();

    this->setAutoFillBackground( true );

    QPalette pal = palette();
    pal.setColor( QPalette::Window, QColor( 0, 0, 0, 180 ) );
    this->setPalette( pal );

    setupUi();
}

void CardPreviewOverlay::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setAlignment( Qt::AlignCenter );
    main_layout->setContentsMargins( 20, 20, 20, 20 );

    QFrame* content_frame = new QFrame( this );
    content_frame->setObjectName( "preview_card" );
    content_frame->setStyleSheet(
        "QFrame#preview_card { "
        "   background-color: #252526; "
        "   border: 1px solid #454545; "
        "   border-radius: 8px; "
        "   min-width: 450px; "
        "   max-width: 650px; "
        "}"
        "QLabel { color: #f0f0f0; font-size: 15px; }"
        "QLabel#header { color: #888888; font-size: 11px; font-weight: bold; margin-top: 12px; "
        "margin-bottom: 4px; }"
        "QLabel#correct { color: #81c784; font-weight: bold; }"
        "QLabel#wrong { color: #e57373; }" );

    QVBoxLayout* card_layout = new QVBoxLayout( content_frame );
    card_layout->setContentsMargins( 0, 0, 0, 20 );
    card_layout->setSpacing( 5 );

    QWidget* top_bar = new QWidget( content_frame );
    top_bar->setStyleSheet( "background-color: transparent; border-bottom: 1px solid #333;" );
    top_bar->setFixedHeight( 40 );
    QHBoxLayout* top_layout = new QHBoxLayout( top_bar );
    top_layout->setContentsMargins( 15, 0, 5, 0 );

    QLabel* title = new QLabel( "PODGLĄD KARTY", top_bar );
    title->setStyleSheet( "color: #aaaaaa; font-weight: bold; font-size: 12px; border: none;" );

    QPushButton* btn_close = new QPushButton( "✕", top_bar );
    btn_close->setFixedSize( 30, 30 );
    btn_close->setCursor( Qt::PointingHandCursor );
    btn_close->setStyleSheet(
        "QPushButton { background-color: transparent; color: #cccccc; border: none; font-size: "
        "14px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #c42b1c; color: white; }" );
    connect( btn_close, &QPushButton::clicked, this, &QWidget::hide );

    top_layout->addWidget( title );
    top_layout->addStretch();
    top_layout->addWidget( btn_close );
    card_layout->addWidget( top_bar );

    QWidget* body = new QWidget( content_frame );
    QVBoxLayout* body_layout = new QVBoxLayout( body );
    body_layout->setContentsMargins( 20, 10, 20, 10 );
    body_layout->setSpacing( 10 );

    body_layout->addWidget( new QLabel( "PYTANIE", body ) );
    lbl_question_ = new QLabel( "", body );
    lbl_question_->setWordWrap( true );
    lbl_question_->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: white; margin-bottom: 5px;" );
    body_layout->addWidget( lbl_question_ );

    body_layout->addWidget( new QLabel( "POPRAWNA ODPOWIEDŹ", body ) );
    lbl_correct_ = new QLabel( "", body );
    lbl_correct_->setObjectName( "correct" );
    lbl_correct_->setWordWrap( true );
    body_layout->addWidget( lbl_correct_ );

    lbl_wrong_header_ = new QLabel( "BŁĘDNE ODPOWIEDZI", body );
    lbl_wrong_header_->setObjectName( "header" );
    body_layout->addWidget( lbl_wrong_header_ );

    wrong_container_ = new QFrame( body );
    wrong_layout_ = new QVBoxLayout( wrong_container_ );
    wrong_layout_->setContentsMargins( 0, 0, 0, 0 );
    wrong_layout_->setSpacing( 5 );
    body_layout->addWidget( wrong_container_ );

    card_layout->addWidget( body );
    main_layout->addWidget( content_frame );
}

void CardPreviewOverlay::showCard( const std::string& question, const std::string& correct,
                                   const std::vector<std::string>& wrong ) {
    lbl_question_->setText( QString::fromStdString( question ) );
    lbl_correct_->setText( "✔  " + QString::fromStdString( correct ) );

    QLayoutItem* item;
    while ( ( item = wrong_layout_->takeAt( 0 ) ) != nullptr ) {
        delete item->widget();
        delete item;
    }

    if ( wrong.empty() ) {
        lbl_wrong_header_->hide();
        wrong_container_->hide();
    } else {
        lbl_wrong_header_->show();
        wrong_container_->show();
        for ( const auto& w : wrong ) {
            QLabel* lbl = new QLabel( "✘  " + QString::fromStdString( w ), wrong_container_ );
            lbl->setObjectName( "wrong" );
            lbl->setWordWrap( true );
            wrong_layout_->addWidget( lbl );
        }
    }

    this->resize( parentWidget()->size() );
    this->raise();
    this->show();
}

void CardPreviewOverlay::mousePressEvent( QMouseEvent* event ) {
    // block clicks to underlying widgets
}