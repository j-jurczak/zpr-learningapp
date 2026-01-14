/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for a specific study set - header file.
 */
#pragma once
#include <QWidget>
#include <QListWidget>
#include <vector>
#include <QLabel>
#include <memory>
#include "../../db/DatabaseManager.h"
#include "../../core/Card.h"

class OverlayContainer;
class AddCardOverlay;

class SetView : public QWidget {
    Q_OBJECT
public:
    explicit SetView( int set_id, DatabaseManager& db, QWidget* parent = nullptr );

signals:
    void backToSetsClicked();
    void learnClicked( int set_id );

protected:
    void resizeEvent( QResizeEvent* event ) override;

private:
    void setupUi();
    void loadData();

    int set_id_;
    DatabaseManager& db_;
    QListWidget* cards_list_;
    QLabel* title_label_;

    std::unique_ptr<OverlayContainer> overlay_container_;

    std::unique_ptr<AddCardOverlay> add_overlay_;
    std::unique_ptr<QWidget> current_preview_;

    std::vector<Card> current_cards_;
};