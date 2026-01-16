/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of LearningView logic.
 */
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>

#include "LearningView.h"
#include "../../core/utils/Overloaded.h"
#include "../../core/learning/strategies/SelectionStrategies.h"

using namespace std;

LearningView::LearningView( DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), db_( db ), session_( db ) {
    setupUi();
}

void LearningView::startSession( int set_id, LearningMode mode ) {
    current_mode_ = mode;
    unique_ptr<ICardSelectionStrategy> strategy;

    switch ( current_mode_ ) {
        case LearningMode::SpacedRepetition:
            strategy = make_unique<SpacedRepetitionStrategy>();
            break;

        case LearningMode::Random:
            strategy = make_unique<RandomSelectionStrategy>();
            break;

        default:
            strategy = make_unique<SpacedRepetitionStrategy>();
            break;
    }

    try {
        // must use std::move because QWidget also has move
        session_.start( set_id, std::move( strategy ), 20 );
        progress_bar_->setValue( 0 );
        loadCurrentCard();
    } catch ( const exception& e ) {
        QMessageBox::warning( this, "Info", "Brak kart do nauki w tym zestawie!" );
        emit sessionFinished();
    }
}

void LearningView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 40, 30, 40 );
    main_layout->setSpacing( 20 );

    progress_bar_ = new QProgressBar( this );
    progress_bar_->setTextVisible( false );
    progress_bar_->setFixedHeight( 6 );
    progress_bar_->setStyleSheet(
        "QProgressBar { border: none; background: #333; border-radius: 3px; } QProgressBar::chunk "
        "{ background: #007acc; border-radius: 3px; }" );
    main_layout->addWidget( progress_bar_ );

    card_frame_ = new QFrame( this );
    card_frame_->setStyleSheet(
        "background-color: #252526; border-radius: 10px; border: 1px solid #3e3e42;" );

    question_layout_ = new QVBoxLayout( card_frame_ );
    question_layout_->setContentsMargins( 30, 30, 30, 30 );
    question_layout_->setAlignment( Qt::AlignCenter );

    main_layout->addWidget( card_frame_, 1 );

    interaction_container_ = new QWidget( this );
    interaction_layout_ = new QVBoxLayout( interaction_container_ );
    interaction_layout_->setContentsMargins( 0, 0, 0, 0 );
    interaction_layout_->setSpacing( 10 );
    main_layout->addWidget( interaction_container_ );

    bottom_controls_container_ = new QWidget( this );
    bottom_controls_layout_ = new QHBoxLayout( bottom_controls_container_ );
    bottom_controls_layout_->setContentsMargins( 0, 0, 0, 0 );
    bottom_controls_layout_->setSpacing( 10 );
    main_layout->addWidget( bottom_controls_container_ );
}

void LearningView::loadCurrentCard() {
    try {
        const Card& card = session_.getCurrentCard();

        clearLayout( question_layout_ );
        clearLayout( interaction_layout_ );
        clearLayout( bottom_controls_layout_ );
        bottom_controls_container_->hide();

        progress_bar_->setValue( static_cast<int>( session_.getProgress() * 100 ) );

        renderQuestion( card );
        renderInteraction( card );

    } catch ( const exception& e ) {
        showSummary();
    }
}

void LearningView::renderQuestion( const Card& card ) {
    const auto& questionPayload = card.getData().question;

    visit( overloaded{
               [&]( const TextContent& c ) {
                   QLabel* l = new QLabel( QString::fromStdString( c.text ), card_frame_ );
                   l->setWordWrap( true );
                   l->setAlignment( Qt::AlignCenter );
                   l->setStyleSheet(
                       "font-size: 24px; font-weight: bold; color: #ffffff; border: none;" );
                   question_layout_->addWidget( l );
               },
               [&]( const ImageContent& c ) {
                   QLabel* imgLabel = new QLabel( card_frame_ );
                   imgLabel->setAlignment( Qt::AlignCenter );
                   QString path = db_.getImagesPath() + QString::fromStdString( c.image_path );
                   QPixmap pix( path );
                   if ( !pix.isNull() ) {
                       imgLabel->setPixmap(
                           pix.scaled( 400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
                   } else {
                       imgLabel->setText(
                           "[Brak obrazka: " + QString::fromStdString( c.image_path ) + "]" );
                       imgLabel->setStyleSheet( "color: #e53935;" );
                   }
                   question_layout_->addWidget( imgLabel );
               },
               [&]( const SoundContent& c ) {
                   QPushButton* btn = new QPushButton( "▶ Odtwórz dźwięk", card_frame_ );
                   btn->setFixedSize( 200, 60 );
                   btn->setStyleSheet( "font-size: 16px;" );
                   // to-do: implement sound playback
                   question_layout_->addWidget( btn );
                   QLabel* info = new QLabel(
                       "(Dźwięk: " + QString::fromStdString( c.sound_path ) + ")", card_frame_ );
                   info->setStyleSheet( "color: #888; border: none;" );
                   question_layout_->addWidget( info );
               } },
           questionPayload );
}

void LearningView::renderInteraction( const Card& card ) {
    const CardData& data = card.getData();
    AnswerType mode = data.answer_type;
    switch ( mode ) {
        case AnswerType::INPUT:
            renderInputView( data );
            break;

        case AnswerType::TEXT_CHOICE:
            renderQuizView( data );
            break;

        case AnswerType::IMAGE_CHOICE:
            // to-do: implement image choice rendering
            // fallback to quiz view for now
            renderQuizView( data );
            break;

        case AnswerType::FLASHCARD:
        default:
            renderFlashcardView( data );
            break;
    }
}

void LearningView::renderFlashcardView( const CardData& data ) {
    QPushButton* btn_show = new QPushButton( "Pokaż odpowiedź", interaction_container_ );
    btn_show->setCursor( Qt::PointingHandCursor );
    btn_show->setMinimumHeight( 50 );
    btn_show->setStyleSheet(
        "background-color: #0078d4; color: white; font-size: 16px; font-weight: bold; "
        "border-radius: 5px;" );
    connect( btn_show, &QPushButton::clicked, this, &LearningView::onShowAnswerClicked );
    interaction_layout_->addWidget( btn_show );
}

void LearningView::renderQuizView( const CardData& data ) {
    vector<string> choices = session_.getCurrentCard().getChoices();

    for ( const auto& choice_str : choices ) {
        QString choice = QString::fromStdString( choice_str );
        QPushButton* btn = new QPushButton( choice, interaction_container_ );
        btn->setCursor( Qt::PointingHandCursor );
        btn->setMinimumHeight( 50 );
        btn->setStyleSheet(
            "QPushButton { background-color: #3e3e42; color: white; font-size: 16px; "
            "border-radius: 5px; border: 1px solid #555; text-align: left; padding-left: 15px; }"
            "QPushButton:hover { background-color: #4e4e52; }" );

        connect( btn, &QPushButton::clicked, this,
                 [this, choice, btn]() { onChoiceClicked( choice, btn ); } );

        interaction_layout_->addWidget( btn );
    }
}

void LearningView::renderInputView( const CardData& data ) {
    QLineEdit* input = new QLineEdit( interaction_container_ );
    input->setPlaceholderText( "Wpisz odpowiedź..." );
    input->setStyleSheet(
        "padding: 10px; font-size: 16px; color: white; background: #3e3e42; border: 1px solid "
        "#555; border-radius: 5px;" );
    input->setFocus();

    connect( input, &QLineEdit::returnPressed, this, &LearningView::onInputChecked );
    interaction_layout_->addWidget( input );

    QPushButton* btn_check = new QPushButton( "Sprawdź", interaction_container_ );
    btn_check->setCursor( Qt::PointingHandCursor );
    btn_check->setMinimumHeight( 50 );
    btn_check->setStyleSheet(
        "background-color: #0078d4; color: white; font-size: 16px; font-weight: bold; "
        "border-radius: 5px;" );

    connect( btn_check, &QPushButton::clicked, this, &LearningView::onInputChecked );
    interaction_layout_->addWidget( btn_check );
}

void LearningView::onShowAnswerClicked() {
    clearLayout( interaction_layout_ );

    const Card& card = session_.getCurrentCard();
    QLabel* l =
        new QLabel( QString::fromStdString( card.getCorrectAnswer() ), interaction_container_ );
    l->setAlignment( Qt::AlignCenter );
    l->setWordWrap( true );
    l->setStyleSheet( "font-size: 20px; color: #a5d6a7; font-weight: bold; margin: 20px;" );
    interaction_layout_->addWidget( l );

    showGradingButtons();
}

void LearningView::onChoiceClicked( const QString& answer, QPushButton* senderBtn ) {
    const Card& card = session_.getCurrentCard();
    QString correct = QString::fromStdString( card.getCorrectAnswer() );

    bool is_correct = ( QString::compare( answer, correct, Qt::CaseInsensitive ) == 0 );

    for ( int i = 0; i < interaction_layout_->count(); ++i ) {
        QWidget* w = interaction_layout_->itemAt( i )->widget();
        if ( QPushButton* btn = qobject_cast<QPushButton*>( w ) ) {
            btn->setEnabled( false );

            if ( btn->text() == correct ) {
                btn->setStyleSheet(
                    "background-color: #388e3c; color: white; border: 1px solid #388e3c; "
                    "font-weight: bold; padding: 10px; border-radius: 5px;" );
            } else if ( btn == senderBtn && !is_correct ) {
                btn->setStyleSheet(
                    "background-color: #d32f2f; color: white; border: 1px solid #d32f2f; "
                    "font-weight: bold; padding: 10px; border-radius: 5px;" );
            }
        }
    }

    if ( current_mode_ == LearningMode::SpacedRepetition ) {
        showGradingButtons();
    } else {
        int grade = is_correct ? 5 : 1;
        clearLayout( bottom_controls_layout_ );
        bottom_controls_container_->show();

        QPushButton* btn_next = new QPushButton( is_correct ? "Dalej (Dobrze!)" : "Dalej (Błąd)",
                                                 bottom_controls_container_ );
        btn_next->setStyleSheet( QString( "background-color: %1; color: white; font-weight: bold; "
                                          "border-radius: 5px; padding: 10px;" )
                                     .arg( is_correct ? "#388e3c" : "#d32f2f" ) );
        connect( btn_next, &QPushButton::clicked, this,
                 [this, grade]() { onGradeClicked( grade ); } );
        bottom_controls_layout_->addWidget( btn_next );
        btn_next->setFocus();
    }
}

void LearningView::onInputChecked() {
    QLineEdit* input = nullptr;
    for ( int i = 0; i < interaction_layout_->count(); ++i ) {
        if ( QLineEdit* le =
                 qobject_cast<QLineEdit*>( interaction_layout_->itemAt( i )->widget() ) ) {
            input = le;
            break;
        }
    }
    if ( !input ) return;

    QString user_answer = input->text().trimmed();
    const Card& card = session_.getCurrentCard();
    bool is_correct = card.checkAnswer( user_answer.toStdString() );
    QString correct_text = QString::fromStdString( card.getCorrectAnswer() );

    input->setReadOnly( true );

    for ( int i = 0; i < interaction_layout_->count(); ++i ) {
        if ( QPushButton* btn =
                 qobject_cast<QPushButton*>( interaction_layout_->itemAt( i )->widget() ) ) {
            btn->hide();
        }
    }

    clearLayout( bottom_controls_layout_ );
    bottom_controls_container_->show();

    if ( is_correct ) {
        input->setStyleSheet(
            "padding: 10px; font-size: 16px; color: white; background: #2e7d32; border: 1px solid "
            "#2e7d32; border-radius: 5px;" );

        QPushButton* btn_next = new QPushButton( "Dalej (Dobrze!)", bottom_controls_container_ );
        btn_next->setStyleSheet(
            "background-color: #388e3c; color: white; font-weight: bold; padding: 10px; "
            "border-radius: 5px;" );
        connect( btn_next, &QPushButton::clicked, this, [this]() { onGradeClicked( 5 ); } );
        bottom_controls_layout_->addWidget( btn_next );
        btn_next->setFocus();
    } else {
        input->setStyleSheet(
            "padding: 10px; font-size: 16px; color: white; background: #c62828; border: 1px solid "
            "#c62828; border-radius: 5px;" );

        QLabel* correction = new QLabel( "Poprawna: " + correct_text, interaction_container_ );
        correction->setStyleSheet(
            "color: #a5d6a7; font-weight: bold; margin-top: 5px; font-size: 16px;" );
        interaction_layout_->addWidget( correction );

        QPushButton* btn_fail = new QPushButton( "Dalej (Błąd)", bottom_controls_container_ );
        btn_fail->setStyleSheet(
            "background-color: #d32f2f; color: white; font-weight: bold; padding: 10px; "
            "border-radius: 5px;" );
        connect( btn_fail, &QPushButton::clicked, this, [this]() { onGradeClicked( 1 ); } );

        QPushButton* btn_override =
            new QPushButton( "Uznaj za poprawne", bottom_controls_container_ );
        btn_override->setStyleSheet(
            "background-color: #f57f17; color: white; font-weight: bold; padding: 10px; "
            "border-radius: 5px;" );
        connect( btn_override, &QPushButton::clicked, this, [this]() { onGradeClicked( 5 ); } );

        bottom_controls_layout_->addWidget( btn_fail );
        bottom_controls_layout_->addWidget( btn_override );
        btn_fail->setFocus();
    }
}

void LearningView::onGradeClicked( int grade ) {
    session_.submitGrade( grade );
    if ( session_.nextCard() ) {
        loadCurrentCard();
    } else {
        showSummary();
    }
}

void LearningView::showGradingButtons() {
    clearLayout( bottom_controls_layout_ );
    bottom_controls_container_->show();

    auto createBtn = [&]( const QString& text, const QString& col, int g ) {
        QPushButton* btn = new QPushButton( text, bottom_controls_container_ );
        btn->setCursor( Qt::PointingHandCursor );
        btn->setMinimumHeight( 50 );
        btn->setStyleSheet( QString( "background-color: %1; color: white; font-weight: bold; "
                                     "border-radius: 5px; border: none;" )
                                .arg( col ) );
        connect( btn, &QPushButton::clicked, this, [this, g]() { onGradeClicked( g ); } );
        bottom_controls_layout_->addWidget( btn );
    };

    createBtn( "Powtórz (0)", "#d32f2f", 0 );
    createBtn( "Trudne (3)", "#f57f17", 3 );
    createBtn( "Dobre (4)", "#388e3c", 4 );
    createBtn( "Łatwe (5)", "#0288d1", 5 );
}

void LearningView::clearLayout( QLayout* layout ) {
    if ( !layout ) return;
    QLayoutItem* item;
    while ( ( item = layout->takeAt( 0 ) ) != nullptr ) {
        if ( item->widget() ) delete item->widget();
        delete item;
    }
}

void LearningView::showSummary() {
    progress_bar_->setValue( 100 );
    QMessageBox::information( this, "Koniec", "Sesja zakończona!" );
    emit sessionFinished();
}