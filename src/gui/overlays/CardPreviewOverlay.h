/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - header file.
 */
#pragma once
#include <QWidget>
#include <vector>
#include <string>

class CardPreviewOverlay : public QWidget {
    Q_OBJECT
public:
    CardPreviewOverlay( const std::string& question, const std::string& correct,
                        const std::vector<std::string>& wrong, QWidget* parent = nullptr );

signals:
    void closeClicked();

private:
    void setupUi( const std::string& question, const std::string& correct,
                  const std::vector<std::string>& wrong );
};