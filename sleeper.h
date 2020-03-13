#ifndef SLEEPER_H
#define SLEEPER_H
#include <QThread>

class Sleeper : public QThread
{
public:
    Sleeper();
    static void sleep(ulong secs){QThread::sleep(secs);}

};

#endif // SLEEPER_H
