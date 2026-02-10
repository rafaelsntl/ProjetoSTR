#pragma once
#include <string>
#include <mutex>
#include <chrono>
#include <vector>
#include <map>

struct RegistroTempo {
    std::chrono::steady_clock::time_point pronto;
    double latencia = 0;
};

class Monitor {
private:
    std::mutex mtx;
    std::map<int, RegistroTempo> metrics_map;
    std::vector<double> historico_latencias;
    std::chrono::steady_clock::time_point start_time;

public:
    int buffer_atual = 0;
    std::string m1_status = "IDLE", m2_status = "IDLE", robo_status = "NEUTRO";

    Monitor();
    void registrar_pronto(int id);
    void registrar_coleta(int id);
    void atualizar_maquina(int id, std::string status);
    void atualizar_robo(std::string status);
    void desenhar();
};

extern Monitor g_monitor;