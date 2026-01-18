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
#include "../overlays/CardPreviewOverlay.h"
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
    preview_list_->setSelectionMode( QAbstractItemView::NoSelection );
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

    QString label_text;

    if ( holds_alternative<TextContent>( card.question ) ) {
        label_text += "[T] " + QString::fromStdString( get<TextContent>( card.question ).text );
    } else if ( holds_alternative<ImageContent>( card.question ) ) {
        label_text +=
            "[IMG] " + QString::fromStdString( get<ImageContent>( card.question ).image_path );
    } else if ( holds_alternative<SoundContent>( card.question ) ) {
        label_text +=
            "[SND] " + QString::fromStdString( get<SoundContent>( card.question ).sound_path );
    }

    label_text += " -> " + QString::fromStdString( card.correct_answer );

    if ( card.answer_type == AnswerType::TEXT_CHOICE ) label_text += tr( " (Quiz)" );
    if ( card.answer_type == AnswerType::INPUT ) label_text += tr( " (Input)" );

    QListWidgetItem* item = new QListWidgetItem( preview_list_ );
    item->setSizeHint( QSize( 0, 50 ) );

    QWidget* row_widget = new QWidget();
    QHBoxLayout* row_layout = new QHBoxLayout( row_widget );
    row_layout->setContentsMargins( 0, 0, 10, 0 );
    row_layout->setSpacing( 10 );

    QPushButton* btn_content = new QPushButton( label_text, row_widget );
    btn_content->setCursor( Qt::PointingHandCursor );
    btn_content->setObjectName( "cardContent" );
    btn_content->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    DraftCard card_copy = card;

    connect( btn_content, &QPushButton::clicked, this, [this, card_copy]() {
        CardData data;
        data.id = 0;
        data.set_id = 0;
        data.question = card_copy.question;
        data.correct_answer = card_copy.correct_answer;
        data.wrong_answers = card_copy.wrong_answers;
        data.answer_type = card_copy.answer_type;

        Card previewCard( data );

        auto* preview = new CardPreviewOverlay( previewCard, this );
        connect( preview, &CardPreviewOverlay::closeClicked, overlay_container_.get(), &OverlayContainer::clearContent );
        overlay_container_->setContent( preview );
    });

    QPushButton* btn_delete = new QPushButton( row_widget );
    btn_delete->setIcon( style()->standardIcon( QStyle::SP_TrashIcon ) );
    btn_delete->setFixedSize( 30, 30 );
    btn_delete->setCursor( Qt::PointingHandCursor );
    btn_delete->setObjectName( "deleteCardBtn" );

    connect( btn_delete, &QPushButton::clicked, this, [this, item]() {
        int row = preview_list_->row( item );
        if ( row >= 0 && row < (int)draft_cards_.size() ) {
            draft_cards_.erase( draft_cards_.begin() + row );
            delete preview_list_->takeItem( row );
        }
    } );

    row_layout->addWidget( btn_content );
    row_layout->addWidget( btn_delete );

    preview_list_->setItemWidget( item, row_widget );
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