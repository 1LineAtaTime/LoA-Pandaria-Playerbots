/* Licensed under the GNU General Public License, version 2 or later. */
#include "DatabaseServerVersion.h"
#include <cstdlib>
#include <iostream>

int main()
{
    using namespace DatabaseServer;
    struct Case { char const* Text; char const* Comment; Family Product; unsigned Number; bool Supported; };
    Case const cases[] = {
        { "5.5.62", "MySQL Community Server (GPL)", Family::MySQL, 50562, true },
        { "5.5.2", "MySQL Community Server", Family::MySQL, 50502, false },
        { "5.5.3-log", "Source distribution", Family::MySQL, 50503, true },
        { "5.7.44-log", "MySQL Community Server", Family::MySQL, 50744, true },
        { "8.0.44", "MySQL Community Server", Family::MySQL, 80044, true },
        { "5.5.5-10.5.29-MariaDB", "mariadb.org binary distribution", Family::MariaDB, 100529, true },
        { "10.5.29-MariaDB-0+deb11u1", "Debian", Family::MariaDB, 100529, true },
        { "5.5.5-10.5.20", "MariaDB Server", Family::MariaDB, 100520, true },
        { "10.5.20", "mArIaDb server", Family::MariaDB, 100520, true },
        { "10.2.8-MariaDB", "MariaDB Server", Family::MariaDB, 100208, false },
        { "10.2.9-MariaDB", "MariaDB Server", Family::MariaDB, 100209, true },
        { "11.4.8-MariaDB", "MariaDB Server", Family::MariaDB, 110408, true },
        { "5.5.5", "MySQL Server", Family::MySQL, 50505, true }
    };
    unsigned failures = 0;
    for (Case const& test : cases)
    {
        auto version = ParseVersion(test.Text, test.Comment);
        if (!version || version->Product != test.Product || version->Number != test.Number || version->IsSupported() != test.Supported)
        {
            std::cerr << "FAIL: " << test.Text << '\n';
            ++failures;
        }
    }
    char const* invalid[] = { "", "garbage", "10", "10.5", "10.5.", ".5.20", "0.0.0", "10.100.0", "10.5.100", "99999999999999999999.1.1", "10.5.20junk", "5.5.5-broken-MariaDB", "-10.5.20" };
    for (char const* text : invalid)
    {
        if (ParseVersion(text, "MariaDB"))
        {
            std::cerr << "FAIL: accepted malformed version " << text << '\n';
            ++failures;
        }
    }
    if (failures)
        return EXIT_FAILURE;
    std::cout << "PASS: " << std::size(cases) + std::size(invalid) << " database version/family cases\n";
    return EXIT_SUCCESS;
}
