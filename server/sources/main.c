#include "server.h"

int main(int ac, char **av)
{
    if (server() != 0)
        return 84;
    return 0;
}