/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Implementation of LearningView logic.
 */
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QSettings>
#include <QRandomGenerator>
#include <QTimer>
#include <QDir>
#include <QStyle>

#include "LearningView.h"
#include "../../core/utils/Overloaded.h"
#include "../../core/learning/strategies/SelectionStrategies.h"
#include "../../core/utils/StyleLoader.h"

using namespace std;

void updateStyle( QWidget* widget ) {
    widget->style()->unpolish( widget );
    widget->style()->polish( widget );
}

QString getMediaPath( const string& relative_path ) {
    QString rel = QString::fromStdString( relative_path );

#ifdef PROJECT_ROOT
    QDir dir( QString( PROJECT_ROOT ) );
    return dir.filePath( "data/media/" + rel );
#else
    return QDir::current().filePath( "data/media/" + rel );
#endif
}

LearningView::LearningView( DatabaseManager& db, QWidget* parent )
    : QWidget( parent ), db_( db ), session_( db ) {
    setupUi();
    StyleLoader::attach( this, "views/LearningView.qss" );
}

void LearningView::ensureAudioInitialized() {
    if ( player_ ) return;
    player_ = new QMediaPlayer( this );
    audio_output_ = new QAudioOutput( this );
    player_->setAudioOutput( audio_output_ );
    audio_output_->setVolume( 1.0 );

    connect( player_, &QMediaPlayer::errorOccurred, this,
             []( QMediaPlayer::Error error, const QString& errorString ) {
                 qCritical() << "Error with QMediaPlayer:" << errorString;
             } );
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
        session_.start( set_id, std::move( strategy ), 20 );

        try {
            session_.getCurrentCard();
        } catch ( ... ) {
            throw runtime_error( "EMPTY_SESSION" );
        }

        progress_bar_->setValue( 0 );
        loadCurrentCard();

    } catch ( const exception& e ) {
        string error_msg = e.what();
        bool is_sm2_empty =
            ( current_mode_ == LearningMode::SpacedRepetition && error_msg == "EMPTY_SESSION" );

        QTimer::singleShot( 0, this, [this, is_sm2_empty]() {
            if ( is_sm2_empty ) {
                QMessageBox::information(
                    this, tr( "Congratulations!" ),
                    tr( "That's all for today!\nSM-2 algorithm does not predict "
                        "more reviews for now.\n\n"
                        "If you want to continue learning, choose 'Quick Review' mode "
                        "or reset progress in the menu." ) );
            } else {
                QMessageBox::warning( this, tr( "Empty" ), tr( "This set contains no cards!" ) );
            }
            emit sessionFinished();
        } );
    }
}

void LearningView::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 30, 20, 30, 40 );
    main_layout->setSpacing( 20 );

    QHBoxLayout* header_layout = new QHBoxLayout();

    QPushButton* btn_exit = new QPushButton( "✕", this );
    btn_exit->setObjectName( "btnExit" );
    btn_exit->setFixedSize( 40, 40 );
    btn_exit->setCursor( Qt::PointingHandCursor );
    btn_exit->setToolTip( tr( "Interrupt session and exit" ) );

    connect( btn_exit, &QPushButton::clicked, this, [this]() {
        auto reply = QMessageBox::question(
            this, tr( "Interrupt session" ),
            tr( "Are you sure you want to end the session? Your progress is saved." ),
            QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::Yes ) {
            emit sessionFinished();
        }
    } );

    progress_bar_ = new QProgressBar( this );
    progress_bar_->setTextVisible( false );
    progress_bar_->setFixedHeight( 8 );

    header_layout->addWidget( btn_exit );
    header_layout->addWidget( progress_bar_ );

    main_layout->addLayout( header_layout );

    card_frame_ = new QFrame( this );
    card_frame_->setObjectName( "cardFrame" );

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
    if ( player_ ) {
        player_->stop();
        player_->setSource( QUrl() );
    }
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

    visit( overloaded{ [&]( const TextContent& c ) {
                          QLabel* l = new QLabel( QString::fromStdString( c.text ), card_frame_ );
                          l->setObjectName( "questionLabel" );
                          l->setWordWrap( true );
                          l->setAlignment( Qt::AlignCenter );
                          question_layout_->addWidget( l );
                      },
                       [&]( const ImageContent& c ) {
                           QLabel* imgLabel = new QLabel( card_frame_ );
                           imgLabel->setAlignment( Qt::AlignCenter );

                           QString path = getMediaPath( c.image_path );
                           QPixmap pix( path );

                           if ( !pix.isNull() ) {
                               imgLabel->setPixmap( pix.scaled( 500, 350, Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation ) );
                           } else {
                               qDebug() << "Image load error:" << path;
                               imgLabel->setText( tr( "[Image load error]\n" ) + path );
                               imgLabel->setProperty( "type", "error" );  // Styl błędu w QSS
                           }
                           question_layout_->addWidget( imgLabel );
                       },
                       [&]( const SoundContent& c ) {
                           QPushButton* btn =
                               new QPushButton( "▶ " + tr( "Play sound" ), card_frame_ );
                           btn->setObjectName( "soundBtn" );
                           btn->setFixedSize( 200, 60 );
                           btn->setCursor( Qt::PointingHandCursor );

                           QString path = getMediaPath( c.sound_path );
                           if ( !QFile::exists( path ) )
                               qCritical() << "File does not exist:" << path;

                           connect( btn, &QPushButton::clicked, this, [this, path, btn]() {
                               ensureAudioInitialized();
                               if ( player_ ) {
                                   player_->disconnect( btn );
                                   btn->setEnabled( false );
                                   connect( player_, &QMediaPlayer::mediaStatusChanged, btn,
                                            [this, btn]( QMediaPlayer::MediaStatus status ) {
                                                if ( status == QMediaPlayer::EndOfMedia ) {
                                                    btn->setEnabled( true );
                                                    player_->stop();
                                                }
                                            } );

                                   player_->setSource( QUrl::fromLocalFile( path ) );
                                   player_->play();
                               }
                           } );
                           question_layout_->addWidget( btn );
                           QLabel* info = new QLabel(
                               tr( "(Sound: " ) + QString::fromStdString( c.sound_path ) + ")",
                               card_frame_ );
                           info->setObjectName( "soundInfoLabel" );
                           question_layout_->addWidget( info );
                       } },
           questionPayload );
}

void LearningView::renderInteraction( const Card& card ) {
    const CardData& data = card.getData();
    QSettings settings( "ZPR", "LearningApp" );
    bool allow_quiz = settings.value( "enable_quiz", true ).toBool();
    bool allow_input = settings.value( "enable_input", true ).toBool();
    bool random_mode = settings.value( "randomize_simple_cards", false ).toBool();
    bool has_distractors = !data.wrong_answers.empty();
    AnswerType db_type = data.answer_type;
    bool is_text_answer =
        ( db_type != AnswerType::IMAGE_CHOICE && db_type != AnswerType::SOUND_CHOICE );

    AnswerType mode = db_type;
    if ( random_mode ) {
        if ( has_distractors ) {
            bool can_do_quiz = allow_quiz;
            bool can_do_input = allow_input && is_text_answer;

            if ( can_do_quiz && can_do_input ) {
                if ( QRandomGenerator::global()->bounded( 2 ) == 0 )
                    mode = AnswerType::TEXT_CHOICE;
                else
                    mode = AnswerType::INPUT;
            } else if ( can_do_quiz )
                mode = AnswerType::TEXT_CHOICE;
            else if ( can_do_input )
                mode = AnswerType::INPUT;
            else
                mode = AnswerType::FLASHCARD;
        } else {
            if ( allow_input && is_text_answer ) {
                if ( QRandomGenerator::global()->bounded( 2 ) == 0 )
                    mode = AnswerType::INPUT;
                else
                    mode = AnswerType::FLASHCARD;
            } else
                mode = AnswerType::FLASHCARD;
        }
    } else {
        if ( mode == AnswerType::TEXT_CHOICE || mode == AnswerType::IMAGE_CHOICE ) {
            if ( !allow_quiz || !has_distractors ) {
                mode =
                    ( allow_input && is_text_answer ) ? AnswerType::INPUT : AnswerType::FLASHCARD;
            }
        }
        if ( mode == AnswerType::INPUT && !allow_input ) {
            mode = AnswerType::FLASHCARD;
        }
    }
    switch ( mode ) {
        case AnswerType::INPUT:
            renderInputView( data );
            break;
        case AnswerType::TEXT_CHOICE:
            renderQuizView( data );
            break;
        case AnswerType::IMAGE_CHOICE:
            renderQuizView( data );
            break;
        case AnswerType::FLASHCARD:
        default:
            renderFlashcardView( data );
            break;
    }
}

void LearningView::renderFlashcardView( const CardData& data ) {
    QPushButton* btn_show = new QPushButton( tr( "Show answer" ), interaction_container_ );
    btn_show->setObjectName( "showAnswerBtn" );
    btn_show->setCursor( Qt::PointingHandCursor );
    btn_show->setMinimumHeight( 50 );

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
        btn->setObjectName( "quizBtn" );

        connect( btn, &QPushButton::clicked, this,
                 [this, choice, btn]() { onChoiceClicked( choice, btn ); } );

        interaction_layout_->addWidget( btn );
    }
}

void LearningView::renderInputView( const CardData& data ) {
    QLineEdit* input = new QLineEdit( interaction_container_ );
    input->setPlaceholderText( tr( "Enter answer..." ) );
    input->setFocus();

    connect( input, &QLineEdit::returnPressed, this, &LearningView::onInputChecked );
    interaction_layout_->addWidget( input );

    QPushButton* btn_check = new QPushButton( tr( "Check" ), interaction_container_ );
    btn_check->setObjectName( "checkBtn" );
    btn_check->setCursor( Qt::PointingHandCursor );
    btn_check->setMinimumHeight( 50 );

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
    l->setObjectName( "correctAnswerLabel" );
    interaction_layout_->addWidget( l );

    if ( current_mode_ == LearningMode::SpacedRepetition ) {
        showGradingButtons();
    } else {
        clearLayout( bottom_controls_layout_ );
        bottom_controls_container_->show();

        QPushButton* btn_next = new QPushButton( tr( "Next" ), bottom_controls_container_ );
        btn_next->setObjectName( "actionBtn" );
        btn_next->setProperty( "type", "primary" );
        btn_next->setCursor( Qt::PointingHandCursor );
        btn_next->setMinimumHeight( 50 );

        connect( btn_next, &QPushButton::clicked, this, [this]() { onGradeClicked( 5 ); } );

        bottom_controls_layout_->addWidget( btn_next );
        btn_next->setFocus();
    }
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
                btn->setProperty( "state", "correct" );
            } else if ( btn == senderBtn && !is_correct ) {
                btn->setProperty( "state", "wrong" );
            }
            updateStyle( btn );
        }
    }

    if ( current_mode_ == LearningMode::SpacedRepetition ) {
        showGradingButtons();
    } else {
        int grade = is_correct ? 5 : 1;
        clearLayout( bottom_controls_layout_ );
        bottom_controls_container_->show();

        QPushButton* btn_next = new QPushButton(
            is_correct ? tr( "Next (Good!)" ) : tr( "Next (Wrong)" ), bottom_controls_container_ );

        btn_next->setObjectName( "actionBtn" );
        btn_next->setProperty( "type", is_correct ? "success" : "failure" );
        btn_next->setMinimumHeight( 50 );
        btn_next->setCursor( Qt::PointingHandCursor );

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
        input->setProperty( "state", "correct" );
        updateStyle( input );

        if ( current_mode_ == LearningMode::SpacedRepetition ) {
            showGradingButtons();
        } else {
            QPushButton* btn_next =
                new QPushButton( tr( "Next (Good!)" ), bottom_controls_container_ );
            btn_next->setObjectName( "actionBtn" );
            btn_next->setProperty( "type", "success" );
            btn_next->setMinimumHeight( 50 );
            btn_next->setCursor( Qt::PointingHandCursor );

            connect( btn_next, &QPushButton::clicked, this, [this]() { onGradeClicked( 5 ); } );
            bottom_controls_layout_->addWidget( btn_next );
            btn_next->setFocus();
        }
    } else {
        input->setProperty( "state", "wrong" );
        updateStyle( input );

        QLabel* correction = new QLabel( tr( "Correct: " ) + correct_text, interaction_container_ );
        correction->setObjectName( "correctionLabel" );
        interaction_layout_->addWidget( correction );

        QString fail_btn_text = ( current_mode_ == LearningMode::SpacedRepetition )
                                    ? tr( "Proceed to grading" )
                                    : tr( "Next (Wrong)" );

        QPushButton* btn_fail = new QPushButton( fail_btn_text, bottom_controls_container_ );
        btn_fail->setObjectName( "actionBtn" );
        btn_fail->setProperty( "type", "failure" );
        btn_fail->setMinimumHeight( 50 );
        btn_fail->setCursor( Qt::PointingHandCursor );

        connect( btn_fail, &QPushButton::clicked, this, [this]() {
            if ( current_mode_ == LearningMode::SpacedRepetition ) {
                showGradingButtons();
            } else {
                onGradeClicked( 1 );
            }
        } );

        QPushButton* btn_override =
            new QPushButton( tr( "Mark as correct" ), bottom_controls_container_ );
        btn_override->setObjectName( "actionBtn" );
        btn_override->setProperty( "type", "warning" );
        btn_override->setMinimumHeight( 50 );
        btn_override->setCursor( Qt::PointingHandCursor );

        connect( btn_override, &QPushButton::clicked, this, [this]() {
            if ( current_mode_ == LearningMode::SpacedRepetition ) {
                showGradingButtons();
            } else {
                onGradeClicked( 5 );
            }
        } );

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

    struct GradeOption {
        QString text;
        int value;
        QString tooltip;
    };

    std::vector<GradeOption> options = {
        { tr( "0\nBlackout" ), 0, tr( "Complete blackout" ) },
        { tr( "1\nIncorrect" ), 1, tr( "Wrong answer, correct one remembered" ) },
        { tr( "2\nHard Fail" ), 2, tr( "Wrong answer, but it seemed easy" ) },
        { tr( "3\nHard Pass" ), 3, tr( "Correct with difficulty" ) },
        { tr( "4\nGood" ), 4, tr( "Correct after hesitation" ) },
        { tr( "5\nPerfect" ), 5, tr( "Perfect recall" ) } };

    bottom_controls_layout_->setSpacing( 5 );

    for ( const auto& opt : options ) {
        QPushButton* btn = new QPushButton( opt.text, bottom_controls_container_ );
        btn->setCursor( Qt::PointingHandCursor );
        btn->setMinimumHeight( 60 );
        btn->setObjectName( "gradeBtn" );
        btn->setProperty( "grade", opt.value );
        btn->setToolTip( opt.tooltip );
        connect( btn, &QPushButton::clicked, this, [this, opt]() { onGradeClicked( opt.value ); } );
        bottom_controls_layout_->addWidget( btn );
    }
}

void LearningView::clearLayout( QLayout* layout ) {
    if ( !layout ) return;

    QLayoutItem* item;
    while ( ( item = layout->takeAt( 0 ) ) != nullptr ) {
        if ( item->widget() ) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void LearningView::showSummary() {
    progress_bar_->setValue( 100 );
    QMessageBox::information( this, tr( "Finished" ), tr( "Session finished!" ) );
    emit sessionFinished();
}