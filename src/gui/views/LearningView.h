/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for the learning process.
 */
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QFrame>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "../../core/learning/LearningSession.h"
#include "../../db/DatabaseManager.h"

class LearningView : public QWidget {
    Q_OBJECT
public:
    explicit LearningView( DatabaseManager& db, QWidget* parent = nullptr );

    void startSession( int set_id, LearningMode mode = LearningMode::SpacedRepetition );

signals:
    void sessionFinished();

private slots:
    void onShowAnswerClicked();
    void onGradeClicked( int grade );
    void onChoiceClicked( const QString& answer, QPushButton* senderBtn );
    void onInputChecked();

private:
    void setupUi();
    void loadCurrentCard();
    void showSummary();

    void renderQuestion( const Card& card );
    void renderInteraction( const Card& card );

    void renderFlashcardView( const CardData& data );
    void renderQuizView( const CardData& data );
    void renderInputView( const CardData& data );
    void clearLayout( QLayout* layout );
    void showGradingButtons();

    DatabaseManager& db_;
    LearningSession session_;
    LearningMode current_mode_ = LearningMode::SpacedRepetition;

    QProgressBar* progress_bar_;

    QFrame* card_frame_;
    QVBoxLayout* question_layout_;

    QWidget* interaction_container_;
    QVBoxLayout* interaction_layout_;

    QWidget* bottom_controls_container_;
    QHBoxLayout* bottom_controls_layout_;

    QMediaPlayer* player_ = nullptr;
    QAudioOutput* audio_output_ = nullptr;
    void ensureAudioInitialized();
};