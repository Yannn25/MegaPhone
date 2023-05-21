#include "server.h"

int main(int ac, char **av)
{
    (void)ac;
    (void)av;
    if (server() != 0)
        return 84;
    return 0;
}