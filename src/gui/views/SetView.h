/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - header file.
 */
#pragma once
#include <QWidget>
#include <QListWidget>
#include <vector>
#include <QLabel>
#include "../../db/DatabaseManager.h"
#include "../../core/Card.h"

class CardPreviewOverlay;

class SetView : public QWidget {
    Q_OBJECT
public:
    explicit SetView( int set_id, DatabaseManager& db, QWidget* parent = nullptr );

signals:
    void backToSetsClicked();
    void learnClicked( int set_id );
    void deleteSetClicked( int set_id );

protected:
    void resizeEvent( QResizeEvent* event ) override;

private:
    void setupUi();
    void setupStyles();
    void loadData();

    int set_id_;
    DatabaseManager& db_;

    QListWidget* cards_list_;
    QLabel* title_label_;

    CardPreviewOverlay* overlay_;

    std::vector<Card> current_cards_;
};