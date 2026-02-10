#include <Windows.h>
#include <thread>
#include "Entidades.h"
#include "Monitor.h"
#include "Constants.h"

void task_maquina(Maquina& m, Robo& r, Buffer& b) {
    while (true) {
        m.processar();
        r.executar_transporte(m, b);
        m.aguardar_retirada();
    }
}

void task_externa(Buffer& b) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_EXTERNAL_CONSUMER));
        b.remover();
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    system("cls");

    Buffer esteira;
    Robo robo;
    Maquina m1(1), m2(2);

    std::thread tm1(task_maquina, std::ref(m1), std::ref(robo), std::ref(esteira));
    std::thread tm2(task_maquina, std::ref(m2), std::ref(robo), std::ref(esteira));
    std::thread text(task_externa, std::ref(esteira));

    tm1.join(); tm2.join(); text.join();
    return 0;
}