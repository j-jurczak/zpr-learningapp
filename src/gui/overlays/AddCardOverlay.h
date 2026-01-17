/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the overlay widget providing a form to add a single card to an existing set.
 */
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>

#include "../../core/learning/Card.h"

class AddCardOverlay : public QWidget {
    Q_OBJECT

public:
    explicit AddCardOverlay( QWidget* parent = nullptr );

signals:
    void cardSaved( const DraftCard& card );
    void cancelled();

private:
    void setupUi();
    void setupConnections();

    void onQuestionTypeChanged( int index );
    void onAnswerTypeChanged( int index );
    void selectImageFile();
    void selectSoundFile();
    QString copyFileToMedia( const QString& sourcePath, const std::string& subfolder );

    QComboBox* combo_question_type_;
    QComboBox* combo_answer_type_;

    QStackedWidget* stack_question_content_;

    QTextEdit* input_question_text_;

    QWidget* page_image_;
    QLabel* label_image_preview_;
    QPushButton* btn_select_image_;
    QString selected_image_path_;

    QWidget* page_sound_;
    QLabel* label_sound_info_;
    QPushButton* btn_select_sound_;
    QString selected_sound_path_;

    QLineEdit* input_correct_answer_;

    QWidget* quiz_container_;
    QLineEdit* input_wrong1_;
    QLineEdit* input_wrong2_;
    QLineEdit* input_wrong3_;

    QPushButton* btn_save_;
    QPushButton* btn_cancel_;
};