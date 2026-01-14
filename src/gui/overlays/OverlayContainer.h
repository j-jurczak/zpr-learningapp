/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Header for the container widget acting as a modal wrapper for displaying overlays over
 * views.
 */
#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPointer>

class OverlayContainer : public QWidget {
    Q_OBJECT
public:
    explicit OverlayContainer( QWidget* parent = nullptr );
    void setContent( QWidget* widget );
    void clearContent();

protected:
    void resizeEvent( QResizeEvent* event ) override;
    void mousePressEvent( QMouseEvent* event ) override {}

private:
    QVBoxLayout* main_layout_;
    QPointer<QWidget> current_content_;
};