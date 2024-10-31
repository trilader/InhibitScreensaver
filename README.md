Inhibit Screensaver
===

A simple utility to launch another process while inhibiting Freedesktop.org compliant power managers and screensavers.

# About

## Why is this a thing?

I've recently (at the end of 2023) switched to running Steam on Linux inside Flatpak.
Playing games with a keyboard and mouse works great. Playing games with a controller also works fine, except there is one issue:
Currently Steam unfortunately doesn't manage to tell my desktop environment (KDE) to inhibit power management (going to sleep) or locking the screen and/or starting the screensaver when playing with a controller.

Whatever mechanism is used to detect user activity doesn't care about my controller.

## Why not use some-other-tool?

There is a convenient tool that ships with KDE: `kde-inhibit` which I wanted to use. However as stated earlier, I'm running Steam inside Flatpak and don't have access to it. I tried to get flatpak to allow access to just that binary but couldn't get it to work.

# Installation

## Standalone
If you just want to use this tool as-is on your machine you'll need:

- A normal C/C++ build environment (compiler, linker, etc)
- CMake
- sdbus-cpp (I've tested this with version 1.5.0 and 2.0.0, which you can find [here](https://github.com/Kistler-Group/sdbus-cpp) or likely in your Linux distribution packages)

This is a normal CMake based application, so you can build it as usual.

### A note for non Steam Flatpak users:

If you're using this inside Flatpak but outside the Steam Flatpak package (e.g., inside bottles) you'll need to ensure that access to the session DBus connection as well as "talk" permissions on `org.freedesktop.PowerManagement` and `org.freedesktop.ScreenSaver` is allowed.
Otherwise the package won't work (and most likely crash). You can grant such access using e.g. Flatseal or the "Flatpak Permissions" from KDE.

## As an extension to the Steam Flatpak package:
This utility is available on Flathub with the id `com.valvesoftware.Steam.Utility.InhibitScreensaver`.

You can install it as an extension to the Steam package either by running `flatpak install flathub com.valvesoftware.Steam.Utility.InhibitScreensaver` in your favorite terminal application or through a GUI if it supports installing extension packages.
