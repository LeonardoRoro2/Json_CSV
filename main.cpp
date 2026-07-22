#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"
#include "json.hpp"

using json = nlohmann::json;

// Modificar la estructura de los datos de arbol a tabla
void aplanar_Json( const json& j, const std::string& prefijo, std::map<std::string, std::string>& fila_datos ) {

    // Objeto
    if ( j.is_object() ) {

        for ( auto& elemento : j.items() ) {

            std::string nuevo_prefijo = prefijo.empty() ? elemento.key() : prefijo + "_" + elemento.key();
            aplanar_Json( elemento.value(), nuevo_prefijo, fila_datos );

        }

    // Arreglo
    } else if ( j.is_array() ) {

        int indice = 0;
        for ( auto& elemento : j ) {

            std::string nuevo_prefijo = prefijo.empty() ? std::to_string( indice ) : prefijo + "_" + std::to_string( indice );
            aplanar_Json( elemento, nuevo_prefijo, fila_datos );
            indice++;

        }

    } else {

        if ( j.is_string() ) {

            fila_datos[ prefijo ] = "\"" + j.get< std::string >() + "\"";

        } else {

            fila_datos[ prefijo ] = j.dump();

        }

    }

}

// Creacion del archivo csv con los datos procesados
void crear_CSV( std::string ruta, const std::vector<std::map<std::string, std::string>>& todos_los_usuarios, const std::set<std::string>& todas_las_columnas ) {

    // Cambia la extensión de .json a .csv
    size_t posicion = ruta.find( '.' );

    if ( posicion != std::string::npos ) {

        ruta.erase( posicion );

    }

    // Crea el archivo
    std::ofstream archivo_salida(ruta + ".csv");

    if ( !archivo_salida.is_open() ) {

        pfd::message( "Error", "Error al crear el archivo CSV.", pfd::choice::ok, pfd::icon::error );
        std::cerr << "Error al crear el archivo CSV." << std::endl;
        return;

    }

    // Escribe la cabecera
    archivo_salida << "ID_Usuario";

    if ( !todas_las_columnas.empty() ) archivo_salida << ",";

    for ( auto it = todas_las_columnas.begin(); it != todas_las_columnas.end(); ++it ) {
        archivo_salida << *it;
        if ( std::next( it ) != todas_las_columnas.end() ) {

            archivo_salida << ",";

        }
    }
    archivo_salida << "\n";

    // Escribe los datos de los usuarios
    for ( const auto& fila : todos_los_usuarios ) {

        auto id_it = fila.find( "ID_Usuario" );

        if ( id_it != fila.end() ) {

            archivo_salida << id_it->second;

        }
        if ( !todas_las_columnas.empty() ) archivo_salida << ",";

        for ( auto it = todas_las_columnas.begin(); it != todas_las_columnas.end(); ++it ) {
            auto valor_encontrado = fila.find( *it );
            if ( valor_encontrado != fila.end() ) {

                archivo_salida << valor_encontrado->second;

            }

            if ( std::next( it ) != todas_las_columnas.end() ) {

                archivo_salida << ",";

            }

        }

        archivo_salida << "\n";

    }

    // Mensaje
    pfd::message ( "Exito", "¡Archivo CSV generado exitosamente!", pfd::choice::ok, pfd::icon::info );

}

// Manejo del archivo y los datos
void procesar_Datos( std::string& ruta )
{

    // Lee el archivo
    if ( std::ifstream archivo_entrada( ruta ); archivo_entrada.is_open() ) {

            json datos;
            archivo_entrada >> datos;

            // Vector para almacenar las tuplas de la tabla (Los usuarios con sus respectivos datos)
            std::vector<std::map<std::string, std::string>> todos_los_usuarios;
            // Conjunto para recopilar y ordenar las columnas (por medio de llaves unicas para evitar los duplicados)
            std::set<std::string> todas_las_columnas;

            // Llena la "tabla" del archivo csv
            for ( auto& usuario : datos.items() ) {

                const std::string& id_usuario = usuario.key();
                // Para guardar los datos del usuario actual
                std::map< std::string, std::string > fila_actual;

                // Pone el id del usuario en la primera columna
                fila_actual[ "ID_Usuario" ] = "\"" + id_usuario + "\"";

                // Convierte los datos anidados a pares llave-valor
                aplanar_Json( usuario.value(), "", fila_actual );

                // Guarda las columnas para evitar perder datos que unos usuarios tengan y otros no
                for ( const auto &key: fila_actual | std::views::keys ) {

                    if ( key != "ID_Usuario" ) {

                        todas_las_columnas.insert( key );

                    }

                }

                // Guarda la tupla en el vector (Los datos del usuario)
                todos_los_usuarios.push_back( fila_actual );

            }

            // Funcion para crear el archivo CSV
            crear_CSV( ruta, todos_los_usuarios, todas_las_columnas );

        } else {

            pfd::message( "Error", "No se pudo leer el archivo.", pfd::choice::ok, pfd::icon::error );

        }

}

// Seleccionar el archivo y lo convierte
void seleccionar_Archivo() {

    // Abre la ventana y espera al archivo elegido
    auto selecciones = pfd::open_file( "Abrir archivo de codigo", ".",
                                      { "Archivos JSON", "*.json", "Todos los archivos", "*" } ).result();

    // Verifica si se eligió algo
    if ( !selecciones.empty() ) {

        std::string& ruta = selecciones[ 0 ];

        // Convierte el archivo JSON a CSV
        procesar_Datos( ruta );

    }
}

// Ventana con los datos del software
void acerca_De() {

    pfd::message ( "Acerca del Software",
        "Desarrollado por: \n Hola",
        pfd::choice::ok, pfd::icon::info );

}

int main() {

    glfwInit();
    GLFWwindow* window = glfwCreateWindow( 640, 360, "JSON a CSV", nullptr, nullptr );
    glfwMakeContextCurrent( window );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 130" );

    // Bucle principal
    while ( !glfwWindowShouldClose( window ) ) {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin( "JSON a CSV" );

        // Botones
        if ( ImGui::Button( "Convertir Archivo", ImVec2( 180, 30 ) ) ) {

            seleccionar_Archivo();

        }

        ImGui::SameLine();
        if ( ImGui::Button( "Acerca De...", ImVec2( 180, 30 ) ) ) {

            acerca_De();

        }

        ImGui::End();

        // Renderizado
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }

    // Limpieza
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;

}
