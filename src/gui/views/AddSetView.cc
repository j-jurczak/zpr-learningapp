/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - source file.
 */
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QFile>
#include <QDebug>
#include <variant>

#include "AddSetView.h"
#include "../overlays/AddCardOverlay.h"
#include "../overlays/OverlayContainer.h"

using namespace std;

AddSetView::AddSetView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_( db ) {
    overlay_container_ = make_unique<OverlayContainer>( this );
    setupUi();
    setupStyles();
}

void AddSetView::resizeEvent( QResizeEvent* event ) {
    QWidget::resizeEvent( event );
    if ( overlay_container_ ) {
        overlay_container_->resize( this->size() );
    }
}

void AddSetView::setupStyles() {
    QFile file( ":/resources/AddSetView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }
}

void AddSetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 30, 30, 30 );
    main_layout->setSpacing( 20 );

    QLabel* header = new QLabel( "Stwórz Nowy Zestaw", this );
    header->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: white; margin-bottom: 10px;" );
    main_layout->addWidget( header );

    main_layout->addWidget( new QLabel( "Nazwa zestawu:", this ) );
    name_input_ = new QLineEdit( this );
    name_input_->setPlaceholderText( "Np. Historia Polski - Daty" );
    name_input_->setStyleSheet(
        "padding: 12px; font-size: 16px; background: #252526; color: white; border: 1px solid "
        "#3e3e42; border-radius: 5px;" );
    main_layout->addWidget( name_input_ );

    main_layout->addWidget( new QLabel( "Lista kart w zestawie:", this ) );
    preview_list_ = new QListWidget( this );
    preview_list_->setStyleSheet(
        "QListWidget { background: #1e1e1e; border: 1px solid #3e3e42; border-radius: 5px; "
        "font-size: 14px; } QListWidget::item { padding: 10px; border-bottom: 1px solid #333; }" );
    main_layout->addWidget( preview_list_ );

    btn_open_creator_ = new QPushButton( "+ Dodaj nową kartę", this );
    btn_open_creator_->setCursor( Qt::PointingHandCursor );
    btn_open_creator_->setMinimumHeight( 50 );
    btn_open_creator_->setStyleSheet(
        "QPushButton { background-color: #0078d4; color: white; font-weight: bold; font-size: "
        "16px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #006cbd; }" );
    connect( btn_open_creator_, &QPushButton::clicked, this, &AddSetView::onAddCardClicked );
    main_layout->addWidget( btn_open_creator_ );

    main_layout->addStretch();

    QHBoxLayout* bottom_btns = new QHBoxLayout();

    QPushButton* btn_cancel = new QPushButton( "Anuluj", this );
    btn_cancel->setCursor( Qt::PointingHandCursor );
    btn_cancel->setStyleSheet(
        "background: transparent; border: 1px solid #555; color: #ccc; padding: 10px 20px; "
        "border-radius: 5px;" );
    connect( btn_cancel, &QPushButton::clicked, this, [this]() { emit creationCancelled(); } );

    QPushButton* btn_save = new QPushButton( "Stwórz Zestaw", this );
    btn_save->setCursor( Qt::PointingHandCursor );
    btn_save->setStyleSheet(
        "background-color: #2e7d32; color: white; font-weight: bold; padding: 10px 20px; "
        "border-radius: 5px;" );
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

    if ( std::holds_alternative<TextContent>( card.question ) ) {
        label += "[T] " + QString::fromStdString( std::get<TextContent>( card.question ).text );
    } else if ( std::holds_alternative<ImageContent>( card.question ) ) {
        label +=
            "[IMG] " + QString::fromStdString( std::get<ImageContent>( card.question ).image_path );
    } else if ( std::holds_alternative<SoundContent>( card.question ) ) {
        label +=
            "[SND] " + QString::fromStdString( std::get<SoundContent>( card.question ).sound_path );
    }

    label += " -> " + QString::fromStdString( card.correct_answer );

    if ( card.answer_type == AnswerType::TEXT_CHOICE ) label += " (Quiz)";
    if ( card.answer_type == AnswerType::INPUT ) label += " (Wpisywanie)";

    QListWidgetItem* item = new QListWidgetItem( label );
    preview_list_->addItem( item );

    preview_list_->scrollToBottom();
}

void AddSetView::saveSet() {
    QString name = name_input_->text().trimmed();

    if ( name.isEmpty() ) {
        QMessageBox::warning( this, "Błąd", "Podaj nazwę zestawu!" );
        return;
    }
    if ( draft_cards_.empty() ) {
        QMessageBox::warning( this, "Błąd", "Zestaw musi zawierać przynajmniej jedną kartę!" );
        return;
    }

    if ( db_.createSet( name.toStdString(), draft_cards_ ) ) {
        name_input_->clear();
        preview_list_->clear();
        draft_cards_.clear();

        emit setCreated();
    } else {
        QMessageBox::critical( this, "Błąd", "Nie udało się zapisać zestawu w bazie danych." );
    }
}