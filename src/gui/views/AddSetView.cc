/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - source file.
 */
#include "AddSetView.h"
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFile>
#include <QDebug>

#include "../overlays/OverlayContainer.h"
#include "../overlays/CardPreviewOverlay.h"

AddSetView::AddSetView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_( db ) {
    overlay_container_ = std::make_unique<OverlayContainer>( this );
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
    } else {
        qDebug() << "BŁĄD: Nie można załadować pliku stylu :/resources/AddSetView.qss";
    }
}

void AddSetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 30, 30, 30 );
    main_layout->setSpacing( 20 );

    QLabel* header = new QLabel( "Stwórz Nowy Zestaw", this );
    header->setObjectName( "header" );
    main_layout->addWidget( header );

    name_input_ = new QLineEdit( this );
    name_input_->setPlaceholderText( "Nazwa zestawu (np. Angielski A1)" );
    main_layout->addWidget( name_input_ );

    main_layout->addWidget( new QLabel( "Dodane karty:", this ) );
    preview_list_ = new QListWidget( this );
    preview_list_->setMinimumHeight( 150 );
    main_layout->addWidget( preview_list_ );

    QFrame* card_form_frame = new QFrame( this );
    card_form_frame->setObjectName( "form_frame" );
    QVBoxLayout* form_layout = new QVBoxLayout( card_form_frame );
    form_layout->setContentsMargins( 20, 20, 20, 20 );
    form_layout->setSpacing( 15 );

    form_layout->addWidget( new QLabel( "Pytanie:", card_form_frame ) );
    question_input_ = new QLineEdit( card_form_frame );
    form_layout->addWidget( question_input_ );

    form_layout->addWidget( new QLabel( "Poprawna odpowiedź:", card_form_frame ) );
    correct_input_ = new QLineEdit( card_form_frame );
    form_layout->addWidget( correct_input_ );

    QHBoxLayout* wrong_header = new QHBoxLayout();
    wrong_header->addWidget( new QLabel( "Błędne odpowiedzi (opcjonalne):", card_form_frame ) );

    btn_add_wrong_ = new QPushButton( "+", card_form_frame );
    btn_add_wrong_->setObjectName( "btn_add_wrong" );
    btn_add_wrong_->setFixedSize( 30, 30 );
    btn_add_wrong_->setCursor( Qt::PointingHandCursor );
    connect( btn_add_wrong_, &QPushButton::clicked, this, &AddSetView::addWrongAnswerField );

    wrong_header->addWidget( btn_add_wrong_ );
    form_layout->addLayout( wrong_header );

    wrong_answers_layout_ = new QVBoxLayout();
    form_layout->addLayout( wrong_answers_layout_ );

    addWrongAnswerField();

    QPushButton* btn_add_card = new QPushButton( "Dodaj kartę", card_form_frame );
    btn_add_card->setObjectName( "btn_add_card" );
    btn_add_card->setCursor( Qt::PointingHandCursor );
    connect( btn_add_card, &QPushButton::clicked, this, &AddSetView::addCardToDraft );

    form_layout->addWidget( btn_add_card );
    main_layout->addWidget( card_form_frame );

    QHBoxLayout* bottom_btns = new QHBoxLayout();
    bottom_btns->addStretch();

    QPushButton* btn_cancel = new QPushButton( "Anuluj", this );
    btn_cancel->setObjectName( "btn_cancel" );
    btn_cancel->setCursor( Qt::PointingHandCursor );
    connect( btn_cancel, &QPushButton::clicked, this, &AddSetView::creationCancelledClicked );

    QPushButton* btn_save = new QPushButton( "Stwórz Zestaw", this );
    btn_save->setObjectName( "btn_save" );
    btn_save->setCursor( Qt::PointingHandCursor );
    connect( btn_save, &QPushButton::clicked, this, &AddSetView::saveSet );

    bottom_btns->addWidget( btn_cancel );
    bottom_btns->addWidget( btn_save );

    main_layout->addLayout( bottom_btns );
}

void AddSetView::addWrongAnswerField() {
    if ( wrong_inputs_.size() >= 3 ) return;

    QLineEdit* input = new QLineEdit( this );
    input->setPlaceholderText( "Błędna odpowiedź..." );
    wrong_answers_layout_->addWidget( input );
    wrong_inputs_.push_back( input );

    if ( wrong_inputs_.size() >= 3 ) btn_add_wrong_->setEnabled( false );
}

void AddSetView::resetCardForm() {
    question_input_->clear();
    correct_input_->clear();

    QLayoutItem* item;
    while ( ( item = wrong_answers_layout_->takeAt( 0 ) ) != nullptr ) {
        delete item->widget();
        delete item;
    }
    wrong_inputs_.clear();
    btn_add_wrong_->setEnabled( true );
    addWrongAnswerField();
}

void AddSetView::addCardToDraft() {
    QString q = question_input_->text().trimmed();
    QString correct = correct_input_->text().trimmed();

    if ( q.isEmpty() || correct.isEmpty() ) {
        QMessageBox::warning( this, "Błąd", "Pytanie i odpowiedź są wymagane!" );
        return;
    }

    std::vector<std::string> wrong_answers_vec;
    for ( QLineEdit* input : wrong_inputs_ ) {
        if ( !input->text().trimmed().isEmpty() ) {
            wrong_answers_vec.push_back( input->text().trimmed().toStdString() );
        }
    }

    draft_cards_.push_back( { q.toStdString(), correct.toStdString(), wrong_answers_vec } );

    QListWidgetItem* item = new QListWidgetItem( preview_list_ );
    item->setSizeHint( QSize( 0, 50 ) );

    QWidget* row_widget = new QWidget();
    QHBoxLayout* row_layout = new QHBoxLayout( row_widget );
    row_layout->setContentsMargins( 10, 0, 10, 0 );

    QLabel* lbl_text = new QLabel( q, row_widget );
    lbl_text->setStyleSheet( "color: white; font-size: 14px;" );
    row_layout->addWidget( lbl_text );
    row_layout->addStretch();

    QPushButton* btn_preview = new QPushButton( "Podgląd", row_widget );
    btn_preview->setObjectName( "btn_preview" );
    btn_preview->setCursor( Qt::PointingHandCursor );

    connect( btn_preview, &QPushButton::clicked, this, [this, q, correct, wrong_answers_vec]() {
        current_preview_ = std::make_unique<CardPreviewOverlay>(
            q.toStdString(), correct.toStdString(), wrong_answers_vec );
        auto* preview_ptr = static_cast<CardPreviewOverlay*>( current_preview_.get() );
        connect( preview_ptr, &CardPreviewOverlay::closeClicked, overlay_container_.get(),
                 &OverlayContainer::clearContent );
        overlay_container_->setContent( current_preview_.get() );
    } );

    row_layout->addWidget( btn_preview );
    preview_list_->setItemWidget( item, row_widget );

    resetCardForm();
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
        resetCardForm();

        emit setCreated();
    } else {
        QMessageBox::critical( this, "Błąd", "Nie udało się zapisać zestawu w bazie danych." );
    }
}

void AddSetView::creationCancelledClicked() { emit creationCancelled(); }