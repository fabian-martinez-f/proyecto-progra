/* Autores: Fabian Martinez y Benjamín Loyola> */

/* ------------------------- DIRECTIVAS DE PREPROCESO ------------------- */

#include <stdio.h>              /* printf, fopen, fscanf, sprintf */
#include <math.h>               /* cos y sin para los vertices del hexagono */
#include "raylib.h"             /* biblioteca grafica (ya define PI) */

/* ------------------------------ CONSTANTES ---------------------------- */

#define CANT 6                  /* cantidad de triangulos del juego */
#define LADOS 3                 /* cantidad de lados de un triangulo */
#define ANCHO_VENTANA 600       /* ancho de la ventana en pixeles */
#define ALTO_VENTANA 600        /* alto de la ventana en pixeles */
#define TAM_NUMERO 22           /* tamano de letra de los numeros */
#define NOMBRE_ARCHIVO "triangulo.txt"  /* archivo de entrada */

/* ----------------------- PROTOTIPOS DE FUNCIONES ---------------------- */

/* Funciones principales del programa (entrada, proceso y salida) */
int leerDatos(char nombreArchivo[], int triangulos[CANT][LADOS]);
int procesarDatos(int triangulos[CANT][LADOS], int mejorHexagono[CANT][LADOS]);
void mostrarSalida(int puntaje, int hexagono[CANT][LADOS]);

/* Funciones de apoyo del proceso */
void buscarHexagono(int triangulos[CANT][LADOS], int usado[CANT], int actual[CANT][LADOS], int posicion, int mejorHexagono[CANT][LADOS], int *mejorPuntaje);
void rotarTriangulo(int original[LADOS], int rotacion, int resultado[LADOS]);
void copiarTriangulo(int origen[LADOS], int destino[LADOS]);
int calcularPuntaje(int hexagono[CANT][LADOS]);

/* Funciones de apoyo de la salida grafica */
void dibujarHexagono(int hexagono[CANT][LADOS], int cx, int cy, int radio);
void dibujarNumero(int valor, double gx, double gy, double mx, double my);
void dibujarLinea(double x1, double y1, double x2, double y2, float grosor, Color color);

/* ---------------------------- PROGRAMA PRINCIPAL ---------------------- */

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

/* ------------------------ DEFINICION DE FUNCIONES --------------------- */

/* Objetivo: leer los 6 triangulos desde el archivo de texto.
   Parametros: nombreArchivo (nombre del archivo) y triangulos (matriz donde
   se guardan los numeros leidos).
   Retorna: 1 si la lectura fue correcta, 0 si hubo algun error. */
int leerDatos(char nombreArchivo[], int triangulos[CANT][LADOS]){
    FILE *archivo;              /* puntero al archivo abierto */
    int i, j;                   /* indices de fila (triangulo) y columna (lado) */
    int correcto = 1;           /* se pone en 0 si falla alguna lectura */

    /* Se abre el archivo en modo lectura; NULL significa que no existe */
    archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL)
        return 0;

    /* Se leen 6 filas de 3 enteros. fscanf retorna cuantos datos leyo,
       por lo que si no retorna 1 hubo un problema con ese numero */
    for (i = 0; i < CANT && correcto; i++)
        for (j = 0; j < LADOS && correcto; j++)
            if (fscanf(archivo, "%d", &triangulos[i][j]) != 1)
                correcto = 0;

    /* Siempre se cierra el archivo al terminar de usarlo */
    fclose(archivo);
    return correcto;
}

/* Objetivo: preparar la busqueda del mejor hexagono y lanzarla.
   Parametros: triangulos (datos de entrada) y mejorHexagono (aqui se deja
   el hexagono ganador).
   Retorna: el mejor puntaje, o -1 si no existe ningun hexagono valido. */
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

/* Objetivo: probar con backtracking todas las formas de ubicar los
   triangulos en el anillo y quedarse con la de mayor puntaje.
   Cada triangulo ubicado se guarda como {radio izquierdo, lado externo,
   radio derecho}. Para que encajen, el radio derecho de un triangulo debe
   ser igual al radio izquierdo del siguiente.
   Parametros: triangulos (datos originales), usado (marcas de uso),
   actual (hexagono en construccion), posicion (casilla del anillo que se
   esta llenando, de 0 a 5), mejorHexagono y mejorPuntaje (resultado). */
void buscarHexagono(int triangulos[CANT][LADOS], int usado[CANT], int actual[CANT][LADOS], int posicion, int mejorHexagono[CANT][LADOS], int *mejorPuntaje){
    int t;                      /* triangulo que se intenta ubicar */
    int r;                      /* rotacion que se prueba (0, 1 o 2) */
    int i;                      /* indice para copiar el hexagono */
    int limite;                 /* hasta que triangulo se prueba */
    int puntaje;                /* puntaje del hexagono completo */
    int rotado[LADOS];          /* triangulo t ya rotado */

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

    /* En la posicion 0 solo se usa el triangulo 0: girar todo el hexagono
       no cambia su puntaje, asi que no hace falta probar otros como inicio.
       En las demas posiciones se prueban los 6 triangulos */
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

/* Objetivo: rotar un triangulo manteniendo el orden del reloj (nunca se
   voltea). Con rotacion 1, por ejemplo, {a, b, c} queda {b, c, a}.
   Parametros: original (triangulo a rotar), rotacion (0, 1 o 2) y
   resultado (donde se deja el triangulo rotado). */
void rotarTriangulo(int original[LADOS], int rotacion, int resultado[LADOS]){
    int i;                      /* indice del lado */

    /* El operador % hace que al llegar al final se vuelva al inicio */
    for (i = 0; i < LADOS; i++)
        resultado[i] = original[(i + rotacion) % LADOS];
}

/* Objetivo: copiar los 3 lados de un triangulo en otro.
   Parametros: origen (se lee) y destino (se escribe). */
void copiarTriangulo(int origen[LADOS], int destino[LADOS]){
    int i;                      /* indice del lado */

    for (i = 0; i < LADOS; i++)
        destino[i] = origen[i];
}

/* Objetivo: calcular la puntuacion de un hexagono, que es la suma de sus
   6 lados externos. En cada triangulo el lado externo es el indice 1.
   Parametros: hexagono (matriz con los 6 triangulos ya ubicados).
   Retorna: la suma de los lados externos. */
int calcularPuntaje(int hexagono[CANT][LADOS]){
    int i;                      /* indice del triangulo */
    int suma = 0;               /* acumulador de la puntuacion */

    for (i = 0; i < CANT; i++)
        suma = suma + hexagono[i][1];

    return suma;
}

/* Objetivo: abrir la ventana de raylib y mostrar el resultado hasta que
   el usuario la cierre (boton X o tecla ESC).
   Parametros: puntaje (-1 si no hay solucion) y hexagono (el hexagono a
   dibujar). */
void mostrarSalida(int puntaje, int hexagono[CANT][LADOS]){
    char texto[10];             /* puntaje convertido a texto */
    const char *mensaje = "Imposible llegar a una solución";
    int ancho;                  /* ancho en pixeles de un texto, para centrarlo */

    /* Se crea la ventana y se fija la velocidad a 60 cuadros por segundo */
    InitWindow(ANCHO_VENTANA, ALTO_VENTANA, "El juego de triangulos");
    SetTargetFPS(60);

    /* Ciclo de raylib: cada vuelta dibuja un cuadro de la ventana y se
       repite hasta que el usuario la cierra */
    while (!WindowShouldClose()){
        BeginDrawing();             /* comienza el dibujo del cuadro */
        ClearBackground(RAYWHITE);  /* se pinta el fondo */

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

/* Objetivo: dibujar el hexagono completo con los numeros de cada lado.
   Parametros: hexagono (triangulos ubicados), cx y cy (centro del dibujo)
   y radio (distancia del centro a cada vertice). */
void dibujarHexagono(int hexagono[CANT][LADOS], int cx, int cy, int radio){
    double vx[CANT], vy[CANT];  /* coordenadas de los 6 vertices */
    double angulo;              /* angulo de cada vertice, en radianes */
    double gx, gy;              /* centro (baricentro) de cada triangulo */
    int k;                      /* triangulo que se dibuja */
    int sig;                    /* indice del vertice siguiente */

    /* Los vertices estan separados 60 grados (PI/3). Se parte desde
       arriba (PI/2) y se avanza en sentido del reloj. A cy se le resta
       porque en pantalla el eje Y crece hacia abajo */
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

/* Objetivo: escribir un numero dentro de un triangulo, cerca del lado
   indicado.
   Parametros: valor (numero a escribir), (gx, gy) centro del triangulo y
   (mx, my) punto medio del lado al que pertenece el numero. */
void dibujarNumero(int valor, double gx, double gy, double mx, double my){
    char texto[10];             /* numero convertido a texto */
    int x, y;                   /* posicion final del texto */
    int ancho;                  /* ancho del texto en pixeles */

    /* El numero se ubica al 60% del camino entre el centro del triangulo
       y el punto medio del lado: asi queda dentro y cerca de su lado */
    x = (int)(gx + 0.6 * (mx - gx));
    y = (int)(gy + 0.6 * (my - gy));

    /* DrawText necesita texto, por eso se convierte el entero */
    sprintf(texto, "%d", valor);

    /* DrawText dibuja desde la esquina superior izquierda, asi que se
       corre medio ancho y medio alto para que el numero quede centrado */
    ancho = MeasureText(texto, TAM_NUMERO);
    DrawText(texto, x - ancho / 2, y - TAM_NUMERO / 2, TAM_NUMERO, BLACK);
}

/* Objetivo: dibujar una linea con grosor entre dos puntos usando raylib.
   Parametros: (x1, y1) y (x2, y2) extremos de la linea, grosor en pixeles
   y color de la linea. */
void dibujarLinea(double x1, double y1, double x2, double y2, float grosor, Color color){
    Vector2 inicio, fin;        /* puntos en el formato que pide raylib */

    inicio.x = (float)x1;
    inicio.y = (float)y1;
    fin.x = (float)x2;
    fin.y = (float)y2;

    DrawLineEx(inicio, fin, grosor, color);
}
