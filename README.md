# Célula de Manufatura Automatizada - Controle Concorrente

Este projeto implementa o controle de uma célula de manufatura automatizada utilizando programação multithread em C++20. O sistema coordena o processamento de peças em duas estações independentes, um robô de transporte compartilhado e um buffer de saída com capacidade limitada.

## 1. Arquitetura do Sistema

O software foi desenvolvido seguindo uma arquitetura modular para separar as preocupações de controle, interface e monitoramento:

- **Constants.h**: Definição de parâmetros estáticos do sistema (capacidades, tempos de processamento e deslocamento).
- **Monitor.h / Monitor.cpp**: Gerenciador de estado global e Interface Homem-Máquina (IHM). Responsável pela coleta de métricas de desempenho.
- **Entidades.h / Entidades.cpp**: Implementação das classes `Maquina`, `Robo` e `Buffer`, contendo a lógica de controle e sincronização.
- **main.cpp**: Orquestrador das threads de execução (Máquinas, Consumidor Externo e Robô).

## 2. Mecanismos de Sincronização e Controle

Para atender aos requisitos de exclusão mútua e evitar condições de corrida, foram aplicadas as seguintes primitivas:

### Exclusão Mútua (Mutexes)
O robô é tratado como um recurso crítico compartilhado. O uso de `std::mutex` garante que apenas uma tarefa de transporte seja executada por vez, evitando colisões de acesso entre as solicitações da Máquina 1 e da Máquina 2.

### Controle de Fluxo (Semáforos)
O buffer de saída é gerenciado por `std::counting_semaphore`.

- O semáforo de **slots** garante que o robô não deposite peças se a capacidade máxima (N = 2) for atingida.
- O semáforo de **peças** sinaliza ao agente externo a existência de itens para remoção.

### Sincronização de Eventos (Variáveis de Condição)
A comunicação entre o fim do processamento de uma máquina e a autorização para o reinício do ciclo ocorre via `std::condition_variable`, garantindo que uma máquina permaneça ociosa até que o robô efetue a coleta da peça pronta.

## 3. Prevenção de Falhas Críticas

O projeto foi desenhado para mitigar problemas clássicos de sistemas concorrentes:

### Prevenção de Deadlock
Implementou-se uma estratégia de reserva antecipada. O robô só inicia o movimento de coleta após adquirir o semáforo de vaga no buffer. Isso evita o cenário onde o robô retira uma peça e fica bloqueado aguardando espaço para depósito, o que paralisaria as máquinas.

### Prevenção de Overflow
O controle estrito via semáforos impede o depósito de peças além da capacidade nominal da esteira.

### Sequenciamento Seguro
A lógica impede que o robô tente coletar uma peça de uma máquina que ainda está em processo de transformação.

## 4. Análise de Tempo Real (STR)

### WCET (Worst-Case Execution Time)
O tempo de execução de pior caso para a tarefa de transporte é calculado pela soma dos tempos de deslocamento e overhead de sincronização:

$WCET_{Robot} = T_{ida} + T_{volta} + T_{sync} \approx 1605\text{ ms}$


### Latência e Jitter
O sistema monitora a latência de coleta (intervalo entre o estado "Peça Pronta" e a coleta efetiva). A variação observada nesta métrica (jitter) é diretamente proporcional ao tempo de bloqueio imposto pela ocupação do buffer, demonstrando a dependência do sistema em relação à taxa de consumo externa.

## 5. Instruções de Compilação

O projeto utiliza recursos do padrão C++20 (como semáforos nativos).

- Utilizar **Visual Studio 2022** ou superior.
- Configurar o padrão de linguagem C++ para **ISO C++20 Standard** (`/std:c++20`).

O executável gera um painel de monitoramento no console via `SetConsoleCursorPosition` para evitar oscilações de imagem.

## 6. Demonstração

[Link para o vídeo no YouTube: https://www.youtube.com/watch?v=vO-0ybE2O1U]
