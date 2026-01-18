/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - header file.
 */
#pragma once
#include <QWidget>

#include "../../core/learning/Card.h"

class CardPreviewOverlay : public QWidget {
    Q_OBJECT
public:
    explicit CardPreviewOverlay( const Card& card, QWidget* parent = nullptr );

signals:
    void closeClicked();

private:
    void setupUi( const Card& card );
};