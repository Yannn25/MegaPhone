#include "client.h"

int main(int ac, char **av)
{
    if (client() != 0)
        return 84;
    return 0;
}