/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - source file.
 */
#include "AddSetView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QScrollArea>
#include <QResizeEvent>

#include "../overlays/CardPreviewOverlay.h"

AddSetView::AddSetView( DatabaseManager& db, QWidget* parent ) : QWidget( parent ), db_( db ) {
    setupUi();

    overlay_ = new CardPreviewOverlay( this );
    setupStyles();
}

void AddSetView::resizeEvent( QResizeEvent* event ) {
    QWidget::resizeEvent( event );
    if ( overlay_ ) {
        overlay_->resize( this->size() );
    }
}

void AddSetView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 0, 0, 0, 0 );

    QScrollArea* scroll_area = new QScrollArea( this );
    scroll_area->setWidgetResizable( true );
    scroll_area->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QWidget* scroll_content = new QWidget();
    scroll_content->setObjectName( "scroll_content" );

    QVBoxLayout* content_layout = new QVBoxLayout( scroll_content );
    content_layout->setContentsMargins( 30, 30, 30, 30 );
    content_layout->setSpacing( 20 );

    QLabel* header = new QLabel( "Tworzenie nowego zestawu", scroll_content );
    header->setObjectName( "header" );
    content_layout->addWidget( header );

    content_layout->addWidget( new QLabel( "Nazwa zestawu:", scroll_content ) );
    name_input_ = new QLineEdit( scroll_content );
    name_input_->setPlaceholderText( "np. Język Niemiecki - Zwierzęta" );
    content_layout->addWidget( name_input_ );

    content_layout->addWidget( new QLabel( "Dodane karty:", scroll_content ) );
    preview_list_ = new QListWidget( scroll_content );
    preview_list_->setObjectName( "preview_list" );
    preview_list_->setMinimumHeight( 200 );
    preview_list_->setMaximumHeight( 300 );
    content_layout->addWidget( preview_list_ );

    QLabel* form_header = new QLabel( "Dodaj nowe pytanie", scroll_content );
    form_header->setStyleSheet(
        "font-size: 18px; font-weight: bold; margin-top: 20px; margin-bottom: 10px;" );
    content_layout->addWidget( form_header );

    content_layout->addWidget( new QLabel( "Treść pytania:", scroll_content ) );
    question_input_ = new QLineEdit( scroll_content );
    question_input_->setPlaceholderText( "Wpisz pytanie..." );
    content_layout->addWidget( question_input_ );

    content_layout->addWidget( new QLabel( "Poprawna odpowiedź:", scroll_content ) );
    correct_input_ = new QLineEdit( scroll_content );
    correct_input_->setPlaceholderText( "Wpisz poprawną odpowiedź..." );
    correct_input_->setStyleSheet( "border: 1px solid #2e7d32;" );
    content_layout->addWidget( correct_input_ );

    QHBoxLayout* wrong_header_layout = new QHBoxLayout();
    wrong_header_layout->addWidget( new QLabel( "Błędne odpowiedzi (max 3):", scroll_content ) );

    btn_add_wrong_ = new QPushButton( "+", scroll_content );
    btn_add_wrong_->setObjectName( "btn_add_field" );
    btn_add_wrong_->setFixedSize( 30, 30 );

    wrong_header_layout->addWidget( btn_add_wrong_ );
    wrong_header_layout->addStretch();

    content_layout->addLayout( wrong_header_layout );

    wrong_answers_layout_ = new QVBoxLayout();
    wrong_answers_layout_->setSpacing( 10 );
    content_layout->addLayout( wrong_answers_layout_ );

    addWrongAnswerField();

    QPushButton* btn_add_card = new QPushButton( "Zatwierdź kartę", scroll_content );
    btn_add_card->setObjectName( "btn_add" );

    content_layout->addSpacing( 10 );
    content_layout->addWidget( btn_add_card );

    content_layout->addStretch();

    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* btn_cancel = new QPushButton( "Anuluj", scroll_content );
    btn_cancel->setObjectName( "btn_cancel" );

    QPushButton* btn_save = new QPushButton( "Zapisz Cały Zestaw", scroll_content );
    btn_save->setObjectName( "btn_save" );

    btn_layout->addWidget( btn_cancel );
    btn_layout->addStretch();
    btn_layout->addWidget( btn_save );
    content_layout->addLayout( btn_layout );

    scroll_area->setWidget( scroll_content );
    main_layout->addWidget( scroll_area );

    connect( btn_add_wrong_, &QPushButton::clicked, this, &AddSetView::addWrongAnswerField );
    connect( btn_add_card, &QPushButton::clicked, this, &AddSetView::addCardToDraft );
    connect( btn_save, &QPushButton::clicked, this, &AddSetView::saveSet );
    connect( btn_cancel, &QPushButton::clicked, this, &AddSetView::creationCancelledClicked );
}

void AddSetView::setupStyles() {
    QFile file( ":/resources/AddSetView.qss" );
    if ( file.open( QFile::ReadOnly ) ) {
        this->setStyleSheet( QString::fromLatin1( file.readAll() ) );
        file.close();
    }
}

void AddSetView::addWrongAnswerField() {
    if ( wrong_inputs_.size() >= 3 ) {
        return;
    }

    QLineEdit* wrong_input = new QLineEdit();
    wrong_input->setPlaceholderText( "Błędna odpowiedź..." );

    wrong_answers_layout_->addWidget( wrong_input );
    wrong_inputs_.push_back( wrong_input );
    wrong_input->setFocus();

    if ( wrong_inputs_.size() >= 3 ) {
        btn_add_wrong_->setEnabled( false );
        btn_add_wrong_->setToolTip( "Osiągnięto limit 3 błędnych odpowiedzi" );
    }
}

void AddSetView::addCardToDraft() {
    QString q = question_input_->text().trimmed();
    QString correct = correct_input_->text().trimmed();

    if ( q.isEmpty() || correct.isEmpty() ) {
        QMessageBox::warning( this, "Błąd", "Pytanie i poprawna odpowiedź są wymagane!" );
        return;
    }

    std::vector<std::string> wrong_answers_vec;
    for ( QLineEdit* input : wrong_inputs_ ) {
        QString wrong = input->text().trimmed();
        if ( !wrong.isEmpty() ) {
            wrong_answers_vec.push_back( wrong.toStdString() );
        }
    }

    DraftCard draft;
    draft.question = q.toStdString();
    draft.correct_answer = correct.toStdString();
    draft.wrong_answers = wrong_answers_vec;
    draft_cards_.push_back( draft );

    QListWidgetItem* item = new QListWidgetItem( preview_list_ );
    item->setSizeHint( QSize( 0, 50 ) );

    QWidget* row_widget = new QWidget();
    QHBoxLayout* row_layout = new QHBoxLayout( row_widget );
    row_layout->setContentsMargins( 0, 0, 0, 0 );
    row_layout->setSpacing( 0 );

    QString label_text = q + ( wrong_answers_vec.empty() ? "" : " [Test]" );

    QPushButton* btn_content = new QPushButton( label_text, row_widget );
    btn_content->setObjectName( "btn_preview_item" );
    btn_content->setCursor( Qt::PointingHandCursor );
    btn_content->setToolTip( "Kliknij, aby zobaczyć podgląd" );
    btn_content->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

    QPushButton* btn_delete = new QPushButton( "X", row_widget );
    btn_delete->setObjectName( "btn_delete_item" );
    btn_delete->setFixedSize( 40, 50 );
    btn_delete->setCursor( Qt::PointingHandCursor );

    row_layout->addWidget( btn_content );
    row_layout->addWidget( btn_delete );

    preview_list_->setItemWidget( item, row_widget );

    connect( btn_content, &QPushButton::clicked, this, [this, q, correct, wrong_answers_vec]() {
        overlay_->showCard( q.toStdString(), correct.toStdString(), wrong_answers_vec );
    } );

    connect( btn_delete, &QPushButton::clicked, this, [this, item]() {
        int row = preview_list_->row( item );
        if ( row < 0 || row >= static_cast<int>( draft_cards_.size() ) ) return;
        draft_cards_.erase( draft_cards_.begin() + row );
        delete item;
    } );

    resetCardForm();
}

void AddSetView::resetCardForm() {
    question_input_->clear();
    correct_input_->clear();

    qDeleteAll( wrong_inputs_ );
    wrong_inputs_.clear();

    addWrongAnswerField();

    btn_add_wrong_->setEnabled( true );
    btn_add_wrong_->setToolTip( "Dodaj kolejne pole na błędną odpowiedź" );

    question_input_->setFocus();
}

void AddSetView::saveSet() {
    QString name = name_input_->text().trimmed();

    if ( name.isEmpty() ) {
        QMessageBox::warning( this, "Błąd", "Podaj nazwę zestawu!" );
        return;
    }
    if ( draft_cards_.empty() ) {
        QMessageBox::warning( this, "Błąd", "Zestaw musi mieć przynajmniej jedną kartę!" );
        return;
    }

    if ( db_.createSet( name.toStdString(), draft_cards_ ) ) {
        name_input_->clear();
        preview_list_->clear();
        draft_cards_.clear();
        resetCardForm();
        emit setCreated();
    } else {
        QMessageBox::critical( this, "Błąd", "Nie udało się zapisać zestawu w bazie." );
    }
}

void AddSetView::creationCancelledClicked() { emit creationCancelled(); }