#ifndef GuessNumClient_H
#define GuessNumClient_H

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QList>
#include <QLabel>
#include <QTimer>
#include <QLineEdit>
#include <QThread>
#include <QMessageBox>
#include "request.h"
#include "config.h"
#include "main.model.pb.h"


namespace Ui {
class GuessNumClient;
}

class GuessNumClient : public QDialog
{
    Q_OBJECT

public:
    explicit GuessNumClient(QWidget *parent = 0);
    ~GuessNumClient();
    void start();

signals:
    void waitForRival();
    void winGame();
    void loseGame();
    void modify_player_money(int);
    void rival_die();

public slots:
    void cleanUp();
    void reject();

private slots:
    void submitAnswer();
    void update();
    void checkRivalFinish();

private:

    void initialize();
    void setMagicNumber();
    void setUpDisplay();
    void setGuessResult(int resultA, int resultB );
    void you_winGame();
    void you_loseGame();
    void you_waitForRival();


    Ui::GuessNumClient *ui;
    QList<QLabel*> magic_numbers;
    QList<QLineEdit*> guess_numbers;
    QTimer* updator;
    QTimer* waitRivalTimer;
    int round;
};

#endif // GuessNumClient_H
