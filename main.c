
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <mongoc/mongoc.h>

// Tamaño máximo de los textos introducidos por el usuario.
#define MAX_TEXTO 150

// Número máximo de armas por personaje.
#define MAX_ARMAS 5

// Declaraciones de las funciones del programa.
void leerTexto(const char *mensaje, char *destino, size_t capacidad);
int leerEntero(const char *mensaje, int minimo, int maximo);

void agregarPersonaje(mongoc_collection_t *coleccion);
void mostrarPersonajes(mongoc_collection_t *coleccion);
void buscarPersonaje(mongoc_collection_t *coleccion);
void actualizarPersonaje(mongoc_collection_t *coleccion);
void eliminarPersonaje(mongoc_collection_t *coleccion);

// Función auxiliar para imprimir documentos BSON.
void imprimirDocumento(const bson_t *documento);

// Función auxiliar para comprobar si existe un personaje.
bool existePersonaje(mongoc_collection_t *coleccion, const char *nombre);

// ============================================================
// FUNCIÓN PRINCIPAL
// ============================================================

int main(void) {

    // Configura la consola de Windows para mostrar UTF-8.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Inicializa los recursos internos del controlador MongoDB.
    mongoc_init();

    // Crea un cliente para comunicarse con MongoDB local.
    mongoc_client_t *cliente = mongoc_client_new("mongodb://localhost:27017/");

    if (cliente == NULL) {
        printf("No se pudo crear el cliente de MongoDB.\n");
        mongoc_cleanup();
        return 1;
    }

    // Comprueba que el servidor realmente esté disponible.
    bson_t comando = BSON_INITIALIZER;
    bson_t respuesta = BSON_INITIALIZER;
    bson_error_t error;

    BSON_APPEND_INT32(&comando, "ping", 1);

    bool conectado = mongoc_client_command_simple(
        cliente, "admin", &comando, NULL, &respuesta, &error
    );

    bson_destroy(&comando);
    bson_destroy(&respuesta);

    if (!conectado) {
        printf("No se pudo conectar a MongoDB: %s\n", error.message);
        mongoc_client_destroy(cliente);
        mongoc_cleanup();
        return 1;
    }

    // Obtiene la colección en la que guardaremos los personajes.
    mongoc_collection_t *coleccion = mongoc_client_get_collection( cliente, "marvel_db", "personajes");
    printf("Conexion con MongoDB establecida correctamente.\n");
    int opcion;

    // Mantiene el menú activo hasta que el usuario elija salir.
    do{
        printf("\n========== MARVEL DATABASE ==========\n");
        printf("1. Agregar personaje\n");
        printf("2. Mostrar personajes\n");
        printf("3. Buscar personaje\n");
        printf("4. Actualizar personaje\n");
        printf("5. Eliminar personaje\n");
        printf("0. Salir\n");

        opcion = leerEntero("Seleccione una opcion: ", 0, 5);

        // Ejecuta la operación correspondiente.
        switch (opcion) {
            case 1:
                agregarPersonaje(coleccion);
                break;

            case 2:
                mostrarPersonajes(coleccion);
                break;

            case 3:
                buscarPersonaje(coleccion);
                break;

            case 4:
                actualizarPersonaje(coleccion);
                break;

            case 5:
                eliminarPersonaje(coleccion);
                break;

            case 0:
                printf("\nCerrando Marvel Database...\n");
                break;
        }

    } while (opcion != 0);

    // Libera los recursos utilizados por MongoDB.
    mongoc_collection_destroy(coleccion);
    mongoc_client_destroy(cliente);
    mongoc_cleanup();

    return 0;
}

// ============================================================
// FUNCIONES AUXILIARES DE ENTRADA
// ============================================================

// Lee una línea de texto, incluyendo espacios.
void leerTexto(const char *mensaje, char *destino, size_t capacidad) {

    do {
        printf("%s", mensaje);

        if (fgets(destino, (int)capacidad, stdin) == NULL) {
            // Si se cierra la entrada, terminamos el programa.
            printf("\nEntrada finalizada.\n");
            exit(EXIT_FAILURE);
        }

        // Busca el salto de línea que normalmente agrega fgets.
        char *salto = strchr(destino, '\n');

        if (salto != NULL) {
            // Sustituye el salto de línea por el fin de cadena.
            *salto = '\0';
        } else {
            // Si el texto excede el espacio disponible, se descartan los caracteres restantes.
            int caracter;

            while ((caracter = getchar()) != '\n' &&
                   caracter != EOF) {
            }
        }

        if (destino[0] == '\0') {
            printf("El texto no puede estar vacio.\n");
        }

    } while (destino[0] == '\0');
}

// Lee un número entero y comprueba que esté en el rango.
// Evita problemas si el usuario introduce letras.
int leerEntero(const char *mensaje, int minimo, int maximo) {

    char entrada[50];
    char *fin;
    long numero;

    while (true) {
        leerTexto(mensaje, entrada, sizeof(entrada));

        // strtol convierte texto a un número entero.
        numero = strtol(entrada, &fin, 10);

        // Si quedan caracteres distintos de espacios,
        // la entrada no es un número válido.
        while (*fin == ' ' || *fin == '\t') {
            fin++;
        }

        if (fin == entrada || *fin != '\0' ||
            numero < minimo || numero > maximo) {
            printf("Introduzca un numero entre %d y %d.\n", minimo, maximo);
            continue;
        }

        return (int)numero;
    }
}

// ============================================================
// FUNCIONES AUXILIARES DE MONGODB
// ============================================================

// Convierte un documento BSON a JSON y lo imprime.
void imprimirDocumento(const bson_t *documento) {

    char *json = bson_as_canonical_extended_json(documento, NULL);

    if (json != NULL) {
        printf("%s\n", json);

        // La conversión reserva memoria para el texto JSON.
        // Se debe liberar cuando ya no se necesite.
        bson_free(json);
    }
}

// Comprueba si existe un personaje con el nombre indicado.
bool existePersonaje(mongoc_collection_t *coleccion, const char *nombre) {

    // Construye el filtro: {"nombre": nombre}
    bson_t *filtro = BCON_NEW("nombre", BCON_UTF8(nombre));

    // Ejecuta la consulta.
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(coleccion, filtro, NULL, NULL);

    const bson_t *documento;
    bson_error_t error;

    // Basta con encontrar un documento para confirmar existencia.
    bool existe = mongoc_cursor_next(cursor, &documento);

    if (mongoc_cursor_error(cursor, &error)) {printf("Error al consultar: %s\n", error.message);}

    // Libera el cursor y el filtro.
    mongoc_cursor_destroy(cursor);
    bson_destroy(filtro);

    return existe;
}

// ============================================================
// CREATE - AGREGAR PERSONAJE
// ============================================================

void agregarPersonaje(mongoc_collection_t *coleccion) {

    char nombre[MAX_TEXTO];
    char identidad[MAX_TEXTO];
    char equipo[MAX_TEXTO];
    char arma[MAX_TEXTO];

    printf("\n===== AGREGAR PERSONAJE =====\n");

    leerTexto("Nombre: ", nombre, sizeof(nombre));

    // Evita insertar un nombre que ya existe.
    if (existePersonaje(coleccion, nombre)) {printf("Ya existe un personaje con ese nombre.\n"); return;}

    leerTexto("Identidad: ", identidad, sizeof(identidad));
    leerTexto("Equipo: ", equipo, sizeof(equipo));

    int cantidad = leerEntero("Cantidad de armas (0 a 5): ", 0, MAX_ARMAS);

    // Crea un documento BSON vacío.
    bson_t *personaje = bson_new();

    // Agrega los atributos de tipo texto al documento.
    BSON_APPEND_UTF8(personaje, "nombre", nombre);
    BSON_APPEND_UTF8(personaje, "identidad", identidad);
    BSON_APPEND_UTF8(personaje, "equipo", equipo);

    // Los arreglos BSON se construyen como documentos hijos.
    bson_t armas;

    // Inicia el arreglo "armas" dentro del personaje.
    BSON_APPEND_ARRAY_BEGIN(personaje, "armas", &armas);

    for (int i = 0; i < cantidad; i++) {

        char mensaje[60]; snprintf(mensaje, sizeof(mensaje),"Arma %d: ", i + 1);
        leerTexto(mensaje, arma, sizeof(arma));

        // Los elementos de un arreglo BSON utilizan claves
        // numéricas representadas como texto: "0", "1", etc.
        char indice[12];
        snprintf(indice, sizeof(indice), "%d", i);

        BSON_APPEND_UTF8(&armas, indice, arma);
    }

    // Finaliza el arreglo y lo incorpora al documento padre.
    bson_append_array_end(personaje, &armas);

    bson_error_t error;

    // Envía el documento a MongoDB para insertarlo.
    bool insertado = mongoc_collection_insert_one(coleccion, personaje, NULL, NULL, &error);

    if (insertado) {
        printf("\nPersonaje agregado correctamente.\n");
    } else {
        printf("Error al insertar: %s\n", error.message);
    }

    // Libera el documento BSON creado en memoria.
    bson_destroy(personaje);
}

// ============================================================
// READ - MOSTRAR TODOS LOS PERSONAJES
// ============================================================

void mostrarPersonajes(mongoc_collection_t *coleccion) {

    printf("\n===== LISTA DE PERSONAJES =====\n");

    // Un documento BSON vacío funciona como filtro sin condiciones.
    bson_t *filtro = bson_new();

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(coleccion, filtro, NULL, NULL);

    const bson_t *documento;
    bson_error_t error;
    int contador = 0;

    // Recorre todos los documentos encontrados.
    while (mongoc_cursor_next(cursor, &documento)) {
        contador++;

        printf("\nPersonaje %d:\n", contador);
        imprimirDocumento(documento);
    }

    if (mongoc_cursor_error(cursor, &error)) {
        printf("Error en la consulta: %s\n", error.message);
    } else if (contador == 0) {
        printf("No hay personajes registrados.\n");
    } else {
        printf("\nTotal de personajes: %d\n", contador);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(filtro);
}

// ============================================================
// READ - BUSCAR PERSONAJE
// ============================================================

void buscarPersonaje(mongoc_collection_t *coleccion) {

    char nombre[MAX_TEXTO];

    printf("\n===== BUSCAR PERSONAJE =====\n");

    leerTexto("Nombre del personaje: ", nombre, sizeof(nombre));

    // Filtra los documentos cuyo campo nombre coincida.
    bson_t *filtro = BCON_NEW("nombre", BCON_UTF8(nombre));

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(coleccion, filtro, NULL, NULL);

    const bson_t *documento;
    bson_error_t error;
    bool encontrado = false;

    while (mongoc_cursor_next(cursor, &documento)) {
        encontrado = true;

        printf("\nPersonaje encontrado:\n");
        imprimirDocumento(documento);
    }

    if (mongoc_cursor_error(cursor, &error)) {
        printf("Error: %s\n", error.message);
    } else if (!encontrado) {
        printf("No se encontro el personaje.\n");
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(filtro);
}

// ============================================================
// UPDATE - ACTUALIZAR PERSONAJE
// ============================================================

void actualizarPersonaje(mongoc_collection_t *coleccion) {

    char nombre[MAX_TEXTO];
    char nuevoValor[MAX_TEXTO];

    printf("\n===== ACTUALIZAR PERSONAJE =====\n");

    leerTexto("Nombre del personaje a actualizar: ", nombre, sizeof(nombre));

    if (!existePersonaje(coleccion, nombre)) {
        printf("El personaje no existe.\n");
        return;
    }

    printf("\nQue desea actualizar?\n");
    printf("1. Nombre\n");
    printf("2. Identidad\n");
    printf("3. Equipo\n");

    int opcion = leerEntero("Seleccione una opcion: ", 1, 3);

    // Determina el campo del documento que se quiere modificar.
    const char *campo;

    switch (opcion) {
        case 1:
            campo = "nombre";
            break;

        case 2:
            campo = "identidad";
            break;

        default:
            campo = "equipo";
            break;
    }

    leerTexto("Nuevo valor: ", nuevoValor, sizeof(nuevoValor));

    // Si cambia el nombre, comprobamos que no esté ocupado.
    if (opcion == 1 && strcmp(nombre, nuevoValor) != 0 && existePersonaje(coleccion, nuevoValor)) {
        printf("Ya existe un personaje con ese nombre.\n");
        return;
    }

    // Selecciona el documento que queremos modificar.
    bson_t *filtro = BCON_NEW("nombre", BCON_UTF8(nombre));

    // Construye la actualización utilizando el operador $set.
    //
    // Ejemplo:
    // {"$set": {"equipo": "Avengers"}}
    //
    // $set modifica únicamente el campo indicado,
    // conservando los demás atributos del documento.
    bson_t *actualizacion = bson_new();
    bson_t cambios;

    BSON_APPEND_DOCUMENT_BEGIN(
        actualizacion, "$set", &cambios
    );

    BSON_APPEND_UTF8(&cambios, campo, nuevoValor);

    bson_append_document_end(actualizacion, &cambios);

    bson_error_t error;
    bson_t respuesta = BSON_INITIALIZER;

    // Actualiza un solo documento que cumpla el filtro.
    bool actualizado = mongoc_collection_update_one(
        coleccion, filtro, actualizacion,
        NULL, &respuesta, &error
    );

    if (actualizado) {
        printf("Operacion de actualizacion ejecutada.\n");
    } else {
        printf("Error al actualizar: %s\n", error.message);
    }

    bson_destroy(&respuesta);
    bson_destroy(actualizacion);
    bson_destroy(filtro);
}

// ============================================================
// DELETE - ELIMINAR PERSONAJE
// ============================================================

void eliminarPersonaje(mongoc_collection_t *coleccion) {

    char nombre[MAX_TEXTO];

    printf("\n===== ELIMINAR PERSONAJE =====\n");

    leerTexto("Nombre del personaje a eliminar: ", nombre, sizeof(nombre));

    if (!existePersonaje(coleccion, nombre)) {
        printf("El personaje no existe.\n");
        return;
    }

    printf("\nEsta seguro de eliminar a %s?\n", nombre);
    printf("1. Si\n");
    printf("2. No\n");

    int confirmacion = leerEntero( "Seleccione una opcion: ", 1, 2);

    if (confirmacion == 2) {
        printf("Eliminacion cancelada.\n");
        return;
    }

    // Filtro que identifica el documento que se eliminará.
    bson_t *filtro = BCON_NEW("nombre", BCON_UTF8(nombre));

    bson_error_t error;
    bson_t respuesta = BSON_INITIALIZER;

    // Elimina un documento que coincida con el filtro.
    bool eliminado = mongoc_collection_delete_one(
        coleccion, filtro, NULL, &respuesta, &error
    );

    if (eliminado) {
        printf("Operacion de eliminacion ejecutada.\n");
    } else {
        printf("Error al eliminar: %s\n",  error.message);
    }

    bson_destroy(&respuesta);
    bson_destroy(filtro);
}