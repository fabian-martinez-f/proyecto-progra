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
    int triangulos[CANT][LADOS];
    int mejorHexagono[CANT][LADOS];
    int puntaje; /* -1 si no hay solucion */

    if (!leerDatos(NOMBRE_ARCHIVO, triangulos)){
        printf("Error al leer el archivo %s\n", NOMBRE_ARCHIVO);
        return 1;
    }

    puntaje = procesarDatos(triangulos, mejorHexagono);
    mostrarSalida(puntaje, mejorHexagono);

    return 0;
}

/* lee los 6 triangulos del archivo, retorna 0 si hay error */
int leerDatos(char nombreArchivo[], int triangulos[CANT][LADOS]){
    FILE *archivo;
    int i, j;
    int correcto = 1;

    archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL)
        return 0;

    for (i = 0; i < CANT && correcto; i++)
        for (j = 0; j < LADOS && correcto; j++)
            if (fscanf(archivo, "%d", &triangulos[i][j]) != 1)
                correcto = 0;

    fclose(archivo);
    return correcto;
}

/* prepara la busqueda y retorna el mejor puntaje (-1 si no hay hexagono valido) */
int procesarDatos(int triangulos[CANT][LADOS], int mejorHexagono[CANT][LADOS]){
    int usado[CANT] = {0, 0, 0, 0, 0, 0}; /* 1 si el triangulo ya esta puesto */
    int actual[CANT][LADOS];
    int mejorPuntaje = -1;

    /* &mejorPuntaje para que la funcion lo pueda cambiar */
    buscarHexagono(triangulos, usado, actual, 0, mejorHexagono, &mejorPuntaje);

    return mejorPuntaje;
}

/* busca el mejor hexagono usando RECURSIVIDAD: en cada llamada pone un
   triangulo en una posicion y se llama a si misma para la siguiente.
   Cada triangulo queda como {radio izq, lado externo, radio der} y el radio
   der de uno tiene que ser igual al radio izq del siguiente */
void buscarHexagono(int triangulos[CANT][LADOS], int usado[CANT], int actual[CANT][LADOS], int posicion, int mejorHexagono[CANT][LADOS], int *mejorPuntaje){
    int t, r, i;
    int limite;
    int puntaje;
    int rotado[LADOS];

    /* caso base: ya estan los 6 triangulos, falta ver que el ultimo
       cierre con el primero */
    if (posicion == CANT){
        if (actual[CANT - 1][2] == actual[0][0]){
            puntaje = calcularPuntaje(actual);

            if (puntaje > *mejorPuntaje){
                *mejorPuntaje = puntaje;
                for (i = 0; i < CANT; i++)
                    copiarTriangulo(actual[i], mejorHexagono[i]);
            }
        }
        return;
    }

    /* en la posicion 0 solo va el triangulo 0 (girar el hexagono
       no cambia el puntaje) */
    limite = (posicion == 0) ? 1 : CANT;

    for (t = 0; t < limite; t++){
        if (usado[t])
            continue;

        for (r = 0; r < LADOS; r++){
            rotarTriangulo(triangulos[t], r, rotado);

            /* solo sigue si encaja con el anterior */
            if (posicion == 0 || rotado[0] == actual[posicion - 1][2]){
                copiarTriangulo(rotado, actual[posicion]);
                usado[t] = 1;

                /* llamada recursiva para la siguiente posicion */
                buscarHexagono(triangulos, usado, actual, posicion + 1,
                               mejorHexagono, mejorPuntaje);

                /* backtracking: se desmarca para probar otro */
                usado[t] = 0;
            }
        }
    }
}

/* gira el triangulo sin voltearlo, {a,b,c} pasa a {b,c,a} */
void rotarTriangulo(int original[LADOS], int rotacion, int resultado[LADOS]){
    int i;

    for (i = 0; i < LADOS; i++)
        resultado[i] = original[(i + rotacion) % LADOS];
}

/* copia un triangulo en otro */
void copiarTriangulo(int origen[LADOS], int destino[LADOS]){
    int i;

    for (i = 0; i < LADOS; i++)
        destino[i] = origen[i];
}

/* suma los 6 lados externos (el indice 1 de cada triangulo) */
int calcularPuntaje(int hexagono[CANT][LADOS]){
    int i;
    int suma = 0;

    for (i = 0; i < CANT; i++)
        suma = suma + hexagono[i][1];

    return suma;
}

/* abre la ventana de raylib y muestra el resultado */
void mostrarSalida(int puntaje, int hexagono[CANT][LADOS]){
    char texto[10];
    const char *mensaje = "Imposible llegar a una solución";
    int ancho; /* para centrar el texto */

    InitWindow(ANCHO_VENTANA, ALTO_VENTANA, "El juego de triangulos");
    SetTargetFPS(60);

    /* se repite hasta que se cierre la ventana */
    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (puntaje < 0){
            ancho = MeasureText(mensaje, 28);
            DrawText(mensaje, (ANCHO_VENTANA - ancho) / 2,
                     ALTO_VENTANA / 2 - 14, 28, MAROON);
        }
        else{
            sprintf(texto, "%d", puntaje);
            ancho = MeasureText(texto, 40);
            DrawText(texto, (ANCHO_VENTANA - ancho) / 2, 20, 40, DARKBLUE);
            dibujarHexagono(hexagono, ANCHO_VENTANA / 2, 340, 220);
        }

        EndDrawing();
    }

    CloseWindow();
}

/* dibuja el hexagono con los numeros, (cx, cy) es el centro */
void dibujarHexagono(int hexagono[CANT][LADOS], int cx, int cy, int radio){
    double vx[CANT], vy[CANT];
    double angulo;
    double gx, gy; /* centro de cada triangulo */
    int k;
    int sig;

    /* vertices cada 60 grados, el menos en Y es porque en pantalla Y crece hacia abajo */
    for (k = 0; k < CANT; k++){
        angulo = PI / 2 - k * PI / 3;
        vx[k] = cx + radio * cos(angulo);
        vy[k] = cy - radio * sin(angulo);
    }

    /* cada triangulo usa el centro y los vertices k y k+1 */
    for (k = 0; k < CANT; k++){
        sig = (k + 1) % CANT; /* despues del 5 vuelve al 0 */

        dibujarLinea(cx, cy, vx[k], vy[k], 2.0f, GRAY);
        dibujarLinea(vx[k], vy[k], vx[sig], vy[sig], 4.0f, DARKBLUE);

        gx = (cx + vx[k] + vx[sig]) / 3.0;
        gy = (cy + vy[k] + vy[sig]) / 3.0;

        /* los numeros van en el punto medio de cada lado */
        dibujarNumero(hexagono[k][0], gx, gy,
                      (cx + vx[k]) / 2.0, (cy + vy[k]) / 2.0);

        dibujarNumero(hexagono[k][1], gx, gy,
                      (vx[k] + vx[sig]) / 2.0, (vy[k] + vy[sig]) / 2.0);

        dibujarNumero(hexagono[k][2], gx, gy,
                      (vx[sig] + cx) / 2.0, (vy[sig] + cy) / 2.0);
    }
}

/* escribe un numero dentro del triangulo, cerca de su lado */
void dibujarNumero(int valor, double gx, double gy, double mx, double my){
    char texto[10];
    int x, y;
    int ancho;

    /* se mueve el 60% desde el centro hacia el lado para que quede adentro */
    x = (int)(gx + 0.6 * (mx - gx));
    y = (int)(gy + 0.6 * (my - gy));

    sprintf(texto, "%d", valor);

    /* DrawText dibuja desde la esquina, por eso se resta la mitad para centrar */
    ancho = MeasureText(texto, TAM_NUMERO);
    DrawText(texto, x - ancho / 2, y - TAM_NUMERO / 2, TAM_NUMERO, BLACK);
}

/* dibuja una linea con grosor */
void dibujarLinea(double x1, double y1, double x2, double y2, float grosor, Color color){
    Vector2 inicio, fin;

    inicio.x = (float)x1;
    inicio.y = (float)y1;
    fin.x = (float)x2;
    fin.y = (float)y2;

    DrawLineEx(inicio, fin, grosor, color);
}
