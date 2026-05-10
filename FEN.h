#pragma once

#include <string>

struct Position;

Position ParseFEN(const std::string &str);
