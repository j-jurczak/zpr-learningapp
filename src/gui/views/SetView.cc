/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - source file.
 */
#include "SetView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

SetView::SetView( int set_id, DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), set_id_( set_id ), db_( db ) {
    setupUi();
    loadData();
}

void SetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 20, 20, 20, 20 );

    auto set_info = db_.getSet( set_id_ );
    std::string title_text = set_info ? set_info->name : "Nieznany zestaw";

    QLabel* title = new QLabel( QString::fromStdString( title_text ), this );
    title->setStyleSheet( "font-size: 24px; font-weight: bold; margin-bottom: 10px;" );
    main_layout->addWidget( title );

    QHBoxLayout* btn_layout = new QHBoxLayout();

    QPushButton* btn_back = new QPushButton( "← Wróć", this );
    QPushButton* btn_study = new QPushButton( "Ucz się", this );
    QPushButton* btn_add = new QPushButton( "+ Dodaj pytanie", this );
    QPushButton* btn_delete = new QPushButton( "Usuń zestaw", this );

    btn_study->setStyleSheet( "background-color: #2e7d32; color: white; padding: 5px 15px;" );
    btn_delete->setStyleSheet( "background-color: #c62828; color: white; padding: 5px 15px;" );

    btn_layout->addWidget( btn_back );
    btn_layout->addStretch();
    btn_layout->addWidget( btn_delete );
    btn_layout->addWidget( btn_add );
    btn_layout->addWidget( btn_study );

    main_layout->addLayout( btn_layout );

    cards_list_ = new QListWidget( this );
    main_layout->addWidget( cards_list_ );

    connect( btn_back, &QPushButton::clicked, this, &SetView::backToSetsClicked );
}

void SetView::loadData() {
    cards_list_->clear();
    std::vector<Card> cards = db_.getCardsForSet( set_id_ );

    if ( cards.empty() ) {
        cards_list_->addItem( "Ten zestaw jest pusty. Dodaj pierwsze pytanie!" );
        return;
    }

    for ( const auto& card : cards ) {
        QString label = QString::fromStdString( card.getQuestion() ) + " -> " +
                        QString::fromStdString( card.getCorrectAnswer() );
        cards_list_->addItem( label );
    }
}