/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - source file.
 */
#include "SetView.h"
#include "../overlays/OverlayContainer.h"
#include "../overlays/AddCardOverlay.h"
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
    overlay_container_ = std::make_unique<OverlayContainer>( this );
    add_overlay_ = std::make_unique<AddCardOverlay>();

    connect( add_overlay_.get(), &AddCardOverlay::cancelled, overlay_container_.get(),
             &OverlayContainer::clearContent );

    connect( add_overlay_.get(), &AddCardOverlay::cardSaved, this, [this]( const DraftCard& c ) {
        if ( db_.addCardToSet( set_id_, c ) ) {
            loadData();
            overlay_container_->clearContent();
        }
    } );

    setupUi();

    QFile file( ":/resources/SetView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }

    loadData();
}

void SetView::resizeEvent( QResizeEvent* event ) {
    QWidget::resizeEvent( event );
    if ( overlay_container_ ) {
        overlay_container_->resize( this->size() );
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
        auto reply = QMessageBox::question( this, "Usuwanie zestawu",
                                            "Czy na pewno chcesz trwale usunąć ten zestaw?",
                                            QMessageBox::Yes | QMessageBox::No );
        if ( reply == QMessageBox::Yes ) {
            if ( db_.deleteSet( set_id_ ) ) emit backToSetsClicked();
        }
    } );

    connect( btn_add, &QPushButton::clicked, this, [this]() {
        add_overlay_->resetForm();
        overlay_container_->setContent( add_overlay_.get() );
    } );

    connect( btn_learn, &QPushButton::clicked, this, [this]() { emit learnClicked( set_id_ ); } );
}

void SetView::loadData() {
    auto set_opt = db_.getSet( set_id_ );
    if ( set_opt.has_value() )
        title_label_->setText( QString::fromStdString( set_opt->name ) );
    else
        title_label_->setText( "Nieznany zestaw" );

    current_cards_ = db_.getCardsForSet( set_id_ );
    cards_list_->clear();

    for ( const auto& card : current_cards_ ) {
        QListWidgetItem* item = new QListWidgetItem( cards_list_ );
        item->setSizeHint( QSize( 0, 50 ) );

        QWidget* row_widget = new QWidget();

        QHBoxLayout* row_layout = new QHBoxLayout( row_widget );
        row_layout->setContentsMargins( 0, 0, 10, 0 );
        row_layout->setSpacing( 10 );

        QString question = QString::fromStdString( card.getQuestion() );
        QString correct = QString::fromStdString( card.getCorrectAnswer() );
        int card_id = card.getId();

        std::vector<std::string> wrong_vec;
        if ( card.isChoiceCard() ) {
            auto choices = card.getChoices();
            for ( const auto& c : choices ) {
                if ( c != card.getCorrectAnswer() ) wrong_vec.push_back( c );
            }
        }

        QPushButton* btn_content = new QPushButton( question, row_widget );
        btn_content->setObjectName( "btn_card_row" );
        btn_content->setCursor( Qt::PointingHandCursor );
        btn_content->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

        connect( btn_content, &QPushButton::clicked, this, [this, question, correct, wrong_vec]() {
            current_preview_ = std::make_unique<CardPreviewOverlay>(
                question.toStdString(), correct.toStdString(), wrong_vec );
            auto* preview_ptr = static_cast<CardPreviewOverlay*>( current_preview_.get() );
            connect( preview_ptr, &CardPreviewOverlay::closeClicked, overlay_container_.get(),
                     &OverlayContainer::clearContent );
            overlay_container_->setContent( current_preview_.get() );
        } );

        QPushButton* btn_delete = new QPushButton( "✕", row_widget );
        btn_delete->setFixedSize( 30, 30 );
        btn_delete->setCursor( Qt::PointingHandCursor );

        btn_delete->setStyleSheet(
            "QPushButton { "
            "  background-color: transparent; "
            "  color: #555; "
            "  border: none; "
            "  font-weight: bold; "
            "  font-size: 16px; "
            "}"
            "QPushButton:hover { "
            "  color: #ff4444; "
            "  background-color: #333; "
            "  border-radius: 4px; "
            "}" );

        connect( btn_delete, &QPushButton::clicked, this, [this, card_id]() {
            auto reply = QMessageBox::question( this, "Usuń", "Usunąć tę kartę?",
                                                QMessageBox::Yes | QMessageBox::No );
            if ( reply == QMessageBox::Yes ) {
                if ( db_.deleteCard( card_id ) ) {
                    loadData();
                }
            }
        } );

        row_layout->addWidget( btn_content );
        row_layout->addWidget( btn_delete );

        cards_list_->setItemWidget( item, row_widget );
    }
}