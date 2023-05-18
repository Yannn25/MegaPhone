/*
** EPITECH PROJECT, 2021
** Untitled (Workspace)
** File description:
** test_test
*/

#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include "client.h"

void redirect_all_std(void)
{
    cr_redirect_stderr();
    cr_redirect_stdout();
}

Test(, test1, .init=redirect_all_std)
{
    cr_assert();
}

Test(, test2, .init=redirect_all_std)
{
    cr_assert();
}

Test(, test3, .init=redirect_all_std)
{
    cr_assert();
}