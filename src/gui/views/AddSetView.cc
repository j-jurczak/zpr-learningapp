/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - source file.
 */
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QDebug>
#include <variant>

#include "AddSetView.h"
#include "../overlays/AddCardOverlay.h"
#include "../overlays/OverlayContainer.h"
#include "../../core/utils/StyleLoader.h"

using namespace std;

AddSetView::AddSetView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_( db ) {
    overlay_container_ = make_unique<OverlayContainer>( this );
    setupUi();
    StyleLoader::attach( this, "views/AddSetView.qss" );
}

void AddSetView::resizeEvent( QResizeEvent* event ) {
    QWidget::resizeEvent( event );
    if ( overlay_container_ ) {
        overlay_container_->resize( this->size() );
    }
}

void AddSetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 30, 30, 30 );
    main_layout->setSpacing( 20 );

    QLabel* header = new QLabel( tr( "Create New Set" ), this );
    header->setObjectName( "title" );
    main_layout->addWidget( header );

    main_layout->addWidget( new QLabel( tr( "Set name:" ), this ) );

    name_input_ = new QLineEdit( this );
    name_input_->setPlaceholderText( tr( "E.g. History - Dates" ) );
    main_layout->addWidget( name_input_ );

    main_layout->addWidget( new QLabel( tr( "Cards in set:" ), this ) );

    preview_list_ = new QListWidget( this );
    main_layout->addWidget( preview_list_ );

    btn_open_creator_ = new QPushButton( "+ " + tr( "Add new card" ), this );
    btn_open_creator_->setCursor( Qt::PointingHandCursor );
    btn_open_creator_->setMinimumHeight( 50 );
    btn_open_creator_->setObjectName( "btnAddCard" );

    connect( btn_open_creator_, &QPushButton::clicked, this, &AddSetView::onAddCardClicked );
    main_layout->addWidget( btn_open_creator_ );

    main_layout->addStretch();

    QHBoxLayout* bottom_btns = new QHBoxLayout();

    QPushButton* btn_cancel = new QPushButton( tr( "Cancel" ), this );
    btn_cancel->setCursor( Qt::PointingHandCursor );

    connect( btn_cancel, &QPushButton::clicked, this, [this]() { emit creationCancelled(); } );

    QPushButton* btn_save = new QPushButton( tr( "Create Set" ), this );
    btn_save->setCursor( Qt::PointingHandCursor );
    btn_save->setProperty( "type", "primary" );

    connect( btn_save, &QPushButton::clicked, this, &AddSetView::saveSet );

    bottom_btns->addStretch();
    bottom_btns->addWidget( btn_cancel );
    bottom_btns->addWidget( btn_save );

    main_layout->addLayout( bottom_btns );
}

void AddSetView::onAddCardClicked() {
    auto* overlay = new AddCardOverlay( this );

    connect( overlay, &AddCardOverlay::cardSaved, this, [this, overlay]( const DraftCard& card ) {
        this->onCardSaved( card );
        overlay_container_->clearContent();
        overlay->deleteLater();
    } );

    connect( overlay, &AddCardOverlay::cancelled, this, [this, overlay]() {
        overlay_container_->clearContent();
        overlay->deleteLater();
    } );

    overlay_container_->setContent( overlay );
}

void AddSetView::onCardSaved( const DraftCard& card ) {
    draft_cards_.push_back( card );

    QString label;

    if ( holds_alternative<TextContent>( card.question ) ) {
        label += "[T] " + QString::fromStdString( get<TextContent>( card.question ).text );
    } else if ( holds_alternative<ImageContent>( card.question ) ) {
        label += "[IMG] " + QString::fromStdString( get<ImageContent>( card.question ).image_path );
    } else if ( holds_alternative<SoundContent>( card.question ) ) {
        label += "[SND] " + QString::fromStdString( get<SoundContent>( card.question ).sound_path );
    }

    label += " -> " + QString::fromStdString( card.correct_answer );

    if ( card.answer_type == AnswerType::TEXT_CHOICE ) label += tr( " (Quiz)" );
    if ( card.answer_type == AnswerType::INPUT ) label += tr( " (Input)" );

    QListWidgetItem* item = new QListWidgetItem( label );
    preview_list_->addItem( item );

    preview_list_->scrollToBottom();
}

void AddSetView::saveSet() {
    QString name = name_input_->text().trimmed();

    if ( name.isEmpty() ) {
        QMessageBox::warning( this, tr( "Error" ), tr( "Enter set name!" ) );
        return;
    }
    if ( draft_cards_.empty() ) {
        QMessageBox::warning( this, tr( "Error" ), tr( "Set must contain at least one card!" ) );
        return;
    }

    if ( db_.createSet( name.toStdString(), draft_cards_ ) ) {
        name_input_->clear();
        preview_list_->clear();
        draft_cards_.clear();

        emit setCreated();
    } else {
        QMessageBox::critical( this, tr( "Error" ), tr( "Could not save set to database." ) );
    }
}