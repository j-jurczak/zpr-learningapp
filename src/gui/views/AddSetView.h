/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - header file.
 */
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <vector>
#include "../../db/DatabaseManager.h"

class CardPreviewOverlay;

class AddSetView : public QWidget {
    Q_OBJECT
public:
    explicit AddSetView( DatabaseManager& db, QWidget* parent = nullptr );

signals:
    void creationCancelled();
    void setCreated();

private slots:
    void addCardToDraft();
    void saveSet();
    void creationCancelledClicked();
    void addWrongAnswerField();

protected:
    void resizeEvent( QResizeEvent* event ) override;

private:
    void setupUi();
    void setupStyles();
    void resetCardForm();

    DatabaseManager& db_;

    QLineEdit* name_input_;
    QListWidget* preview_list_;

    QLineEdit* question_input_;
    QLineEdit* correct_input_;
    QPushButton* btn_add_wrong_;
    QVBoxLayout* wrong_answers_layout_;
    std::vector<QLineEdit*> wrong_inputs_;
    std::vector<DraftCard> draft_cards_;

    CardPreviewOverlay* overlay_;
};