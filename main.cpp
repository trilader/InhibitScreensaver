// SPDX-License-Identifier: LGPL-2.1-or-later

#include <cstdlib>
#include <iostream>
#include <sdbus-c++/sdbus-c++.h>
#include "subprocess.h"

bool verbose_output = false;
const char *inhibit_reason = "A game is running";

void debug(const std::string &s)
{
    if (!verbose_output)
        return;
    std::cerr << s;
}

#if defined(SDBUSCPP_15)
    #define SNAME(x) x
    #define OPATH(x) x
#elif defined(SDBUSCPP_20)
    #define SNAME(x) sdbus::ServiceName{x}
    #define OPATH(x) sdbus::ObjectPath{x}
#else
    #define SNAME(x) x
    #define OPATH(x) x
    #error "Unknown SDBusC++ version define?"
#endif

void inhibit_via_inhibit_portal(sdbus::IConnection &dbusConnection)
{
    std::unique_ptr<sdbus::IProxy> desktopPortalProxy = sdbus::createProxy(
        dbusConnection,
        SNAME("org.freedesktop.portal.Desktop"),
        OPATH("/org/freedesktop/portal/desktop")
    );

    try {
        debug("Trying to inhibit idle via org.freedesktop.portal.Inhibit interface: ");
        desktopPortalProxy->callMethod("Inhibit")
            .onInterface("org.freedesktop.portal.Inhibit")
            .withArguments("",  // Window id, if empty default to "parent_window".
                           8u,  // Supported flags: 1: Logout, 2: User Switch, 4: Suspend, 8: Idle
                           std::map<std::string, sdbus::Variant>{
                               {"reason", sdbus::Variant{inhibit_reason}},
                           });
        debug("Ok.\n");
    } catch (const sdbus::Error &e) {
        debug("Failed.\n");
        std::cerr << "Failed to inhibit idle via Freedesktop.org inhibit portal: " << e.getName() << " with message: " << e.getMessage() << std::endl;
    }
}

void inhibit_via_apis(sdbus::IConnection &dbusConnection, const char *application)
{
    std::unique_ptr<sdbus::IProxy> screensaverProxy = sdbus::createProxy(
        dbusConnection,
        SNAME("org.freedesktop.ScreenSaver"),
        OPATH("/org/freedesktop/ScreenSaver")
    );

    try {
        debug("Trying to inhibit screensaver via org.freedesktop.ScreenSaver interface: ");
        screensaverProxy->callMethod("Inhibit")
            .onInterface("org.freedesktop.ScreenSaver")
            .withArguments(inhibit_reason, application);
        debug("Ok.\n");
    } catch (const sdbus::Error &e) {
        debug("Failed.\n");
        std::cerr << "Failed to inhibit screensaver: " << e.getName() << " with message: " << e.getMessage() << std::endl;
    }

    std::unique_ptr<sdbus::IProxy> powerManagementProxy = sdbus::createProxy(
        dbusConnection,
        SNAME("org.freedesktop.PowerManagement.Inhibit"),
        OPATH("/org/freedesktop/PowerManagement/Inhibit")
    );

    try {
        debug("Trying to inhibit power management via org.freedesktop.PowerManagement.Inhibit interface: ");
        powerManagementProxy->callMethod("Inhibit")
            .onInterface("org.freedesktop.PowerManagement.Inhibit")
            .withArguments(inhibit_reason, application);
        debug("Ok.\n");
    } catch (const sdbus::Error &e) {
        debug("Failed.\n");
        std::cerr << "Failed to inhibit power saving: " << e.getName() << " with message: " << e.getMessage() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    // Bail if no command is given.
    if (argc < 2)
        return 0;

    const char *inhibit_debug = std::getenv("INHIBIT_DEBUG");
    if (inhibit_debug != nullptr && strlen(inhibit_debug) != 0)
        verbose_output = true;

    const std::unique_ptr<sdbus::IConnection> sessionBusConnection = sdbus::createSessionBusConnection();

    inhibit_via_inhibit_portal(*sessionBusConnection);
    inhibit_via_apis(*sessionBusConnection, argv[1]);

    // Convert from argc/argv into: An list of arguments for execution and a space-joined string for output.
    std::string subprocess_str;
    std::vector<const char*> args(argc, 0); // Reserve a buffer of argc elements. We only care about argc[1] and later but need a 0 byte as a sentinel at the end, therefore argc is the right count for this.

    for (int i=1; i<argc; i++) {
        subprocess_str.append(argv[i]).append(" "); // Build string for output.
        args[i-1] = argv[i]; // Copy argument from argv into the buffer.
    }
    subprocess_str.pop_back(); // Remove ' ' added by the append after the last entry.

    debug("Starting process '" + subprocess_str + "': ");
    subprocess_s subprocess;
    // Create a subprocess with the same environment we're running with.
    int res = subprocess_create(args.data(),
                                subprocess_option_inherit_environment,
                                &subprocess);
    if (res != 0) {
        debug("Failed.\n");
        std::cerr << "Failed to start '" << subprocess_str << "'. res=" << res << std::endl;
        // Abort if we can't start the process.
        return 1;
    }
    debug("Ok.\n");

    // Wait till the process is done.
    int rc = 0;
    res = subprocess_join(&subprocess, &rc);
    if (res != 0) {
        std::cerr << "Failed to join '" << subprocess_str << "'. res=" << res << "." << std::endl;
    }

    // Print a message if the subprocess didn't exit cleanly.
    if (rc != 0)
        std::cerr << "Process '" << subprocess_str << "' exited with code " << rc << "." << std::endl;
    else
        debug("Process '" + subprocess_str + "' exited with code " + std::to_string(rc) + ".\n");

    // Forward subprocess exit code.
    return rc;
}
