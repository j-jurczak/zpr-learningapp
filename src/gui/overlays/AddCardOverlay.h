/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the overlay widget providing a form to add a single card to an existing set.
 */
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <vector>
#include "../../db/DatabaseManager.h"

class AddCardOverlay : public QWidget {
    Q_OBJECT
public:
    explicit AddCardOverlay( QWidget* parent = nullptr );
    void resetForm();

signals:
    void cardSaved( const DraftCard& card );
    void cancelled();

private:
    void setupUi();
    void addWrongAnswerField();

    QLineEdit* question_input_;
    QLineEdit* correct_input_;
    QVBoxLayout* wrong_answers_layout_;
    std::vector<QLineEdit*> wrong_inputs_;
    QPushButton* btn_add_wrong_;
};