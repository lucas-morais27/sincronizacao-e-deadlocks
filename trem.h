#ifndef TREM_H
#define TREM_H

#include <QThread>

/*
 * Classe Trem herda QThread
 * Classe Trem passa a ser uma thread.
 * A função START inicializa a thread. Após inicializada, a thread irá executar a função RUN.
 * Para parar a execução da função RUN da thread, basta executar a função TERMINATE.
 *
*/
class Trem: public QThread {
    Q_OBJECT
public:
    Trem(int,int,int);  //construtor
    void run();         //função a ser executada pela thread
    void set_velocidade(int velocidade);
    int areas[6]; //controlar os estados.
    int area;
    

//Cria um sinal
signals:
    void updateGUI(int,int,int);

private:
    int x, x_i;      //posição X do trem na tela | posicão auxiliar do X.
    int y, y_i;      //posição Y do trem na tela | posicão auxiliar do Y.
    int ID;          //ID do trem
    int velocidade;  //Velocidade. É o tempo de dormir em milisegundos entre a mudança de posição do trem
};

#endif // TREM_H