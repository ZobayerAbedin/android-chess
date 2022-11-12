#include "../common.h"

#include <unistd.h>
#include <functional>

#include "../ChessBoard.h"
#include "../Game.h"
#include "../Move.h"
#include "chess-test.h"

void miniTest();
void startThread();
bool testGame();
void speedTest();
bool testDB();
bool testSetupNewGame();
bool testSetupMate();
bool testInCheck();
bool testInSelfCheck();
bool testSetupCastle();
bool testSetupQuiesce();
bool testHouse();
bool testMoves();
bool testGenmoves();
bool testDuck();
bool testEngine();
bool testSequence();
void newGame();
void newGameDuck();
void printFENAndState(ChessBoard *board);
void printMove(int move);

using std::function;

int main(int argc, char **argv) {
    DEBUG_PRINT("\n\n=== START TESTS == \n", 0);

    TestFunction tests[] = {testSetupNewGame,
                            testSetupMate,
                            testInCheck,
                            testInSelfCheck,
                            testSetupCastle,
                            testSetupQuiesce,
                            testMoves,
                            testDB,
                            testDuck,
                            testEngine,
                            testSequence};

    // TestFunction tests[] = {
    //     // testGame,
    //     testDuckGame,
    // };

    int testFail = 0, testSuccess = 0;
    for (int i = 0; i < sizeof(tests) / sizeof(TestFunction); i++) {
        Game::deleteInstance();
        
        if (tests[i]()) {
            testSuccess++;
        } else {
            testFail++;
        }
    }

    DEBUG_PRINT("\n\n=== DONE === SUCCESS: [%d] FAIL: [%d]\n", testSuccess, testFail);
}

bool testSetupNewGame() {
    newGame();
    char buf[255];

    ChessBoard *board = Game::getInstance()->getBoard();
    board->toFEN(buf);

    return ChessTest::expectEqualString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", buf, "testSetupNewGame");
}

bool testInCheck() {
    ChessBoard *board = Game::getInstance()->getBoard();

    board->reset();
    board->put(ChessBoard::f8, ChessBoard::KING, ChessBoard::BLACK);
    board->put(ChessBoard::b1, ChessBoard::KING, ChessBoard::WHITE);
    board->put(ChessBoard::a8, ChessBoard::ROOK, ChessBoard::WHITE);
    board->setCastlingsEPAnd50(0, 0, 0, 0, -1, 0);
    board->setTurn(0);
    Game::getInstance()->commitBoard();

    // bool b = board->checkInCheck();
    // if (b) {
    //     DEBUG_PRINT("in check!\n", 0);
    // }

    return ChessTest::expectEqualInt(Game::getInstance()->getBoard()->getState(), ChessBoard::CHECK, "State should equal CHECK");
}

bool testInSelfCheck() {
    return true;
}

bool testSetupMate() {
    ChessBoard *board = Game::getInstance()->getBoard();

    board->reset();
    board->put(ChessBoard::f8, ChessBoard::KING, ChessBoard::BLACK);
    board->put(ChessBoard::f6, ChessBoard::KING, ChessBoard::WHITE);
    board->put(ChessBoard::a8, ChessBoard::ROOK, ChessBoard::WHITE);
    board->setCastlingsEPAnd50(0, 0, 0, 0, -1, 0);
    board->setTurn(0);
    Game::getInstance()->commitBoard();

    return ChessTest::expectEqualInt(Game::getInstance()->getBoard()->getState(), ChessBoard::MATE, "State should equal MATE");
}


bool testDB() {
    Game::getInstance()->loadDB("db.bin", 3);

    newGame();

    int move = Game::getInstance()->searchDB();
    printMove(move);

    Game::getInstance()->move(move);

    move = Game::getInstance()->searchDB();
    printMove(move);

    return true;
}

bool testSetupCastle() {
    ChessBoard *board = Game::getInstance()->getBoard();

    board->put(ChessBoard::c8, ChessBoard::KING, ChessBoard::BLACK);
    board->put(ChessBoard::a8, ChessBoard::ROOK, ChessBoard::BLACK);
    // board->put(ChessBoard::b8, ChessBoard::ROOK, ChessBoard::BLACK);

    board->put(ChessBoard::g1, ChessBoard::ROOK, ChessBoard::WHITE);
    board->put(ChessBoard::f1, ChessBoard::KING, ChessBoard::WHITE);

    board->setCastlingsEPAnd50(1, 1, 1, 1, -1, 0);
    // board->setTurn(0);
    Game::getInstance()->commitBoard();

    char buf[512];
    board->toFEN(buf);

    bool ret = ChessTest::expectEqualString("r1k5/8/8/8/8/8/8/5KR1 w KQkq - 0 1", buf, "testSetupCastle");

    return ret;
}

bool testSetupQuiesce() {
    ChessBoard *board = Game::getInstance()->getBoard();

    board->put(ChessBoard::d7, ChessBoard::PAWN, ChessBoard::BLACK);
    board->put(ChessBoard::f8, ChessBoard::KING, ChessBoard::BLACK);
    board->put(ChessBoard::c6, ChessBoard::PAWN, ChessBoard::BLACK);
    board->put(ChessBoard::e7, ChessBoard::QUEEN, ChessBoard::BLACK);

    board->put(ChessBoard::b4, ChessBoard::BISHOP, ChessBoard::WHITE);
    board->put(ChessBoard::c3, ChessBoard::PAWN, ChessBoard::WHITE);
    board->put(ChessBoard::d4, ChessBoard::PAWN, ChessBoard::WHITE);
    board->put(ChessBoard::e5, ChessBoard::PAWN, ChessBoard::WHITE);
    board->put(ChessBoard::d3, ChessBoard::KING, ChessBoard::WHITE);

    board->setCastlingsEPAnd50(0, 0, 0, 0, -1, 0);
    board->setTurn(0);
    Game::getInstance()->commitBoard();

    // printFENAndState(board);
    return true;
}

bool testHouse() {
    ChessBoard *board = Game::getInstance()->getBoard();

    printFENAndState(board);

    if (Game::getInstance()->putPieceHouse(ChessBoard::e2, ChessBoard::KNIGHT, false)) {
        DEBUG_PRINT("PUT HOUSE\n", 0);
    }

    // printFENAndState(board);
    return true;
}

bool testMoves() {
    newGame();

    bool ret = Game::getInstance()->requestMove(ChessBoard::e2, ChessBoard::e4) && Game::getInstance()->requestMove(ChessBoard::e7, ChessBoard::e5) &&
               Game::getInstance()->requestMove(ChessBoard::g1, ChessBoard::f3) && Game::getInstance()->requestMove(ChessBoard::b8, ChessBoard::c6) &&
               Game::getInstance()->requestMove(ChessBoard::d1, ChessBoard::e2) && Game::getInstance()->requestMove(ChessBoard::f8, ChessBoard::e7);

    if (!ret) {
        DEBUG_PRINT("testMoves failed\n", 0);
    }

    return ret;
}

bool testGenmoves() {
    ChessBoard *board = Game::getInstance()->getBoard();
    board->hasMoreMoves();
    board->getNextMove();
    board->getNumMoves();

    return true;
}

bool testDuck() {
    char buf[512];
    ChessBoard *board;
    bool ret;

    newGameDuck();

    board = Game::getInstance()->getBoard();
    board->toFEN(buf);
    ret = ChessTest::expectEqualString("rnbqkbnr/pppppppp/8/7$/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", buf, "testDuck");
    if (!ret) {
        return false;
    }

    ret = Game::getInstance()->requestMove(ChessBoard::e2, ChessBoard::e4);
    ret = Game::getInstance()->requestDuckMove(ChessBoard::e6);
    if (!ret) {
        DEBUG_PRINT("no request duck move 1", 0);
        return false;
    }
    ret = Game::getInstance()->requestMove(ChessBoard::e7, ChessBoard::e5);
    if (ret) {
        DEBUG_PRINT("move e7-e5 should not", 0);
        return false;
    }

    ret = Game::getInstance()->requestMove(ChessBoard::d7, ChessBoard::e6);
    if (ret) {
        DEBUG_PRINT("move d7-e6 should not", 0);
        return false;
    }

    board = Game::getInstance()->getBoard();
    board->toFEN(buf);
    ret = ChessTest::expectEqualString("rnbqkbnr/pppppppp/4$3/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", buf, "testDuck");
    if (!ret) {
        return false;
    }

    ret = ChessTest::expectEqualInt(board->getDuckPos(), ChessBoard::e6, "testDuck");

    // board->printB(buf);
    // DEBUG_PRINT("\n%s\n", buf);
    ret = Game::getInstance()->requestMove(ChessBoard::d7, ChessBoard::d5);
    if (!ret) {
        DEBUG_PRINT("no d7-d5", 0);
        return false;
    }

    ret = Game::getInstance()->requestDuckMove(ChessBoard::e3);
    if (!ret) {
        DEBUG_PRINT("no duck e3", 0);
        return false;
    }

    ret = Game::getInstance()->requestMove(ChessBoard::f1, ChessBoard::b5);
    if (!ret) {
        DEBUG_PRINT("no f1-b5", 0);
        return false;
    }

    ret = Game::getInstance()->requestDuckMove(ChessBoard::f3);
    if (!ret) {
        DEBUG_PRINT("no duck f3", 0);
        return false;
    }

    ret = Game::getInstance()->requestMove(ChessBoard::f7, ChessBoard::f6);
    if (!ret) {
        DEBUG_PRINT("no f7-f6", 0);
        return false;
    }

    ret = Game::getInstance()->requestDuckMove(ChessBoard::f4);
    if (!ret) {
        DEBUG_PRINT("no duck f4 x", 0);
        return false;
    }

    ret = Game::getInstance()->requestMove(ChessBoard::b5, ChessBoard::e8);
    if (!ret) {
        DEBUG_PRINT("no b5-e8", 0);
        return false;
    }

    ret = ChessTest::expectEqualInt(ChessBoard::MATE, Game::getInstance()->getBoard()->getState(), "State mate");

    return ret;
}

bool testEngine() {
    EngineInOutFEN scenarios[3] = {
        {Game::getInstance(), "8/8/8/8/8/r2k4/8/3K4 b - - 0 1", "8/8/8/8/8/3k4/8/r2K4 w - - 1 1", 1, 1, "Mate in one"},
        {Game::getInstance(), "r6k/6pp/8/8/8/8/1R6/1R1K4 w - - 0 1", "rR5k/6pp/8/8/8/8/8/1R1K4 b - - 1 1", 2, 1, "Mate in two"},
        {Game::getInstance(), "2Q5/5pk1/8/8/1b6/1b6/r3n1P1/2K5 w - - 0 1", "2Q5/5pk1/8/8/1b6/1b6/r3n1P1/1K6 b - - 1 1", 2, 1, "In check"}};

    bool bRet = true;

    for (int i = 0; i < 3; i++) {
        if (!ChessTest::expectEngineMove(scenarios[i])) {
            bRet = false;
        }
    }

    return bRet;
}

bool testSequence() {
    SequenceInOutFEN scenarios[2] = {
        {Game::getInstance(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "r1bqkbnr/pppp1ppp/2n5/8/3pP3/5N2/PPP2PPP/RNBQKB1R w KQkq - 0 4",
         (int[]){Move_makeMoveFirstPawn(ChessBoard::e2, ChessBoard::e4),
                 Move_makeMoveFirstPawn(ChessBoard::e7, ChessBoard::e5),
                 Move_makeMove(ChessBoard::g1, ChessBoard::f3),
                 Move_makeMove(ChessBoard::b8, ChessBoard::c6),
                 Move_makeMoveFirstPawn(ChessBoard::d2, ChessBoard::d4),
                 Move_makeMoveHit(ChessBoard::e5, ChessBoard::d4)},
         6,
         "Opening"},
        {Game::getInstance(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "rnb1kbnr/pppp1ppp/4p3/8/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",
         (int[]){Move_makeMove(ChessBoard::f2, ChessBoard::f3),
                 Move_makeMove(ChessBoard::e7, ChessBoard::e6),
                 Move_makeMoveFirstPawn(ChessBoard::g2, ChessBoard::g4),
                 Move_setCheck(Move_makeMove(ChessBoard::d8, ChessBoard::h4))},
         4,
         "To mate"},
    };

    bool bRet = true;

    for (int i = 0; i < 2; i++) {
        if (!ChessTest::expectSequence(scenarios[i])) {
            bRet = false;
        }
    }

    return bRet;
}

void speedTest() {
    ChessBoard *board = Game::getInstance()->getBoard();

    newGame();

    printFENAndState(board);

    // sprintf(s, "State %d = %d = %d\n", Game::getInstance()->getBoard()->getState(), Game::getInstance()->getBoard()->isEnded(),
    // Game::getInstance()->getBoard()->getNumMoves()); DEBUG_PRINT(s);

    Game::getInstance()->setSearchTime(10);
    Game::getInstance()->search();
}

void newGame() {
    Game::getInstance()->newGameFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void newGameDuck() {
    Game::getInstance()->newGameFromFEN("rnbqkbnr/pppppppp/8/7$/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void printMove(int move) {
    char buf[10];
    Move::toDbgString(move, buf);
    DEBUG_PRINT("Move %s\n", buf);
}

void printFENAndState(ChessBoard *board) {
    char buf[512];
    board->toFEN(buf);
    DEBUG_PRINT("\nFEN\t%s\n", buf);

    int state = board->getState();

    switch (state) {
        case ChessBoard::PLAY:
            DEBUG_PRINT("Play\n", 0);
            break;

        case ChessBoard::CHECK:
            DEBUG_PRINT("Check\n", 0);
            break;

        case ChessBoard::INVALID:
            DEBUG_PRINT("Invalid\n", 0);
            break;

        case ChessBoard::DRAW_MATERIAL:
            DEBUG_PRINT("Draw material\n", 0);
            break;

        case ChessBoard::DRAW_50:
            DEBUG_PRINT("Draw 50 move\n", 0);
            break;

        case ChessBoard::MATE:
            DEBUG_PRINT("Mate\n", 0);
            break;

        case ChessBoard::STALEMATE:
            DEBUG_PRINT("Stalemate\n", 0);
            break;

        case ChessBoard::DRAW_REPEAT:
            DEBUG_PRINT("Draw repetition\n", 0);
            break;

        default:
            break;
    }
}
