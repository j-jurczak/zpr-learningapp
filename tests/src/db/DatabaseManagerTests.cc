#include <catch2/catch_test_macros.hpp>
#include <QDir>
#include <QFile>
#include <QDate>
#include "db/DatabaseManager.h"
#include "core/learning/Card.h"

using namespace std;

TEST_CASE( "Database Manager Operations", "[Database]" ) {
    QString test_db_name = "test_db_" + QString::number( rand() % 10000 ) + ".sqlite";
    DatabaseManager db( test_db_name );

    REQUIRE( db.connect() );
    REQUIRE( db.createTables() );
    db.flushData();

    SECTION( "Set Management" ) {
        vector<DraftCard> cards;
        DraftCard c1;
        c1.question = TextContent{ "Q1" };
        c1.correct_answer = "A1";
        c1.answer_type = AnswerType::FLASHCARD;
        cards.push_back( c1 );

        REQUIRE( db.createSet( "Test Set 1", cards ) );

        vector<StudySet> sets = db.getAllSets();
        REQUIRE( sets.size() == 1 );
        REQUIRE( sets[0].name == "Test Set 1" );

        int set_id = sets[0].id;
        auto set_opt = db.getSet( set_id );
        REQUIRE( set_opt.has_value() );
        REQUIRE( set_opt->name == "Test Set 1" );

        REQUIRE( db.deleteSet( set_id ) );
        REQUIRE( db.getAllSets().empty() );

        REQUIRE_FALSE( db.deleteSet( 99999 ) );
    }

    SECTION( "Card Management" ) {
        vector<DraftCard> init_cards;
        REQUIRE( db.createSet( "Card Set", init_cards ) );

        int set_id = db.getAllSets()[0].id;

        DraftCard c;
        c.question = TextContent{ "Q_Add" };
        c.correct_answer = "A_Add";
        c.answer_type = AnswerType::FLASHCARD;

        REQUIRE( db.addCardToSet( set_id, c ) );

        vector<Card> cards = db.getCardsForSet( set_id );
        REQUIRE( cards.size() == 1 );
        REQUIRE( cards[0].getQuestion() == "Q_Add" );

        REQUIRE( db.deleteCard( cards[0].getId() ) );
        REQUIRE( db.getCardsForSet( set_id ).empty() );
        REQUIRE_FALSE( db.deleteCard( 99999 ) );
    }

    SECTION( "Card Progress" ) {
        vector<DraftCard> cards;
        DraftCard c1;
        c1.question = TextContent{ "P1" };
        c1.correct_answer = "A1";
        cards.push_back( c1 );
        db.createSet( "Progress Set", cards );

        int set_id = db.getAllSets()[0].id;
        int card_id = db.getCardsForSet( set_id )[0].getId();

        REQUIRE( db.updateCardProgress( card_id, 1, 1, 2.5f, DatabaseManager::calculateNextDate( 1 ) ) );

        auto [interval, reps, ef] = db.getCardProgress( card_id );
        REQUIRE( interval == 1 );
        REQUIRE( reps == 1 );
        REQUIRE( ef == 2.5f );

        REQUIRE( db.resetSetProgress( set_id ) );
        auto [ri, rr, re] = db.getCardProgress( card_id );
        REQUIRE( ri == 0 );
        REQUIRE( rr == 0 );
        REQUIRE( re == 2.5f );
    }

    SECTION( "Due Cards Logic" ) {
        vector<DraftCard> cards;
        DraftCard c;
        c.question = TextContent{ "DueQ" };
        c.correct_answer = "DueA";
        cards.push_back(c);
        db.createSet("Due Set", cards);
        int set_id = db.getAllSets()[0].id;
        int card_id = db.getCardsForSet(set_id)[0].getId();

        vector<Card> due = db.getDueCards(set_id, 10);
        REQUIRE(due.size() == 1);

        string future_date = DatabaseManager::calculateNextDate(10);
        db.updateCardProgress(card_id, 1, 1, 2.5, future_date);

        due = db.getDueCards(set_id, 10);
        REQUIRE(due.empty());

        string past_date = DatabaseManager::calculateNextDate(-1);
        db.updateCardProgress(card_id, 1, 1, 2.5, past_date);

        due = db.getDueCards(set_id, 10);
        REQUIRE(due.size() == 1);
    }

    SECTION( "Random Card Selection" ) {
        vector<DraftCard> cards;
        for( int i = 0; i < 10; ++i ) {
             DraftCard c;
             c.question = TextContent{ "RQ" + to_string(i) };
             c.correct_answer = "RA" + to_string(i);
             cards.push_back(c);
        }
        db.createSet("Random Set", cards);
        int set_id = db.getAllSets()[0].id;

        vector<Card> random_cards = db.getRandomCards(set_id, 5);
        REQUIRE(random_cards.size() == 5);

        random_cards = db.getRandomCards(set_id, 15);
        REQUIRE(random_cards.size() == 10);
    }

    SECTION( "Statistics Detailed" ) {
        vector<DraftCard> cards;
        cards.push_back({TextContent{"S1"}, "A1"});
        cards.push_back({TextContent{"S2"}, "A2"});
        cards.push_back({TextContent{"S3"}, "A3"});

        db.createSet("Stats Set", cards);
        int set_id = db.getAllSets()[0].id;

        SetStats stats = db.getSetStatistics(set_id);
        REQUIRE(stats.total == 3);
        REQUIRE(stats.new_cards == 3);
        REQUIRE(stats.learning == 0);
        REQUIRE(stats.mastered == 0);

        vector<Card> db_cards = db.getCardsForSet(set_id);

        db.updateCardProgress(db_cards[0].getId(), 1, 1, 2.5, DatabaseManager::calculateNextDate(1));

        db.updateCardProgress(db_cards[1].getId(), 22, 5, 2.5, DatabaseManager::calculateNextDate(10));

        stats = db.getSetStatistics(set_id);
        REQUIRE(stats.new_cards == 1);
        REQUIRE(stats.learning == 1);
        REQUIRE(stats.mastered == 1);
    }

    SECTION( "Statistics" ) {
        vector<DraftCard> cards;
        cards.push_back({TextContent{"S1"}, "A1"});
        cards.push_back({TextContent{"S2"}, "A2"});

        db.createSet("Stats Set", cards);
        int set_id = db.getAllSets()[0].id;

        SetStats stats = db.getSetStatistics(set_id);
        REQUIRE(stats.total == 2);
        REQUIRE(stats.new_cards == 2);
        REQUIRE(stats.learning == 0);
        REQUIRE(stats.mastered == 0);

        int c1 = db.getCardsForSet(set_id)[0].getId();
        db.updateCardProgress(c1, 1, 1, 2.5, DatabaseManager::calculateNextDate(1));

        stats = db.getSetStatistics(set_id);
        REQUIRE(stats.new_cards == 1);
        REQUIRE(stats.learning == 1);

        db.updateCardProgress(c1, 22, 1, 2.5, DatabaseManager::calculateNextDate(1));
        stats = db.getSetStatistics(set_id);
        REQUIRE(stats.mastered == 1);
    }

    QFile::remove( QDir::current().filePath( "data/" + test_db_name ) );
}
