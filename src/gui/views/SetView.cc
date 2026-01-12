/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - source file.
 */
#include "SetView.h"
#include "../overlays/CardPreviewOverlay.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QResizeEvent>

SetView::SetView( int set_id, DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), set_id_( set_id ), db_( db ) {
    overlay_ = new CardPreviewOverlay( this );

    setupUi();
    setupStyles();
    loadData();
}

void SetView::resizeEvent( QResizeEvent* event ) {
    QWidget::resizeEvent( event );
    if ( overlay_ ) {
        overlay_->resize( this->size() );
    }
}

void SetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 30, 30, 30 );
    main_layout->setSpacing( 20 );

    QHBoxLayout* top_bar = new QHBoxLayout();
    QPushButton* btn_back = new QPushButton( "← Wróć", this );
    btn_back->setObjectName( "btn_back" );
    btn_back->setCursor( Qt::PointingHandCursor );
    connect( btn_back, &QPushButton::clicked, this, &SetView::backToSetsClicked );

    title_label_ = new QLabel( "Ładowanie...", this );
    title_label_->setObjectName( "set_title" );

    top_bar->addWidget( btn_back );
    top_bar->addSpacing( 20 );
    top_bar->addWidget( title_label_ );
    top_bar->addStretch();

    QPushButton* btn_delete = new QPushButton( "Usuń zestaw", this );
    btn_delete->setObjectName( "btn_delete" );
    btn_delete->setCursor( Qt::PointingHandCursor );

    QPushButton* btn_add = new QPushButton( "+ Dodaj pytanie", this );
    btn_add->setObjectName( "btn_action" );

    QPushButton* btn_learn = new QPushButton( "Ucz się", this );
    btn_learn->setObjectName( "btn_learn" );
    btn_learn->setCursor( Qt::PointingHandCursor );

    top_bar->addWidget( btn_delete );
    top_bar->addWidget( btn_add );
    top_bar->addWidget( btn_learn );

    main_layout->addLayout( top_bar );

    main_layout->addWidget( new QLabel( "Lista kart w zestawie:", this ) );

    cards_list_ = new QListWidget( this );
    cards_list_->setObjectName( "cards_list" );
    cards_list_->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

    main_layout->addWidget( cards_list_ );

    connect( btn_delete, &QPushButton::clicked, this, [this]() {
        auto reply = QMessageBox::question(
            this, "Potwierdzenie", "Czy na pewno chcesz usunąć ten zestaw i wszystkie jego karty?",
            QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::Yes ) {
            // to-do: implement delete functionality in DatabaseManager
            emit backToSetsClicked();
        }
    } );

    connect( btn_learn, &QPushButton::clicked, this, [this]() { emit learnClicked( set_id_ ); } );
}

void SetView::setupStyles() {
    QFile file( ":/resources/SetView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }
}

void SetView::loadData() {
    auto set_opt = db_.getSet( set_id_ );
    if ( set_opt.has_value() ) {
        title_label_->setText( QString::fromStdString( set_opt->name ) );
    } else {
        title_label_->setText( "Nieznany zestaw" );
    }

    current_cards_ = db_.getCardsForSet( set_id_ );
    cards_list_->clear();

    if ( current_cards_.empty() ) {
        return;
    }

    for ( const auto& card : current_cards_ ) {
        QListWidgetItem* item = new QListWidgetItem( cards_list_ );
        item->setSizeHint( QSize( 0, 50 ) );

        QWidget* row_widget = new QWidget();
        QHBoxLayout* row_layout = new QHBoxLayout( row_widget );
        row_layout->setContentsMargins( 0, 0, 10, 0 );
        row_layout->setSpacing( 0 );

        QString question = QString::fromStdString( card.getQuestion() );
        QString correct = QString::fromStdString( card.getCorrectAnswer() );

        std::vector<std::string> wrong_vec;
        if ( card.isChoiceCard() ) {
            // to-do: need to handle different answer types
            auto choices = card.getChoices();
            for ( const auto& c : choices ) {
                if ( c != card.getCorrectAnswer() ) wrong_vec.push_back( c );
            }
        }

        QPushButton* btn_content = new QPushButton( question, row_widget );
        btn_content->setObjectName( "btn_card_row" );
        btn_content->setCursor( Qt::PointingHandCursor );
        btn_content->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

        row_layout->addWidget( btn_content );

        cards_list_->setItemWidget( item, row_widget );

        connect( btn_content, &QPushButton::clicked, this, [this, question, correct, wrong_vec]() {
            overlay_->showCard( question.toStdString(), correct.toStdString(), wrong_vec );
        } );
    }
}