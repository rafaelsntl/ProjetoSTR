#include "Entidades.h"
#include "Monitor.h"
#include "Constants.h"
#include <thread>

// --- MAQUINA ---
Maquina::Maquina(int _id) : id(_id) {}

void Maquina::processar() {
    g_monitor.atualizar_maquina(id, "PROCESSANDO");
    g_monitor.desenhar();

    // Simula o tempo de processamento definido nas constantes
    std::this_thread::sleep_for(std::chrono::milliseconds(id == 1 ? TIME_M1_PROC : TIME_M2_PROC));

    {
        std::lock_guard<std::mutex> lock(mtx_m);
        peca_pronta = true;
    }

    g_monitor.registrar_pronto(id);
    g_monitor.atualizar_maquina(id, "PRONTA");
    g_monitor.desenhar();
}

void Maquina::aguardar_retirada() {
    std::unique_lock<std::mutex> lock(mtx_m);
    cv_m.wait(lock, [this] { return !peca_pronta; });
}

void Maquina::sinalizar_retirada() {
    {
        std::lock_guard<std::mutex> lock(mtx_m);
        peca_pronta = false;
    }
    cv_m.notify_one();
}

// --- ROBO ---
void Robo::executar_transporte(Maquina& m, Buffer& b) {
    // Garante que apenas uma thread de máquina use o robô por vez
    std::lock_guard<std::mutex> lock(mtx_r);

    g_monitor.atualizar_robo("ESPERANDO VAGA");
    g_monitor.desenhar();

    // PREVENÇÃO DE DEADLOCK: Reserva o buffer antes de se mover
    b.reservar_vaga();

    g_monitor.atualizar_robo("INDO PARA M" + std::to_string(m.id));
    g_monitor.desenhar();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_ROBOT_MOVE));

    // Coleta a peça e registra métrica de latência
    g_monitor.registrar_coleta(m.id);
    m.sinalizar_retirada(); // Libera a máquina para o próximo ciclo

    g_monitor.atualizar_robo("LEVANDO AO BUFFER");
    g_monitor.desenhar();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_ROBOT_MOVE));

    b.depositar();

    g_monitor.atualizar_robo("NEUTRO");
    g_monitor.desenhar();
}

// --- BUFFER ---
void Buffer::reservar_vaga() {
    slots_vazios.acquire();
}

void Buffer::depositar() {
    {
        // Protege a variável global de contagem para a interface
        std::lock_guard<std::mutex> lock(mtx_buffer);
        g_monitor.buffer_atual++;
    }
    pecas_no_buffer.release();
}

void Buffer::remover() {
    pecas_no_buffer.acquire();
    {
        std::lock_guard<std::mutex> lock(mtx_buffer);
        g_monitor.buffer_atual--;
    }
    slots_vazios.release();
    g_monitor.desenhar();
}