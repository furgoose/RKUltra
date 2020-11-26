#include <signal.h>

int main()
{
    kill(0, 9000);
    return 0;
};