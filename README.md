# Marvel Database

Decidimos utilizar una base de datos NoSQL orientada a documentos usando MongoDB y desarrollamos un programa en C que permite almacenar y administrar información de personajes de Marvel mediante diferentes operaciones, como agregar, consultar, actualizar y eliminar personajes.

## Escenario (Problema)

Se necesita almacenar información de personajes de Marvel, incluyendo su nombre, identidad, equipo y armas, los personajes pueden tener características diferentes, por lo que resulta conveniente utilizar una base de datos que permita manejar documentos con estructuras flexibles.

## Justificación de MongoDB

Se eligió MongoDB porque es una base de datos NoSQL orientada a documentos, lo que permite almacenar la información de cada personaje mediante campos y valores, de manera similar a JSON.

En nuestro programa, cada personaje se guarda como un documento dentro de la colección `personajes`, perteneciente a la base de datos `marvel_db`. Para interactuar con estos documentos, utilizamos el controlador de MongoDB para C, que nos permite agregar, consultar, actualizar y eliminar información directamente desde el código.

## Funcionalidades del programa

El programa cuenta con un menú en consola que permite realizar las siguientes operaciones:

1. **Agregar personaje:** permite registrar un personaje indicando su nombre, identidad, equipo y armas.
2. **Mostrar personajes:** muestra todos los personajes almacenados en la base de datos.
3. **Buscar personaje:** permite consultar la información de un personaje mediante su nombre.
4. **Actualizar personaje:** permite modificar el nombre, la identidad o el equipo de un personaje existente.
5. **Eliminar personaje:** permite eliminar un personaje de la base de datos, solicitando una confirmación antes de hacerlo.

Todas estas operaciones se realizan directamente sobre la colección de MongoDB, por lo que los cambios permanecen guardados incluso después de cerrar el programa.

Aunque el quiz solicita desarrollar un programa pequeño, decidimos ampliar un poco sus funcionalidades para hacerlo más completo, quizás las opciones de actualizar y eliminar personajes no eran estrictamente necesarias para demostrar el uso de MongoDB, pero consideramos que incluirlas permitiría trabajar con más operaciones sobre los documentos almacenados.

## Configuración del proyecto

Para desarrollar el programa utilizamos C17, CLion y MongoDB Community Server como base de datos local, además, instalamos MongoDB Compass para visualizar los documentos y el controlador MongoDB C Driver mediante vcpkg, para permitir la comunicación entre el programa en C y la base de datos.

La conexión se realiza mediante la dirección `mongodb://localhost:27017/`, utilizando la base de datos `marvel_db` y la colección `personajes`.

El repositorio también incluye el archivo `personajes.json`, que contiene una exportación de los personajes registrados, este archivo permite visualizar los datos desde GitHub e importarlos a MongoDB sin necesidad de agregar nuevamente cada personaje de forma manual.

## Ejecución del programa

Para ejecutar el programa, primero se debe tener MongoDB Community Server instalado y en funcionamiento, junto con el controlador MongoDB C Driver configurado en el proyecto.

1. Descargar o clonar el repositorio de GitHub.
2. Abrir el proyecto en CLion y configurar las dependencias necesarias mediante CMake y vcpkg.
3. En MongoDB Compass, crear la base de datos `marvel_db` y la colección `personajes`.
4. Importar el archivo `personajes.json` en la colección, si se desea utilizar los personajes de ejemplo.
5. Compilar y ejecutar el programa desde CLion.

Una vez iniciado, se mostrará un menú en la consola desde el cual se pueden realizar las diferentes operaciones sobre los personajes almacenados en MongoDB.

## Uso de herramientas de inteligencia artificial

Durante el desarrollo del proyecto se utilizó ChatGPT como herramienta de apoyo para la instalación y configuración de MongoDB Community Server, MongoDB Compass, CMake, vcpkg y las dependencias necesarias para conectar el programa en C con la base de datos.

También se utilizó como apoyo para comprender el funcionamiento de estas herramientas y resolver problemas que surgieron durante la configuración y el desarrollo del programa.