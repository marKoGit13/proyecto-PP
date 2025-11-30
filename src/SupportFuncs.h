#pragma once
#include <tuple>
#include <string>
#include <random>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include <fstream>

std::tuple<int, std::string, std::string, std::string, std::string, float, float, float>ReadFromConfigFile(const std::string& file);
float NumberRandomizer(bool flag, float RI, float RS);