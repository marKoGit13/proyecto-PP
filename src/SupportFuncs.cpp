#include "SupportFuncs.h"
float NumberRandomizer(bool flag, float RI, float RS){
    std::random_device rd; //Cosas de semillas
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribucion(RI, RS); //Crea la distribución uniforme
    //Switch entre distribución Discreta o Continua
    float randomizer = 0.f;
    if (flag == true){
        randomizer = distribucion(gen); 
    }
    else{
        randomizer = std::round(distribucion(gen));
    }
    return randomizer;
}

std::tuple<int, std::string, std::string, std::string, std::string, float, float, float> ReadFromConfigFile(const std::string& file)
{
    std::ifstream input(file);
    if (!input.is_open())
    {
        spdlog::error("No se pudo abrir el archivo de configuración");
    }

    nlohmann::json config;
    input >> config;

    int count = config["cantidad"];
    std::string route1 = config["ubicacion_imagen_character"];
    std::string route2 = config["ubicacion_imagen_background"];
    std::string route3 = config["ubicacion_imagen_enemy"];
    std::string route4 = config["ubicacion_imagen_barrier"];
    float X = config["ancho"];
    float Y = config["alto"];
    float interval = config["interval"];
    
    return {count, route1, route2, route3, route4, X, Y, interval};
}