#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <string>
#include <condition_variable>
#include <iomanip>
#include <functional>
#include <Windows.h>

using namespace std;

// --- ESTRUTURAS DE DADOS PARA O DASHBOARD ---
struct CelulaStatus {
    string m1_status = "IDLE";
    string m2_status = "IDLE";
    string robo_status = "NEUTRO";
    int buffer_count = 0;
} status;

mutex mtx_status;

void desenhar_status() {
    lock_guard<mutex> lock(mtx_status);
    // Posiciona o cursor no início (evita o flicker do cls)
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

    cout << "====================================================\n";
    cout << "       PAINEL DE CONTROLE - CELULA DE MANUFATURA    \n";
    cout << "====================================================\n\n";

    cout << "  MAQUINA 1: [" << setw(12) << status.m1_status << "]  ";
    cout << "  MAQUINA 2: [" << setw(12) << status.m2_status << "]\n\n";

    cout << "  ROBO:      [" << setw(20) << status.robo_status << "]\n\n";

    cout << "  ESTEIRA (BUFFER): [";
    for (int i = 0; i < 2; i++) {
        if (i < status.buffer_count) cout << " (X) "; // Peça presente
        else cout << " ( ) "; // Vago
    }
    cout << "] (" << status.buffer_count << "/2)\n\n";
    cout << "====================================================\n";
    cout << " Mensagens recentes:                                \n";
}

// Re-implementação do log para não estragar o desenho
void log_dashboard(string msg) {
    lock_guard<mutex> lock(mtx_status);
    COORD coord = { 0, 12 }; // Pula para a linha das mensagens
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    cout << " > " << setw(50) << left << msg << endl;
}

// --- CLASSES COM ATUALIZACAO DE STATUS ---

class Buffer {
    counting_semaphore<2> slots_vazios{ 2 };
    counting_semaphore<2> pecas_no_buffer{ 0 };
public:
    void reservar_vaga() { slots_vazios.acquire(); }
    void depositar() {
        {
            lock_guard<mutex> lock(mtx_status);
            status.buffer_count++;
        }
        pecas_no_buffer.release();
        desenhar_status();
    }
    void remover() {
        pecas_no_buffer.acquire();
        {
            lock_guard<mutex> lock(mtx_status);
            status.buffer_count--;
        }
        slots_vazios.release();
        desenhar_status();
    }
};

class Maquina {
public:
    int id;
    bool peca_pronta = false;
    mutex mtx_m;
    condition_variable cv_m;

    Maquina(int _id) : id(_id) {}

    void processar() {
        {
            lock_guard<mutex> lock(mtx_status);
            if (id == 1) status.m1_status = "PROCESSANDO";
            else status.m2_status = "PROCESSANDO";
        }
        desenhar_status();
        this_thread::sleep_for(chrono::milliseconds(2000 + (id * 1000)));

        {
            lock_guard<mutex> lock(mtx_status);
            if (id == 1) status.m1_status = "PRONTA";
            else status.m2_status = "PRONTA";
            peca_pronta = true;
        }
        desenhar_status();
    }

    void aguardar_retirada() {
        unique_lock<mutex> lock(mtx_m);
        cv_m.wait(lock, [this] { return !peca_pronta; });
    }

    void sinalizar_retirada() {
        {
            lock_guard<mutex> lock(mtx_m);
            peca_pronta = false;
        }
        cv_m.notify_one();
    }
};

class Robo {
    mutex mtx_r;
public:
    void executar_transporte(Maquina& m, Buffer& b) {
        lock_guard<mutex> lock(mtx_r);

        {
            lock_guard<mutex> lk(mtx_status);
            status.robo_status = "ESPERANDO BUFFER";
        }
        desenhar_status();
        b.reservar_vaga();

        {
            lock_guard<mutex> lk(mtx_status);
            status.robo_status = "INDO PARA M" + to_string(m.id);
        }
        desenhar_status();
        this_thread::sleep_for(chrono::milliseconds(1000));

        m.sinalizar_retirada();

        {
            lock_guard<mutex> lk(mtx_status);
            status.robo_status = "LEVANDO PARA BUFFER";
        }
        desenhar_status();
        this_thread::sleep_for(chrono::milliseconds(1000));

        b.depositar();
        {
            lock_guard<mutex> lk(mtx_status);
            status.robo_status = "NEUTRO";
        }
        desenhar_status();
    }
};

// --- TASKS ---

void task_maquina(Maquina& m, Robo& r, Buffer& b) {
    while (true) {
        m.processar();
        r.executar_transporte(m, b);
        m.aguardar_retirada();
    }
}

void task_externa(Buffer& b) {
    while (true) {
        this_thread::sleep_for(chrono::seconds(8));
        b.remover();
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    // Limpa a tela inicial
    system("cls");

    Buffer esteira;
    Robo robo;
    Maquina m1(1), m2(2);

    thread tm1(task_maquina, ref(m1), ref(robo), ref(esteira));
    thread tm2(task_maquina, ref(m2), ref(robo), ref(esteira));
    thread text(task_externa, ref(esteira));

    tm1.join(); tm2.join(); text.join();
    return 0;
}