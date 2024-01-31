// SPDX-License-Identifier: LGPL-2.1-or-later

#include <iostream>
#include <sdbus-c++/sdbus-c++.h>
#include "subprocess.h"

const char *inhibit_reason = "A game is running";

void inhibit_via_apis(sdbus::IConnection &dbusConnection, const char *application)
{
    std::unique_ptr<sdbus::IProxy> screensaverProxy = sdbus::createProxy(
        dbusConnection,
        "org.freedesktop.ScreenSaver",
        "/org/freedesktop/ScreenSaver"
    );

    std::unique_ptr<sdbus::IProxy> powerManagementProxy = sdbus::createProxy(
        dbusConnection,
        "org.freedesktop.PowerManagement.Inhibit",
        "/org/freedesktop/PowerManagement/Inhibit"
    );

    try {
        screensaverProxy->callMethod("Inhibit")
            .onInterface("org.freedesktop.ScreenSaver")
            .withArguments(inhibit_reason, application);
    } catch (const sdbus::Error &e) {
        std::cerr << "Failed to inhibit screensaver: " << e.getName() << " with message: " << e.getMessage() << std::endl;
    }

    try {
        powerManagementProxy->callMethod("Inhibit")
            .onInterface("org.freedesktop.PowerManagement.Inhibit")
            .withArguments(inhibit_reason, application);
    } catch (const sdbus::Error &e) {
        std::cerr << "Failed to inhibit power saving: " << e.getName() << " with message: " << e.getMessage() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    // Bail if no command is given.
    if (argc < 2)
        return 0;

    const std::unique_ptr<sdbus::IConnection> sessionBusConnection = sdbus::createSessionBusConnection();

    inhibit_via_apis(*sessionBusConnection, argv[1]);

    // Convert from argc/argv into: An list of arguments for execution and a space-joined string for output.
    std::string subprocess_str;
    std::vector<const char*> args(argc, 0); // Reserve a buffer of argc elements. We only care about argc[1] and later but need a 0 byte as a sentinel at the end, therefore argc is the right count for this.

    for (int i=1; i<argc; i++) {
        subprocess_str.append(argv[i]).append(" "); // Build string for output.
        args[i-1] = argv[i]; // Copy argument from argv into the buffer.
    }
    subprocess_str.pop_back(); // Remove ' ' added by the append after the last entry.

    subprocess_s subprocess;
    // Create a subprocess with the same environment we're running with.
    int res = subprocess_create(args.data(),
                                subprocess_option_inherit_environment,
                                &subprocess);
    if (res != 0) {
        std::cerr << "Failed to spawn '" << subprocess_str << "'. res=" << res << std::endl;
        // Abort if we can't spawn the process.
        return 1;
    }

    // Wait till the process is done.
    int rc = 0;
    res = subprocess_join(&subprocess, &rc);
    if (res != 0) {
        std::cerr << "Failed to join '" << subprocess_str << "'. res=" << res << std::endl;
    }

    // Print a message if the subprocess didn't exit cleanly.
    if (rc != 0)
        std::cerr << "Process '" << subprocess_str << "' exited with code " << rc << "." << std::endl;

    // Forward subprocess exit code.
    return rc;
}
