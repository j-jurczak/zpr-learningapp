/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class CardPreviewOverlay, manages the overlay for previewing a card - source file.
 */
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

#include "CardPreviewOverlay.h"
#include "../../core/utils/Overloaded.h"
#include "../../core/utils/StyleLoader.h"

using namespace std;

static QString getMediaPath( const string& relative_path ) {
    QString rel = QString::fromStdString( relative_path );
#ifdef PROJECT_ROOT
    QDir dir( QString( PROJECT_ROOT ) );
    return dir.filePath( "data/media/" + rel );
#else
    return QDir::current().filePath( "data/media/" + rel );
#endif
}

CardPreviewOverlay::CardPreviewOverlay( const Card& card, QWidget* parent ) : QWidget( parent ) {
    setupUi( card );
    StyleLoader::attach( this, "overlays/CardPreviewOverlay.qss" );
}

void CardPreviewOverlay::setupUi( const Card& card ) {
    this->setObjectName( "overlayBackground" );

    QVBoxLayout* main_layout = new QVBoxLayout( this );
    main_layout->setContentsMargins( 0, 0, 0, 0 );
    main_layout->setAlignment( Qt::AlignCenter );

    QFrame* content_frame = new QFrame( this );
    content_frame->setObjectName( "contentFrame" );

    QVBoxLayout* card_layout = new QVBoxLayout( content_frame );
    card_layout->setContentsMargins( 0, 0, 0, 20 );
    card_layout->setSpacing( 5 );

    QWidget* top_bar = new QWidget( content_frame );
    top_bar->setObjectName( "topBar" );
    top_bar->setFixedHeight( 40 );

    QHBoxLayout* top_layout = new QHBoxLayout( top_bar );
    top_layout->setContentsMargins( 15, 0, 5, 0 );

    QLabel* title = new QLabel( tr( "CARD PREVIEW" ), top_bar );
    title->setObjectName( "dialogTitle" );

    QPushButton* btn_close = new QPushButton( "✕", top_bar );
    btn_close->setObjectName( "closeBtn" );
    btn_close->setFixedSize( 30, 30 );
    btn_close->setCursor( Qt::PointingHandCursor );

    connect( btn_close, &QPushButton::clicked, this, &CardPreviewOverlay::closeClicked );

    top_layout->addWidget( title );
    top_layout->addStretch();
    top_layout->addWidget( btn_close );
    card_layout->addWidget( top_bar );

    QWidget* body = new QWidget( content_frame );
    QVBoxLayout* body_layout = new QVBoxLayout( body );
    body_layout->setContentsMargins( 20, 10, 20, 10 );
    body_layout->setSpacing( 10 );

    QLabel* lbl_q_header = new QLabel( tr( "QUESTION" ), body );
    lbl_q_header->setObjectName( "sectionHeader" );
    body_layout->addWidget( lbl_q_header );

    const auto& questionPayload = card.getData().question;

    visit(
        overloaded{ [&]( const TextContent& c ) {
                       QLabel* lbl_question = new QLabel( QString::fromStdString( c.text ), body );
                       lbl_question->setObjectName( "questionText" );
                       lbl_question->setWordWrap( true );
                       body_layout->addWidget( lbl_question );
                   },
                    [&]( const ImageContent& c ) {
                        QLabel* imgLabel = new QLabel( body );
                        imgLabel->setAlignment( Qt::AlignCenter );
                        QString path = getMediaPath( c.image_path );
                        QPixmap pix( path );
                        if ( !pix.isNull() ) {
                            imgLabel->setPixmap( pix.scaled( 400, 300, Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation ) );
                        } else {
                            imgLabel->setText( tr( "[Image load error] " ) + path );
                            imgLabel->setObjectName( "errorLabel" );
                        }
                        body_layout->addWidget( imgLabel );
                    },
                    [&]( const SoundContent& c ) {
                        QPushButton* btn = new QPushButton( "▶ " + tr( "Play sound" ), body );
                        btn->setObjectName( "playSoundBtn" );
                        btn->setCursor( Qt::PointingHandCursor );
                        body_layout->addWidget( btn );

                        QLabel* info = new QLabel(
                            tr( "(Sound: " ) + QString::fromStdString( c.sound_path ) + ")", body );
                        info->setObjectName( "soundInfoLabel" );
                        body_layout->addWidget( info );
                        QMediaPlayer* player = new QMediaPlayer( this );
                        QAudioOutput* audioOutput = new QAudioOutput( this );
                        player->setAudioOutput( audioOutput );
                        audioOutput->setVolume( 1.0 );

                        QString path = getMediaPath( c.sound_path );

                        connect( btn, &QPushButton::clicked, this, [player, path, btn]() {
                            if ( player->playbackState() == QMediaPlayer::PlayingState ) {
                                player->stop();
                                btn->setText( "▶ " + tr( "Play sound" ) );
                            } else {
                                player->setSource( QUrl::fromLocalFile( path ) );
                                player->play();
                                btn->setText( "■ " + tr( "Stop" ) );
                            }
                        } );

                        connect( player, &QMediaPlayer::mediaStatusChanged, this,
                                 [player, btn]( QMediaPlayer::MediaStatus status ) {
                                     if ( status == QMediaPlayer::EndOfMedia ) {
                                         btn->setText( "▶ " + tr( "Play sound" ) );
                                     }
                                 } );
                    } },
        questionPayload );

    QLabel* lbl_a_header = new QLabel( tr( "CORRECT ANSWER" ), body );
    lbl_a_header->setObjectName( "sectionHeader" );
    body_layout->addWidget( lbl_a_header );

    QLabel* lbl_correct =
        new QLabel( "✔  " + QString::fromStdString( card.getCorrectAnswer() ), body );
    lbl_correct->setObjectName( "correctAnswer" );
    lbl_correct->setWordWrap( true );
    body_layout->addWidget( lbl_correct );

    const auto& wrong = card.getData().wrong_answers;
    if ( !wrong.empty() ) {
        QLabel* lbl_w_header = new QLabel( tr( "WRONG ANSWERS" ), body );
        lbl_w_header->setObjectName( "sectionHeader" );
        body_layout->addWidget( lbl_w_header );

        for ( const auto& w : wrong ) {
            QLabel* lbl_wrong = new QLabel( "✘  " + QString::fromStdString( w ), body );
            lbl_wrong->setObjectName( "wrongAnswer" );
            lbl_wrong->setWordWrap( true );
            body_layout->addWidget( lbl_wrong );
        }
    }

    card_layout->addWidget( body );
    main_layout->addWidget( content_frame );
}