/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - source file.
 */
#include "CardPreviewOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

CardPreviewOverlay::CardPreviewOverlay( const std::string& question, const std::string& correct,
                                        const std::vector<std::string>& wrong, QWidget* parent )
    : QWidget( parent ) {
    setupUi( question, correct, wrong );
}

void CardPreviewOverlay::setupUi( const std::string& question, const std::string& correct,
                                  const std::vector<std::string>& wrong ) {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 0, 0, 0, 0 );

    QFrame* content_frame = new QFrame( this );
    content_frame->setStyleSheet(
        "QFrame { background-color: #252526; border: 1px solid #454545; border-radius: 8px; "
        "min-width: 450px; max-width: 650px; }"
        "QLabel { color: #f0f0f0; font-size: 15px; border: none; }"
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
    connect( btn_close, &QPushButton::clicked, this, &CardPreviewOverlay::closeClicked );

    top_layout->addWidget( title );
    top_layout->addStretch();
    top_layout->addWidget( btn_close );
    card_layout->addWidget( top_bar );

    QWidget* body = new QWidget( content_frame );
    body->setStyleSheet( "border: none;" );
    QVBoxLayout* body_layout = new QVBoxLayout( body );
    body_layout->setContentsMargins( 20, 10, 20, 10 );
    body_layout->setSpacing( 10 );

    QLabel* lbl_q_header = new QLabel( "PYTANIE", body );
    lbl_q_header->setObjectName( "header" );
    body_layout->addWidget( lbl_q_header );

    QLabel* lbl_question = new QLabel( QString::fromStdString( question ), body );
    lbl_question->setWordWrap( true );
    lbl_question->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: white; margin-bottom: 5px;" );
    body_layout->addWidget( lbl_question );

    QLabel* lbl_a_header = new QLabel( "POPRAWNA ODPOWIEDŹ", body );
    lbl_a_header->setObjectName( "header" );
    body_layout->addWidget( lbl_a_header );

    QLabel* lbl_correct = new QLabel( "✔  " + QString::fromStdString( correct ), body );
    lbl_correct->setObjectName( "correct" );
    lbl_correct->setWordWrap( true );
    body_layout->addWidget( lbl_correct );

    if ( !wrong.empty() ) {
        QLabel* lbl_w_header = new QLabel( "BŁĘDNE ODPOWIEDZI", body );
        lbl_w_header->setObjectName( "header" );
        body_layout->addWidget( lbl_w_header );

        for ( const auto& w : wrong ) {
            QLabel* lbl_wrong = new QLabel( "✘  " + QString::fromStdString( w ), body );
            lbl_wrong->setObjectName( "wrong" );
            lbl_wrong->setWordWrap( true );
            body_layout->addWidget( lbl_wrong );
        }
    }

    card_layout->addWidget( body );
    main_layout->addWidget( content_frame );
}