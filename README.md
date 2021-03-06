# Quasar

HTML widgets for your desktop, powered by Qt WebEngine. Licensed under GPLv3.

![](https://i.imgur.com/NX2SNUD.png)

## Usage

After running the application, right click the application icon in the notification bar to load widgets.

## Widgets

Widgets are just webpages. They can be as simple or as complex as you want, perfect for those with web skills that wants to make their desktop a little fancier. Refer to the wiki for details on how to create a widget definition.

## Data Server

Quasar propagates data to widgets over the WebSocket protocol using a local data server, and it is not just limited to Quasar widgets. The data server can be used in various other scenarios, such as an HTML stream overlay. Refer to the wiki for details on the WebSocket API.

## Plugins

Need more data? Data available to widgets can be extended through plugins. Refer to the wiki for details on the Plugin API.

## System Requirements

An OS and computer capable of running Chrome.

[Microsoft Visual C++ Redistributable for Visual Studio 2017](https://go.microsoft.com/fwlink/?LinkId=746572) is needed on Windows.

## Build

### Windows

[![Build status](https://ci.appveyor.com/api/projects/status/yd5l7u53ufo4mur1?svg=true)](https://ci.appveyor.com/project/r52/quasar)

[Qt 5.9 or higher](http://www.qt.io/) and Visual Studio 2017 is required.

### Linux

[Qt 5.9 or higher](http://www.qt.io/), GCC 7+ or Clang 4+, and CMake 3.9+ is required. Tested on Ubuntu 17.10.

### Mac

Quasar is written in cross-platform C++ and should build on Mac with minimal changes. However, it is currently untested and unsupported.
