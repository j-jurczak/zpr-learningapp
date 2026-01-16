/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of LearningView logic (Modular: Flashcard & Quiz).
 */
#include <QMessageBox>
#include <QDebug>
#include <algorithm>

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
        "QProgressBar { border: none; background: #333; border-radius: 3px; } "
        "QProgressBar::chunk { background: #007acc; border-radius: 3px; }" );
    main_layout->addWidget( progress_bar_ );

    QFrame* card_frame = new QFrame( this );
    card_frame->setStyleSheet(
        "background-color: #252526; border-radius: 10px; border: 1px solid #3e3e42;" );
    QVBoxLayout* card_layout = new QVBoxLayout( card_frame );
    card_layout->setContentsMargins( 30, 30, 30, 30 );
    card_layout->setSpacing( 20 );

    lbl_question_ = new QLabel( "Pytanie...", card_frame );
    lbl_question_->setWordWrap( true );
    lbl_question_->setAlignment( Qt::AlignCenter );
    lbl_question_->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #ffffff; border: none;" );
    card_layout->addWidget( lbl_question_ );

    QFrame* line = new QFrame();
    line->setFrameShape( QFrame::HLine );
    line->setStyleSheet( "color: #444;" );
    card_layout->addWidget( line );

    answer_container_ = new QWidget( card_frame );
    answer_container_->setStyleSheet( "border: none;" );
    answer_layout_ = new QVBoxLayout( answer_container_ );
    answer_layout_->setContentsMargins( 0, 10, 0, 0 );
    card_layout->addWidget( answer_container_ );

    main_layout->addWidget( card_frame, 1 );

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

        lbl_question_->setText( QString::fromStdString( card.getQuestion() ) );
        clearLayout( answer_layout_ );
        clearLayout( interaction_layout_ );
        clearLayout( bottom_controls_layout_ );
        answer_container_->hide();
        bottom_controls_container_->hide();

        progress_bar_->setValue( static_cast<int>( session_.getProgress() * 100 ) );

        visit(
            overloaded{
                [&]( const StandardData& d ) {
                    QLabel* l = new QLabel( QString::fromStdString( d.correct_answer ) );
                    l->setAlignment( Qt::AlignCenter );
                    l->setWordWrap( true );
                    l->setStyleSheet( "font-size: 20px; color: #a5d6a7; font-weight: bold;" );
                    answer_layout_->addWidget( l );

                    QPushButton* btn_show =
                        new QPushButton( "Pokaż odpowiedź", interaction_container_ );
                    btn_show->setCursor( Qt::PointingHandCursor );
                    btn_show->setMinimumHeight( 50 );
                    btn_show->setStyleSheet(
                        "background-color: #0078d4; color: white; font-size: 16px; font-weight: "
                        "bold; border-radius: 5px;" );
                    connect( btn_show, &QPushButton::clicked, this,
                             &LearningView::onShowAnswerClicked );
                    interaction_layout_->addWidget( btn_show );
                },
                [&]( const ChoiceData& d ) {
                    vector<string> choices = card.getChoices();

                    for ( const auto& choice_str : choices ) {
                        QString choice = QString::fromStdString( choice_str );
                        QPushButton* btn = new QPushButton( choice, interaction_container_ );
                        btn->setCursor( Qt::PointingHandCursor );
                        btn->setMinimumHeight( 50 );
                        btn->setStyleSheet(
                            "QPushButton { background-color: #3e3e42; color: white; font-size: "
                            "16px; border-radius: 5px; border: 1px solid #555; text-align: left; "
                            "padding-left: 15px; }"
                            "QPushButton:hover { background-color: #4e4e52; }" );

                        connect( btn, &QPushButton::clicked, this,
                                 [this, choice, btn]() { onChoiceClicked( choice, btn ); } );

                        interaction_layout_->addWidget( btn );
                    }
                } },
            card.getData() );

    } catch ( const exception& e ) {
        showSummary();
    }
}

void LearningView::onShowAnswerClicked() {
    clearLayout( interaction_layout_ );
    answer_container_->show();
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
        btn_next->setCursor( Qt::PointingHandCursor );
        btn_next->setMinimumHeight( 50 );
        QString color = is_correct ? "#388e3c" : "#d32f2f";
        btn_next->setStyleSheet(
            QString( "background-color: %1; color: white; font-weight: bold; border-radius: 5px;" )
                .arg( color ) );

        connect( btn_next, &QPushButton::clicked, this,
                 [this, grade]() { onGradeClicked( grade ); } );
        bottom_controls_layout_->addWidget( btn_next );
        btn_next->setFocus();
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
    auto createBtn = [&]( const QString& text, const QString& baseColor, const QString& hoverColor,
                          int g ) {
        QPushButton* btn = new QPushButton( text, bottom_controls_container_ );
        btn->setCursor( Qt::PointingHandCursor );
        btn->setMinimumHeight( 50 );
        btn->setStyleSheet( QString( "QPushButton { background-color: %1; color: white; "
                                     "font-weight: bold; border-radius: 5px; border: none; }"
                                     "QPushButton:hover { background-color: %2; }" )
                                .arg( baseColor, hoverColor ) );

        connect( btn, &QPushButton::clicked, this, [this, g]() { onGradeClicked( g ); } );
        bottom_controls_layout_->addWidget( btn );
    };

    createBtn( "Powtórz (0)", "#d32f2f", "#e53935", 0 );
    createBtn( "Trudne (3)", "#f57f17", "#fbc02d", 3 );
    createBtn( "Dobre (4)", "#388e3c", "#43a047", 4 );
    createBtn( "Łatwe (5)", "#0288d1", "#039be5", 5 );
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