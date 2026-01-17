/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: View for creating new study sets - header file.
 */
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <vector>
#include <memory>

#include "../../db/DatabaseManager.h"
#include "../../core/learning/Card.h"

class OverlayContainer;

class AddSetView : public QWidget {
    Q_OBJECT

public:
    explicit AddSetView( DatabaseManager& db, QWidget* parent = nullptr );
    void resizeEvent( QResizeEvent* event ) override;

signals:
    void setCreated();
    void creationCancelled();

private:
    void setupUi();
    void setupStyles();

    void onAddCardClicked();
    void onCardSaved( const DraftCard& card );
    void saveSet();

    DatabaseManager& db_;
    std::unique_ptr<OverlayContainer> overlay_container_;

    QLineEdit* name_input_;
    QListWidget* preview_list_;

    QPushButton* btn_open_creator_;

    std::vector<DraftCard> draft_cards_;
};