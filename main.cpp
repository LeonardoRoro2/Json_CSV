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

void aplanar_json(const json& j, const std::string& prefijo, std::map<std::string, std::string>& fila_datos) {
    if (j.is_object()) {
        for (auto& elemento : j.items()) {
            std::string nuevo_prefijo = prefijo.empty() ? elemento.key() : prefijo + "_" + elemento.key();
            aplanar_json(elemento.value(), nuevo_prefijo, fila_datos);
        }
    } else {
        if (j.is_string()) {
            fila_datos[prefijo] = "\"" + j.get<std::string>() + "\"";
        } else {
            fila_datos[prefijo] = j.dump();
        }
    }
}

void Abrir() {
    auto selecciones = pfd::open_file( "Abrir archivo de codigo", ".",
                                      { "Archivos JSON", "*.json", "Todos los archivos", "*" } ).result();

    if ( !selecciones.empty() ) {
        std::string& ruta = selecciones[ 0 ];

        if ( std::ifstream archivo_entrada( ruta ); archivo_entrada.is_open() ) {

            json datos;
            archivo_entrada >> datos;

            std::vector<std::map<std::string, std::string>> todos_los_usuarios;
            std::set<std::string> todas_las_columnas;

            for (auto& usuario : datos.items()) {
                const std::string& id_usuario = usuario.key();
                std::map<std::string, std::string> fila_actual;

                fila_actual["ID_Usuario"] = "\"" + id_usuario + "\"";

                aplanar_json(usuario.value(), "", fila_actual);

                for (const auto &key: fila_actual | std::views::keys) {
                    if (key != "ID_Usuario") {
                        todas_las_columnas.insert(key);
                    }
                }

                todos_los_usuarios.push_back(fila_actual);
            }

            size_t posicion = ruta.find('.');

            if (posicion != std::string::npos) {
                ruta.erase(posicion);
            }

            std::ofstream archivo_salida(ruta + ".csv");
            if (!archivo_salida.is_open()) {
                pfd::message( "Error", "Error al crear el archivo CSV.", pfd::choice::ok, pfd::icon::error );
                std::cerr << "Error al crear el archivo CSV." << std::endl;
                return;
            }

            archivo_salida << "ID_Usuario";
            if (!todas_las_columnas.empty()) archivo_salida << ",";

            for (auto it = todas_las_columnas.begin(); it != todas_las_columnas.end(); ++it) {
                archivo_salida << *it;
                if (std::next(it) != todas_las_columnas.end()) {
                    archivo_salida << ",";
                }
            }
            archivo_salida << "\n";

            for (const auto& fila : todos_los_usuarios) {
                auto id_it = fila.find("ID_Usuario");
                if (id_it != fila.end()) {
                    archivo_salida << id_it->second;
                }
                if (!todas_las_columnas.empty()) archivo_salida << ",";

                for (auto it = todas_las_columnas.begin(); it != todas_las_columnas.end(); ++it) {
                    auto valor_encontrado = fila.find(*it);
                    if (valor_encontrado != fila.end()) {
                        archivo_salida << valor_encontrado->second;
                    }

                    if (std::next(it) != todas_las_columnas.end()) {
                        archivo_salida << ",";
                    }
                }
                archivo_salida << "\n";
            }

            pfd::message ( "Exito", "¡Archivo CSV generado exitosamente!", pfd::choice::ok, pfd::icon::info );

        } else {
            pfd::message( "Error", "No se pudo leer el archivo.", pfd::choice::ok, pfd::icon::error );
        }

    }
}

void About() {

    pfd::message ( "Acerca del Software",
        "Desarrollado por: \n Hola",
        pfd::choice::ok, pfd::icon::info );

}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(640, 360, "JSON a CSV", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("JSON a CSV");

        // Botones
        if (ImGui::Button("Convertir Archivo", ImVec2(180, 30))) {
            Abrir();
        }
        ImGui::SameLine();
        if (ImGui::Button("Acerca De...", ImVec2(180, 30))) {
            About();
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
