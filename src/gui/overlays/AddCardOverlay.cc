/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of the single card addition logic within an overlay.
 */
#include "AddCardOverlay.h"
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QHBoxLayout>

AddCardOverlay::AddCardOverlay( QWidget* parent ) : QWidget( parent ) { setupUi(); }

void AddCardOverlay::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 0, 0, 0, 0 );

    QFrame* form_frame = new QFrame( this );
    form_frame->setStyleSheet(
        "QFrame { background-color: #252526; border: 1px solid #454545; border-radius: 8px; "
        "min-width: 400px; }"
        "QLabel { color: #f0f0f0; font-weight: bold; font-size: 14px; }"
        "QLineEdit { padding: 5px; background-color: #333; color: white; border: 1px solid #555; "
        "border-radius: 4px; }"
        "QLineEdit:focus { border: 1px solid #007acc; }" );

    QVBoxLayout* form_layout = new QVBoxLayout( form_frame );
    form_layout->setContentsMargins( 30, 30, 30, 30 );
    form_layout->setSpacing( 15 );

    QLabel* header = new QLabel( "DODAJ NOWĄ KARTĘ", form_frame );
    header->setAlignment( Qt::AlignCenter );
    header->setStyleSheet( "font-size: 18px; margin-bottom: 10px; color: #007acc; border: none;" );
    form_layout->addWidget( header );

    form_layout->addWidget( new QLabel( "Pytanie:", form_frame ) );
    question_input_ = new QLineEdit( form_frame );
    form_layout->addWidget( question_input_ );

    form_layout->addWidget( new QLabel( "Poprawna odpowiedź:", form_frame ) );
    correct_input_ = new QLineEdit( form_frame );
    form_layout->addWidget( correct_input_ );

    QHBoxLayout* wrong_header = new QHBoxLayout();
    QLabel* lbl_wrong = new QLabel( "Błędne odpowiedzi (opcjonalne):", form_frame );
    lbl_wrong->setStyleSheet( "border: none;" );
    wrong_header->addWidget( lbl_wrong );

    btn_add_wrong_ = new QPushButton( "+", form_frame );
    btn_add_wrong_->setFixedSize( 30, 30 );
    btn_add_wrong_->setCursor( Qt::PointingHandCursor );
    btn_add_wrong_->setStyleSheet(
        "QPushButton { background-color: #444; color: white; border-radius: 15px; border: none; "
        "}" );
    connect( btn_add_wrong_, &QPushButton::clicked, this, &AddCardOverlay::addWrongAnswerField );

    wrong_header->addWidget( btn_add_wrong_ );
    form_layout->addLayout( wrong_header );

    wrong_answers_layout_ = new QVBoxLayout();
    form_layout->addLayout( wrong_answers_layout_ );

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->setSpacing( 10 );

    QPushButton* btn_cancel = new QPushButton( "Anuluj", form_frame );
    btn_cancel->setCursor( Qt::PointingHandCursor );
    btn_cancel->setStyleSheet(
        "background-color: #c62828; color: white; padding: 8px; border-radius: 4px; border: "
        "none;" );
    connect( btn_cancel, &QPushButton::clicked, this, &AddCardOverlay::cancelled );

    QPushButton* btn_save = new QPushButton( "Zapisz", form_frame );
    btn_save->setCursor( Qt::PointingHandCursor );
    btn_save->setStyleSheet(
        "background-color: #2e7d32; color: white; padding: 8px; border-radius: 4px; border: "
        "none;" );

    connect( btn_save, &QPushButton::clicked, this, [this]() {
        QString q = question_input_->text().trimmed();
        QString c = correct_input_->text().trimmed();

        if ( q.isEmpty() || c.isEmpty() ) {
            QMessageBox::warning( this, "Błąd", "Pytanie i odpowiedź są wymagane!" );
            return;
        }

        std::vector<std::string> wrongs;
        for ( auto* input : wrong_inputs_ ) {
            if ( !input->text().trimmed().isEmpty() )
                wrongs.push_back( input->text().trimmed().toStdString() );
        }

        DraftCard draft{ q.toStdString(), c.toStdString(), wrongs };
        emit cardSaved( draft );
    } );

    btn_layout->addWidget( btn_cancel );
    btn_layout->addWidget( btn_save );
    form_layout->addSpacing( 10 );
    form_layout->addLayout( btn_layout );

    main_layout->addWidget( form_frame );
}

void AddCardOverlay::resetForm() {
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

void AddCardOverlay::addWrongAnswerField() {
    if ( wrong_inputs_.size() >= 3 ) return;
    QLineEdit* input = new QLineEdit( this );
    input->setPlaceholderText( "Błędna odpowiedź..." );
    wrong_answers_layout_->addWidget( input );
    wrong_inputs_.push_back( input );
    if ( wrong_inputs_.size() >= 3 ) btn_add_wrong_->setEnabled( false );
}