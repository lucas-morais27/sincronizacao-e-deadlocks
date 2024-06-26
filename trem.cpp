#include "trem.h"
#include <QtCore>
#include <QSemaphore>
#include <iostream>

using namespace std;

/*
 * Esquema das áreas críticas.

                _____#_______________
                |         |         |
                |    ⤵    0   ⤵    #
            ____|_1_____2_|_3_____4_|____
            |        |         |        |   
            #   ⤵   5    ⤵    6   ⤵   #
            |________|____#____|________|

 * Áreas críticas.
 */
static QMutex area0; 
static QMutex area1;
static QMutex area2;
static QMutex area3;
static QMutex area4;
static QMutex area5;
static QMutex area6;
static QMutex mutex_auxiliar;

/*
 * Semáforos para auxiliar os deadlocks.
 */

static QSemaphore regiao1(2); // Composta pelas áreas 0, 2 e 3.
static QSemaphore regiao2(2); // Composta pelas áreas 1, 2 e 5.
static QSemaphore regiao3(2); // Composta pelas áreas 3, 4 e 6.
static QSemaphore regiao1_2(2); // Composta pelas regiões 1 e 2.
static QSemaphore regiao1_3(2); // Composta pelas regiões 1 e 3.
static QSemaphore regiao_geral(2); // Composta por todas as regiões.


Trem::Trem(int ID, int x, int y) {
    this->ID = ID;
    this->x = x;
    this->y = y;
    this->velocidade = 100; 
    this->parado = false;
}

void Trem::set_velocidade(int velocidade) {
    this->velocidade = velocidade;
}

void Trem::move(const int opcoesCoordenadas[5][4]){
    if(!parado && velocidade < 200){
        const int xMax = opcoesCoordenadas[ID-1][0];
        const int yMin = opcoesCoordenadas[ID-1][1];
        const int yMax = opcoesCoordenadas[ID-1][2];
        const int xMin = opcoesCoordenadas[ID-1][3];

        if(x < xMax && y == yMin) // DIREITA
            x+=10;
        else if(x == xMax && y < yMax) // BAIXO
            y+=10;
        else if(x > xMin && y == yMax) // ESQUERA
            x-=10;
        else // CIMA
            y-=10;

        emit updateGUI(ID, x, y);
    }

    msleep(velocidade);
}

void Trem::run() {
    const int opcoesCoordenadas[5][4] = {
        // xMax yMin yMax xMin
          {240, 40, 140, 110},
          {380, 40, 140, 240},
          {170, 140, 240, 40},
          {310, 140, 240, 170},
          {450, 140, 240, 310}
    };

    while(true){
        this->move(opcoesCoordenadas); // Fazer trem se mover

        switch(ID) {
        case 1: //Trem 1
            if (x == 220 && y == 40) { //entrando na região 0

                mutex_auxiliar.lock();

                if (regiao_geral.tryAcquire(1)) {

                    if (regiao1_2.tryAcquire(1)) {

                        if (regiao1_3.tryAcquire(1)) {

                            if (regiao1.tryAcquire(1)) {

                                if (area0.try_lock()) {
                                    this->parado = false;

                                } else {
                                    regiao_geral.release(1);
                                    regiao1.release(1);
                                    regiao1_2.release(1);
                                    regiao1_3.release(1);
                                    this->parado = true;
                                }

                            } else {
                                regiao_geral.release(1);
                                regiao1_3.release(1);
                                regiao1_2.release(1);
                                this->parado = true;
                            }

                        } else {
                            regiao_geral.release(1);
                            regiao1_2.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao_geral.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();  

            } else if (x == 240 && y == 120) { //entrando na região 2

                mutex_auxiliar.lock();
                
                if (regiao2.tryAcquire(1)) {

                    if (area2.try_lock()) {
                        area0.unlock();
                        this->parado = false;

                    } else {
                        regiao2.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();  

            } else if (x == 190 && y == 140) { //entrando na região 1

                area1.lock();
                area2.unlock();

                mutex_auxiliar.lock();

                regiao1.release(1);

                mutex_auxiliar.unlock();

            } else if (x == 110 && y == 140) { //saindo da região 1

                area1.unlock();
                mutex_auxiliar.lock();

                regiao_geral.release(1);
                regiao2.release(1);
                regiao1_3.release(1);
                regiao1_2.release(1);

                mutex_auxiliar.unlock();

            }
            
            break;

        case 2: //Trem 2
            if (x == 380 && y == 120) { //entrando na região 4

                mutex_auxiliar.lock();
                
                if (regiao_geral.tryAcquire(1)) {
                    
                    if (regiao1_3.tryAcquire(1)) {
                        
                        if (regiao3.tryAcquire(1)) {
                            
                            if (area4.try_lock()) {
                                this->parado = false;

                            } else {
                                regiao_geral.release(1);
                                regiao3.release(1);
                                regiao1_3.release(1);
                                this->parado = true;
                            }

                        } else {
                            regiao_geral.release(1);
                            regiao1_3.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao_geral.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }
                
                mutex_auxiliar.unlock();    

            } else if (x == 310 && y == 140) { //entrando na região 3

                mutex_auxiliar.lock();

                if (regiao1_2.tryAcquire(1)) {
                    
                    if (regiao1.tryAcquire(1)) {
                        
                        if (area3.try_lock()) {
                            area4.unlock();
                            this->parado = false;

                        } else {
                            regiao1.release(1);
                            regiao1_2.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao1_2.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock(); 

            } else if (x == 260 && y == 140) { //entrando na região 0

                area0.lock();
                area3.unlock();

                mutex_auxiliar.lock();
                
                regiao3.release(1);

                mutex_auxiliar.unlock();

            } else if (x == 240 && y == 40) { //saindo da região 0

                area0.unlock();

                mutex_auxiliar.lock();

                regiao_geral.release(1);
                regiao1.release(1);
                regiao1_2.release(1);
                regiao1_3.release(1);

                mutex_auxiliar.unlock();

            }

            break;

        case 3: //Trem 3
            if (x == 90 && y == 140) { //entrando na regiao 1.

                mutex_auxiliar.lock();
                
                if (regiao_geral.tryAcquire(1)) {
                    
                    if (regiao1_2.tryAcquire(1)) {
                        
                        if (regiao2.tryAcquire(1)) {
                            
                            if (area1.try_lock()) {
                                this->parado = false;

                            } else {
                                regiao_geral.release(1);
                                regiao2.release(1);
                                regiao1_2.release(1);
                                this->parado = true;
                            }

                        } else {
                            regiao_geral.release(1);
                            regiao1_2.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao_geral.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();

            } else if (x == 150 && y == 140) { //entrando na região 5.

                area5.lock();
                area1.unlock();

            } else if (x == 170 && y == 220) { //saindo da regiao 5.

                area5.unlock();

                mutex_auxiliar.lock();

                regiao_geral.release(1);
                regiao2.release(1);
                regiao1_2.release(1);

                mutex_auxiliar.unlock();

            }

            break;

        case 4: //Trem 4
            if (x == 190 && y == 240) { //entrando na região 5.
                
                mutex_auxiliar.lock();
                
                if (regiao_geral.tryAcquire(1)) {
                    
                    if (regiao1_2.tryAcquire(1)) {
                        
                        if (regiao2.tryAcquire(1)) {
                            
                            if (area5.try_lock()) {
                                this->parado = false;

                            } else {
                                regiao_geral.release(1);
                                regiao2.release(1);
                                regiao1_2.release(1);
                                this->parado = true;
                            }

                        } else {
                            regiao_geral.release(1);
                            regiao1_2.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao_geral.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }
                    
                mutex_auxiliar.unlock();

            } else if (x == 170 && y == 160) { //entrando na região 2
                
                mutex_auxiliar.lock();
                
                if (regiao1_3.tryAcquire(1)) {
                    
                    if (regiao1.tryAcquire(1)) {
                        
                        if (area2.try_lock()) {
                            area5.unlock();
                            this->parado = false;

                        } else {

                            if (regiao1_3.available() < 2) {
                                regiao1_3.release(1);
                            }
                            regiao1.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao1_3.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();

            } else if (x == 220 && y == 140) { //entrando na região 3
                
                mutex_auxiliar.lock();
                
                if (regiao3.tryAcquire(1)) {
                    
                    if (area3.try_lock()) {
                        area2.unlock();
                        regiao2.release(1);
                        this->parado = false;

                    } else {
                        regiao3.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();

            } else if (x == 290 && y == 140) { //entrando na região 6.
                
                area6.lock();
                area3.unlock();

                mutex_auxiliar.lock();

                regiao1.release(1);
                regiao1_2.release(1);

                mutex_auxiliar.unlock();

            } else if (x == 310 && y == 220) { //saindo da região 6.

                area6.unlock();

                mutex_auxiliar.lock();

                regiao_geral.release(1);
                regiao3.release(1);
                regiao1_3.release(1);

                mutex_auxiliar.unlock();

            }

            break;

        case 5: //Trem 5
            if (x == 330 && y == 240) { //entrando na região 6
                
                mutex_auxiliar.lock();
                
                if (regiao_geral.tryAcquire(1)) {
                    
                    if (regiao1_3.tryAcquire(1)) {
                        
                        if (regiao3.tryAcquire(1)) {
                            
                            if (area6.try_lock()) {
                                this->parado = false;

                            } else {
                                regiao_geral.release(1);
                                regiao3.release(1);
                                regiao1_3.release(1);
                                this->parado = true;
                            }

                        } else {
                            regiao_geral.release(1);
                            regiao1_3.release(1);
                            this->parado = true;
                        }

                    } else {
                        regiao_geral.release(1);
                        this->parado = true;
                    }

                } else {
                    this->parado = true;
                }

                mutex_auxiliar.unlock();

            } else if (x == 310 && y == 160) { //entrando na região 4
                
                area4.lock();
                area6.unlock();

            } else if (x == 360 && y == 140) { //saindo da região 4.
                
                area4.unlock();

                mutex_auxiliar.lock();

                regiao_geral.release(1);
                regiao3.release(1);
                regiao1_3.release(1);

                mutex_auxiliar.unlock();

            }

            break;

        default:
            break;
        }
    }
}
