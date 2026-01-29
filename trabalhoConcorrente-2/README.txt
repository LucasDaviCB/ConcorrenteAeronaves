Dupla:

João Pedro Theodoro (21200077)

Lucas Davi Cascaes Brena (21203362)

Disciplina: Programação Concorrente

Professor: 

Giovani Gracioli

Márcio Castro

Trabalho: Simulador Concorrente de Controle de Tráfego Aéreo

Descrição

Simulador onde múltiplas aeronaves percorrem rotas independentes e competem por acesso a setores do espaço aéreo.
O controle é centralizado, com filas de prioridade e sincronização via mutexes e variáveis de condição.
O sistema garante exclusão mútua correta, ausência de deadlocks e respeito à prioridade das aeronaves.

***Como Compilar***

No diretório do projeto:

make


Gera o executável:

atc

Como Executar
./atc <num_setores> <num_aeronaves>


Exemplo:

./atc 10 5

Limpar Arquivos de Compilação:

make clean
