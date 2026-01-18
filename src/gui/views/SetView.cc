/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - source file.
 */
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QFrame>
#include <algorithm>

#include "SetView.h"
#include "../overlays/OverlayContainer.h"
#include "../overlays/AddCardOverlay.h"
#include "../overlays/CardPreviewOverlay.h"
#include "../../core/utils/StyleLoader.h"

using namespace std;

SetView::SetView( int set_id, DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), set_id_( set_id ), db_( db ) {
    overlay_container_ = make_unique<OverlayContainer>( this );

    setupUi();
    loadData();

    StyleLoader::attach( this, "views/SetView.qss" );
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
    title_label_->setObjectName( "title" );

    header_layout->addWidget( btn_back );
    header_layout->addWidget( title_label_ );
    header_layout->addStretch();

    QPushButton* btn_delete_set = new QPushButton( "🗑 " + tr( "Delete Set" ), this );
    btn_delete_set->setCursor( Qt::PointingHandCursor );
    btn_delete_set->setProperty( "type", "danger" );

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
    btn_learn->setProperty( "type", "primary" );

    QMenu* learn_menu = new QMenu( btn_learn );

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
                loadData();
            } else {
                QMessageBox::critical( this, tr( "Error" ), tr( "Could not reset progress." ) );
            }
        }
    } );

    learn_menu->addAction( act_sm2 );
    learn_menu->addAction( act_random );
    learn_menu->addSeparator();
    learn_menu->addAction( act_reset );

    btn_learn->setMenu( learn_menu );

    header_layout->addWidget( btn_add );
    header_layout->addWidget( btn_learn );

    main_layout->addLayout( header_layout );

    QFrame* stats_container = new QFrame( this );
    stats_container->setFixedHeight( 180 );
    stats_container->setObjectName( "statsContainer" );

    QHBoxLayout* stats_layout = new QHBoxLayout( stats_container );
    stats_layout->setContentsMargins( 40, 20, 40, 20 );
    stats_layout->setSpacing( 20 );

    QVBoxLayout* text_layout = new QVBoxLayout();
    text_layout->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    text_layout->setSpacing( 10 );

    QLabel* lbl_total = new QLabel( "Total Cards: 0", stats_container );
    lbl_total->setObjectName( "statsTotal" );

    QLabel* lbl_mastered = new QLabel( "Mastered: 0%", stats_container );
    lbl_mastered->setObjectName( "statsMastered" );

    QLabel* lbl_learning = new QLabel( "Learning: 0%", stats_container );
    lbl_learning->setObjectName( "statsLearning" );

    QLabel* lbl_new = new QLabel( "New: 100%", stats_container );
    lbl_new->setObjectName( "statsNew" );

    text_layout->addWidget( lbl_total );
    text_layout->addWidget( lbl_mastered );
    text_layout->addWidget( lbl_learning );
    text_layout->addWidget( lbl_new );

    QFrame* chart_frame = new QFrame( stats_container );
    chart_frame->setObjectName( "statsChart" );
    chart_frame->setFixedSize( 120, 120 );
    stats_layout->addLayout( text_layout );
    stats_layout->addStretch();
    stats_layout->addWidget( chart_frame );

    main_layout->addWidget( stats_container );

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

    SetStats stats = db_.getSetStatistics( set_id_ );

    int total = stats.total;
    int new_cards = stats.new_cards;
    int learning = stats.learning;
    int mastered = stats.mastered;

    QLabel* lbl_total = this->findChild<QLabel*>( "statsTotal" );
    QLabel* lbl_mastered = this->findChild<QLabel*>( "statsMastered" );
    QLabel* lbl_learning = this->findChild<QLabel*>( "statsLearning" );
    QLabel* lbl_new = this->findChild<QLabel*>( "statsNew" );
    QFrame* chart = this->findChild<QFrame*>( "statsChart" );

    if ( lbl_total ) lbl_total->setText( tr( "Total Cards: " ) + QString::number( total ) );

    double p_mastered = total > 0 ? (double)mastered / total : 0.0;
    double p_learning = total > 0 ? (double)learning / total : 0.0;
    double p_new = total > 0 ? (double)new_cards / total : 1.0;

    if ( lbl_mastered )
        lbl_mastered->setText( tr( "Mastered: " ) + QString::number( p_mastered * 100, 'f', 0 ) +
                               "%" );
    if ( lbl_learning )
        lbl_learning->setText( tr( "Learning: " ) + QString::number( p_learning * 100, 'f', 0 ) +
                               "%" );
    if ( lbl_new ) lbl_new->setText( tr( "New: " ) + QString::number( p_new * 100, 'f', 0 ) + "%" );

    if ( chart ) {
        QString style;
        if ( total == 0 || p_new > 0.999 ) {
            style = "border-radius: 60px; background-color: #444;";
        } else if ( p_mastered > 0.999 ) {
            style = "border-radius: 60px; background-color: #388e3c;";
        } else if ( p_learning > 0.999 ) {
            style = "border-radius: 60px; background-color: #f57c00;";
        } else {
            double end_green = p_mastered;
            double end_orange = p_mastered + p_learning;
            double eps = 0.0001;

            style = QString(
                        "border-radius: 60px; "
                        "background-color: qconicalgradient(cx:0.5, cy:0.5, angle:90, "
                        "stop:0 #388e3c, "
                        "stop:%1 #388e3c, "
                        "stop:%2 #f57c00, "
                        "stop:%3 #f57c00, "
                        "stop:%4 #444, "
                        "stop:1 #444)" )
                        .arg( end_green )
                        .arg( min( 1.0, end_green + eps ) )
                        .arg( end_orange )
                        .arg( min( 1.0, end_orange + eps ) );
        }

        chart->setStyleSheet( style );
    }

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
        btn_content->setObjectName( "cardContent" );
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
        btn_delete->setObjectName( "deleteCardBtn" );

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