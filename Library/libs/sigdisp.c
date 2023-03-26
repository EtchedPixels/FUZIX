#include <signal.h>

extern int _sigdisp(int sig, int disp);

int sighold(int sig)
{
    return _sigdisp(sig, 1);
}

int sigrelse(int sig)
{
    return _sigdisp(sig, 0);
}

int sigignore(int sig)
{
    signal(sig, SIG_IGN);
}

