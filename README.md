# zpr-learningapp
## Wymagane biblioteki
 - CMake
 - Qt
 - SQLite
 - Catch2
## Sposób kompilacji i uruchamiania programu
W pierwszej kolejności trzeba utworzyć folder build: *"mkdir build"* oraz *"cd build"*

Aby skompilować/uruchomić projekt należy:
1. wywołać komendę *"cmake .."*
2. wywołać komendę *"cmake --build ."*
3. po kompilacji uruchomić testy za pomocą *"ctest"* bądź aplikację za pomocą *"./src/LearningApp"*