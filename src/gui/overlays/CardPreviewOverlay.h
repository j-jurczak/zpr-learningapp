/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - header file.
 */
#pragma once
#include <QWidget>
#include <vector>
#include <string>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>

class CardPreviewOverlay : public QWidget {
    Q_OBJECT
public:
    explicit CardPreviewOverlay( QWidget* parent = nullptr );

    void showCard( const std::string& question, const std::string& correct,
                   const std::vector<std::string>& wrong );

protected:
    void mousePressEvent( QMouseEvent* event ) override;

private:
    void setupUi();

    QLabel* lbl_question_;
    QLabel* lbl_correct_;
    QLabel* lbl_wrong_header_;
    QFrame* wrong_container_;
    QVBoxLayout* wrong_layout_;
};