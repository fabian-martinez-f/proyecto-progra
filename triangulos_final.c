/* Autores: Fabian Martinez y Benjamín Loyola
   Fecha: 26/10/2026 */

#include <stdio.h>
#include <math.h>
#include "raylib.h"

#define CANT 6
#define LADOS 3
#define ANCHO_VENTANA 600
#define ALTO_VENTANA 600
#define TAM_NUMERO 22
#define NOMBRE_ARCHIVO "triangulo.txt"

int leerDatos(char nombreArchivo[], int triangulos[CANT][LADOS]);
int procesarDatos(int triangulos[CANT][LADOS], int mejorHexagono[CANT][LADOS]);
void mostrarSalida(int puntaje, int hexagono[CANT][LADOS]);
void buscarHexagono(int triangulos[CANT][LADOS], int usado[CANT], int actual[CANT][LADOS], int posicion, int mejorHexagono[CANT][LADOS], int *mejorPuntaje);
void rotarTriangulo(int original[LADOS], int rotacion, int resultado[LADOS]);
void copiarTriangulo(int origen[LADOS], int destino[LADOS]);
int calcularPuntaje(int hexagono[CANT][LADOS]);
void dibujarHexagono(int hexagono[CANT][LADOS], int cx, int cy, int radio);
void dibujarNumero(int valor, double gx, double gy, double mx, double my);
void dibujarLinea(double x1, double y1, double x2, double y2, float grosor, Color color);


int main(void){
    /* Matriz con los 6 triangulos leidos del archivo (cada fila = 1 triangulo) */
    int triangulos[CANT][LADOS];

    /* Matriz donde se guardara el mejor hexagono encontrado, en el orden
       en que los triangulos quedan ubicados alrededor del centro */
    int mejorHexagono[CANT][LADOS];

    /* Puntuacion del mejor hexagono (-1 si no existe solucion) */
    int puntaje;

    /* 1) ENTRADA: leer los datos. Si falla, se avisa y se termina */
    if (!leerDatos(NOMBRE_ARCHIVO, triangulos)){
        printf("Error al leer el archivo %s\n", NOMBRE_ARCHIVO);
        return 1;
    }

    /* 2) PROCESO: buscar el hexagono valido de mayor puntaje */
    puntaje = procesarDatos(triangulos, mejorHexagono);

    /* 3) SALIDA: mostrar el resultado en la ventana grafica */
    mostrarSalida(puntaje, mejorHexagono);

    return 0;
}

int leerDatos(char nombreArchivo[], int triangulos[CANT][LADOS]){
    FILE *archivo;
    int i, j;
    int correcto = 1; /* se pone en 0 si falla alguna lectura */

    archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL)
        return 0;

    /* Se leen 6 filas de 3 enteros. fscanf retorna cuantos datos leyo,
       por lo que si no retorna 1 hubo un problema con ese numero */
    for (i = 0; i < CANT && correcto; i++)
        for (j = 0; j < LADOS && correcto; j++)
            if (fscanf(archivo, "%d", &triangulos[i][j]) != 1)
                correcto = 0;

    fclose(archivo);
    return correcto;
}

int procesarDatos(int triangulos[CANT][LADOS], int mejorHexagono[CANT][LADOS]){
    /* usado[t] vale 1 si el triangulo t ya esta puesto en el anillo */
    int usado[CANT] = {0, 0, 0, 0, 0, 0};

    /* Hexagono que se va armando durante la busqueda */
    int actual[CANT][LADOS];

    /* Empieza en -1: cualquier puntaje valido (minimo 6) sera mayor */
    int mejorPuntaje = -1;

    /* Se parte desde la posicion 0 del anillo. mejorPuntaje se pasa por
       referencia (&) para que la funcion pueda modificarlo */
    buscarHexagono(triangulos, usado, actual, 0, mejorHexagono, &mejorPuntaje);

    return mejorPuntaje;
}

void buscarHexagono(int triangulos[CANT][LADOS], int usado[CANT], int actual[CANT][LADOS], int posicion, int mejorHexagono[CANT][LADOS], int *mejorPuntaje){
    int t, r, i; /* triangulo que se intenta ubicar */
    int limite; /* hasta que triangulo se prueba */
    int puntaje; /* puntaje del hexagono completo */
    int rotado[LADOS]; /* triangulo t ya rotado */

    /* CASO BASE: los 6 triangulos ya estan ubicados */
    if (posicion == CANT){
        /* El anillo debe cerrarse: el radio derecho del ultimo debe
           coincidir con el radio izquierdo del primero */
        if (actual[CANT - 1][2] == actual[0][0]){
            puntaje = calcularPuntaje(actual);

            /* Si es mejor que el mejor encontrado, se guarda una copia */
            if (puntaje > *mejorPuntaje){
                *mejorPuntaje = puntaje;
                for (i = 0; i < CANT; i++)
                    copiarTriangulo(actual[i], mejorHexagono[i]);
            }
        }
        return;
    }

    limite = (posicion == 0) ? 1 : CANT;

    for (t = 0; t < limite; t++){
        /* Si el triangulo ya esta en el anillo, se salta */
        if (usado[t])
            continue;

        /* Se prueban las 3 rotaciones posibles del triangulo */
        for (r = 0; r < LADOS; r++){
            rotarTriangulo(triangulos[t], r, rotado);

            /* Solo se continua si encaja con el triangulo anterior
               (en la posicion 0 no hay anterior, por eso siempre sirve) */
            if (posicion == 0 || rotado[0] == actual[posicion - 1][2]){
                /* Se ubica el triangulo y se marca como usado */
                copiarTriangulo(rotado, actual[posicion]);
                usado[t] = 1;

                /* Recursion: se intenta llenar la siguiente posicion */
                buscarHexagono(triangulos, usado, actual, posicion + 1,
                               mejorHexagono, mejorPuntaje);

                /* Backtracking: se deshace la eleccion para probar otra */
                usado[t] = 0;
            }
        }
    }
}

void rotarTriangulo(int original[LADOS], int rotacion, int resultado[LADOS]){
    int i; /* indice del lado */

    /* El operador % hace que al llegar al final se vuelva al inicio */
    for (i = 0; i < LADOS; i++)
        resultado[i] = original[(i + rotacion) % LADOS];
}

void copiarTriangulo(int origen[LADOS], int destino[LADOS]){
    int i; /* indice del lado */

    for (i = 0; i < LADOS; i++)
        destino[i] = origen[i];
}

int calcularPuntaje(int hexagono[CANT][LADOS]){
    int i; /* indice del triangulo */
    int suma = 0; /* acumulador de la puntuacion */

    for (i = 0; i < CANT; i++)
        suma = suma + hexagono[i][1];

    return suma;
}

void mostrarSalida(int puntaje, int hexagono[CANT][LADOS]){
    char texto[10]; /* puntaje convertido a texto */
    const char *mensaje = "Imposible llegar a una solución";
    int ancho; /* ancho en pixeles de un texto, para centrarlo */

    /* Se crea la ventana y se fija la velocidad a 60 cuadros por segundo */
    InitWindow(ANCHO_VENTANA, ALTO_VENTANA, "El juego de triangulos");
    SetTargetFPS(60);

    /* Ciclo de raylib: cada vuelta dibuja un cuadro de la ventana y se
       repite hasta que el usuario la cierra */
    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (puntaje < 0){
            /* No hay solucion: solo se muestra la frase, centrada */
            ancho = MeasureText(mensaje, 28);
            DrawText(mensaje, (ANCHO_VENTANA - ancho) / 2,
                     ALTO_VENTANA / 2 - 14, 28, MAROON);
        }
        else{
            /* Hay solucion: puntaje arriba (centrado) y hexagono debajo */
            sprintf(texto, "%d", puntaje);
            ancho = MeasureText(texto, 40);
            DrawText(texto, (ANCHO_VENTANA - ancho) / 2, 20, 40, DARKBLUE);
            dibujarHexagono(hexagono, ANCHO_VENTANA / 2, 340, 220);
        }

        EndDrawing();               /* termina y muestra el cuadro */
    }

    /* Se libera la ventana al salir del ciclo */
    CloseWindow();
}

void dibujarHexagono(int hexagono[CANT][LADOS], int cx, int cy, int radio){
    double vx[CANT], vy[CANT]; /* coordenadas de los 6 vertices */
    double angulo; /* angulo de cada vertice, en radianes */
    double gx, gy; /* centro (baricentro) de cada triangulo */
    int k; /* triangulo que se dibuja */
    int sig; /* indice del vertice siguiente */

    for (k = 0; k < CANT; k++){
        angulo = PI / 2 - k * PI / 3;
        vx[k] = cx + radio * cos(angulo);
        vy[k] = cy - radio * sin(angulo);
    }

    /* El triangulo k tiene como vertices el centro, v[k] y v[k+1] */
    for (k = 0; k < CANT; k++){
        sig = (k + 1) % CANT;   /* el % hace que despues del 5 vuelva al 0 */

        /* Radio (linea fina) y lado externo (linea gruesa) */
        dibujarLinea(cx, cy, vx[k], vy[k], 2.0f, GRAY);
        dibujarLinea(vx[k], vy[k], vx[sig], vy[sig], 4.0f, DARKBLUE);

        /* Centro del triangulo: promedio de sus 3 vertices */
        gx = (cx + vx[k] + vx[sig]) / 3.0;
        gy = (cy + vy[k] + vy[sig]) / 3.0;

        /* Numero 0: radio izquierdo (punto medio entre centro y v[k]) */
        dibujarNumero(hexagono[k][0], gx, gy,
                      (cx + vx[k]) / 2.0, (cy + vy[k]) / 2.0);

        /* Numero 1: lado externo (punto medio entre v[k] y v[sig]) */
        dibujarNumero(hexagono[k][1], gx, gy,
                      (vx[k] + vx[sig]) / 2.0, (vy[k] + vy[sig]) / 2.0);

        /* Numero 2: radio derecho (punto medio entre v[sig] y el centro) */
        dibujarNumero(hexagono[k][2], gx, gy,
                      (vx[sig] + cx) / 2.0, (vy[sig] + cy) / 2.0);
    }
}

void dibujarNumero(int valor, double gx, double gy, double mx, double my){
    char texto[10]; /* numero convertido a texto */
    int x, y; /* posicion final del texto */
    int ancho;

    x = (int)(gx + 0.6 * (mx - gx));
    y = (int)(gy + 0.6 * (my - gy));

    /* DrawText necesita texto, por eso se convierte el entero */
    sprintf(texto, "%d", valor);

    /* DrawText dibuja desde la esquina superior izquierda, asi que se
       corre medio ancho y medio alto para que el numero quede centrado */
    ancho = MeasureText(texto, TAM_NUMERO);
    DrawText(texto, x - ancho / 2, y - TAM_NUMERO / 2, TAM_NUMERO, BLACK);
}

void dibujarLinea(double x1, double y1, double x2, double y2, float grosor, Color color){
    Vector2 inicio, fin;/* puntos en el formato que pide raylib */

    inicio.x = (float)x1;
    inicio.y = (float)y1;
    fin.x = (float)x2;
    fin.y = (float)y2;

    DrawLineEx(inicio, fin, grosor, color);
}
