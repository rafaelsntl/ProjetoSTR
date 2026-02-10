#pragma once
#include <mutex>
#include <condition_variable>
#include <semaphore>

class Buffer {
public:
    std::counting_semaphore<2> slots_vazios{ 2 };
    std::counting_semaphore<2> pecas_no_buffer{ 0 };
    std::mutex mtx_buffer;

    void reservar_vaga();
    void depositar();
    void remover();
};

class Maquina {
public:
    int id;
    bool peca_pronta = false;
    std::mutex mtx_m;
    std::condition_variable cv_m;

    Maquina(int _id);
    void processar();
    void aguardar_retirada();
    void sinalizar_retirada();
};

class Robo {
    std::mutex mtx_r;
public:
    void executar_transporte(Maquina& m, Buffer& b);
};