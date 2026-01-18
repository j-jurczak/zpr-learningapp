# zpr-learningapp
## Wymagane biblioteki
 - CMake
 - Qt (w tym Qt::Multimedia)
 - SQLite
 - Catch2
## Sposób kompilacji i uruchamiania programu
W pierwszej kolejności trzeba utworzyć folder build: *"mkdir build"* oraz *"cd build"*

Aby skompilować/uruchomić projekt należy:
1. wywołać komendę *"cmake .."*
2. wywołać komendę *"cmake --build ."*
3. po kompilacji uruchomić testy za pomocą *"ctest"* bądź aplikację za pomocą *"./src/LearningApp"*

## Skrypt instalujący potrzebne pakiety na Ubuntu 24
```
#!/bin/bash
set -e
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qt6-base-dev libqt6sql6-sqlite
sudo apt install -y catch2 
sudo apt install -y qt6-multimedia-dev
sudo apt install -y gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav
sudo apt install -y gstreamer1.0-pulseaudio gstreamer1.0-alsa
sudo apt install -y zip
```