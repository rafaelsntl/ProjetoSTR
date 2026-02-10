#include "Monitor.h"
#include <iostream>
#include <Windows.h>
#include <iomanip>

Monitor g_monitor;

Monitor::Monitor() {
    start_time = std::chrono::steady_clock::now();
}

void Monitor::atualizar_maquina(int id, std::string status) {
    std::lock_guard<std::mutex> lock(mtx);
    if (id == 1) m1_status = status; else m2_status = status;
}

void Monitor::atualizar_robo(std::string status) {
    std::lock_guard<std::mutex> lock(mtx);
    robo_status = status;
}

void Monitor::registrar_pronto(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    metrics_map[id].pronto = std::chrono::steady_clock::now();
}

void Monitor::registrar_coleta(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    auto agora = std::chrono::steady_clock::now();
    auto duracao = std::chrono::duration_cast<std::chrono::milliseconds>(agora - metrics_map[id].pronto);
    double latencia = (double)duracao.count();
    historico_latencias.push_back(latencia);
    metrics_map[id].latencia = latencia;
}

void Monitor::desenhar() {
    std::lock_guard<std::mutex> lock(mtx);
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

    auto agora = std::chrono::steady_clock::now();
    auto up = std::chrono::duration_cast<std::chrono::seconds>(agora - start_time).count();

    std::cout << "====================================================\n";
    std::cout << "   SISTEMA DE MANUFATURA STR - UPTIME: " << up << "s\n";
    std::cout << "====================================================\n";
    std::cout << " M1: [" << std::setw(12) << m1_status << "] | M2: [" << std::setw(12) << m2_status << "]\n";
    std::cout << " ROBO: [" << std::setw(20) << robo_status << "]\n";
    std::cout << " ESTEIRA: [";
    for (int i = 0; i < 2; i++) std::cout << (i < buffer_atual ? " (X) " : " ( ) ");
    std::cout << "] (" << buffer_atual << "/2)\n";
    std::cout << "----------------------------------------------------\n";
    if (!historico_latencias.empty())
        std::cout << " ULTIMA LATENCIA DE COLETA: " << historico_latencias.back() << " ms\n";
    std::cout << "====================================================\n";
}