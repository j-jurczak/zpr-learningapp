/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - source file.
 */
#include "SetView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>
#include <QMenu>
#include <QAction>

#include "../overlays/OverlayContainer.h"
#include "../overlays/AddCardOverlay.h"
#include "../overlays/CardPreviewOverlay.h"

using namespace std;

SetView::SetView( int set_id, DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), set_id_( set_id ), db_( db ) {
    overlay_container_ = make_unique<OverlayContainer>( this );

    setupUi();
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
    main_layout->setContentsMargins( 20, 20, 20, 20 );
    main_layout->setSpacing( 15 );

    QHBoxLayout* header_layout = new QHBoxLayout();

    QPushButton* btn_back = new QPushButton( "← " + tr( "Back" ), this );
    btn_back->setCursor( Qt::PointingHandCursor );
    connect( btn_back, &QPushButton::clicked, this, &SetView::backToSetsClicked );

    title_label_ = new QLabel( tr( "Loading..." ), this );
    title_label_->setStyleSheet( "font-size: 24px; font-weight: bold; margin-left: 10px;" );

    header_layout->addWidget( btn_back );
    header_layout->addWidget( title_label_ );
    header_layout->addStretch();

    QPushButton* btn_delete_set = new QPushButton( "🗑 " + tr( "Delete Set" ), this );
    btn_delete_set->setCursor( Qt::PointingHandCursor );
    btn_delete_set->setStyleSheet(
        "background-color: #c62828; color: white; border: none; padding: 5px 10px; border-radius: "
        "4px; margin-right: 10px;" );

    connect( btn_delete_set, &QPushButton::clicked, this, [this]() {
        auto reply = QMessageBox::question(
            this, tr( "Delete Set" ),
            tr( "Are you sure you want to delete this set and all its cards?" ),
            QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::Yes ) {
            if ( db_.deleteSet( set_id_ ) ) {
                emit backToSetsClicked();
            } else {
                QMessageBox::critical( this, tr( "Error" ), tr( "Could not delete set." ) );
            }
        }
    } );

    header_layout->addWidget( btn_delete_set );

    QPushButton* btn_add = new QPushButton( "+ " + tr( "Add Question" ), this );
    btn_add->setCursor( Qt::PointingHandCursor );

    connect( btn_add, &QPushButton::clicked, this, [this]() {
        add_overlay_ = make_unique<AddCardOverlay>();

        connect( add_overlay_.get(), &AddCardOverlay::cancelled, overlay_container_.get(),
                 &OverlayContainer::clearContent );

        connect( add_overlay_.get(), &AddCardOverlay::cardSaved, this,
                 [this]( const DraftCard& c ) {
                     if ( db_.addCardToSet( set_id_, c ) ) {
                         overlay_container_->clearContent();
                         loadData();
                     } else {
                         QMessageBox::critical( this, tr( "Error" ), tr( "Could not save card." ) );
                     }
                 } );

        overlay_container_->setContent( add_overlay_.get() );
    } );

    QPushButton* btn_learn = new QPushButton( "▶ " + tr( "Learn" ), this );
    btn_learn->setCursor( Qt::PointingHandCursor );
    btn_learn->setStyleSheet(
        "QPushButton { background-color: #0078d4; color: white; font-weight: bold; padding: 5px "
        "15px; border-radius: 4px; }"
        "QPushButton::menu-indicator { subcontrol-origin: padding; subcontrol-position: center "
        "right; right: 5px; }" );

    QMenu* learn_menu = new QMenu( btn_learn );
    learn_menu->setStyleSheet(
        "QMenu { background-color: #2d2d30; color: white; border: 1px solid #3e3e42; } "
        "QMenu::item:selected { background-color: #0078d4; }" );

    QAction* act_sm2 = new QAction( tr( "Smart Repetition (SM-2)" ), learn_menu );
    connect( act_sm2, &QAction::triggered, this,
             [this]() { emit learnClicked( set_id_, LearningMode::SpacedRepetition ); } );

    QAction* act_random = new QAction( tr( "Quick Review (Random)" ), learn_menu );
    connect( act_random, &QAction::triggered, this,
             [this]() { emit learnClicked( set_id_, LearningMode::Random ); } );

    learn_menu->addSeparator();

    QAction* act_reset = new QAction( tr( "Reset set progress" ), learn_menu );

    connect( act_reset, &QAction::triggered, this, [this]() {
        auto reply =
            QMessageBox::question( this, tr( "Reset progress" ),
                                   tr( "Are you sure you want to reset learning progress for this "
                                       "set? Cards will be treated as new." ),
                                   QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::Yes ) {
            if ( db_.resetSetProgress( set_id_ ) ) {
                QMessageBox::information( this, tr( "Success" ), tr( "Progress reset." ) );
            } else {
                QMessageBox::critical( this, tr( "Error" ), tr( "Could not reset progress." ) );
            }
        }
    } );

    learn_menu->addAction( act_sm2 );
    learn_menu->addAction( act_random );
    learn_menu->addSeparator();
    learn_menu->addAction( act_reset );
    learn_menu->addSeparator();
    learn_menu->addAction( act_reset );

    btn_learn->setMenu( learn_menu );

    header_layout->addWidget( btn_add );
    header_layout->addWidget( btn_learn );

    main_layout->addLayout( header_layout );

    cards_list_ = new QListWidget( this );
    main_layout->addWidget( cards_list_ );
}

void SetView::loadData() {
    auto set_opt = db_.getSet( set_id_ );
    if ( set_opt.has_value() ) {
        title_label_->setText( QString::fromStdString( set_opt->name ) );
    } else {
        title_label_->setText( tr( "Unknown set" ) );
    }

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
        int card_id = card.getId();

        QPushButton* btn_content = new QPushButton( question, row_widget );
        btn_content->setCursor( Qt::PointingHandCursor );
        btn_content->setStyleSheet(
            "text-align: left; border: none; padding-left: 10px; background: transparent;" );
        btn_content->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

        connect( btn_content, &QPushButton::clicked, this, [this, card]() {
            current_preview_ = make_unique<CardPreviewOverlay>( card );
            auto* ptr = static_cast<CardPreviewOverlay*>( current_preview_.get() );
            connect( ptr, &CardPreviewOverlay::closeClicked, overlay_container_.get(),
                     &OverlayContainer::clearContent );

            overlay_container_->setContent( current_preview_.get() );
        } );

        QPushButton* btn_delete = new QPushButton( "✕", row_widget );
        btn_delete->setFixedSize( 30, 30 );
        btn_delete->setCursor( Qt::PointingHandCursor );
        btn_delete->setStyleSheet( "color: #888; border: none; font-weight: bold;" );

        connect( btn_delete, &QPushButton::clicked, this, [this, card_id]() {
            auto reply = QMessageBox::question( this, tr( "Delete" ), tr( "Delete this question?" ),
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