/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for the learning process (Modular for Flashcards/Quiz).
 */
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QFrame>

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

private:
    void setupUi();
    void loadCurrentCard();
    void showSummary();

    void clearLayout( QLayout* layout );
    void showGradingButtons();

    DatabaseManager& db_;
    LearningSession session_;
    LearningMode current_mode_ = LearningMode::SpacedRepetition;

    QProgressBar* progress_bar_;
    QLabel* lbl_question_;

    QWidget* answer_container_;
    QVBoxLayout* answer_layout_;

    QWidget* interaction_container_;
    QVBoxLayout* interaction_layout_;

    QWidget* bottom_controls_container_;
    QHBoxLayout* bottom_controls_layout_;
};