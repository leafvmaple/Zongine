#include "Utilities/Engine.h"

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    Zongine::Engine engine;
    engine.Initialize(hInstance);

    while (true) {
        engine.Run();
    }
}