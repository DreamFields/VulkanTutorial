#pragma once

#define GLM_FORCE_RADIANS
#include <glm.hpp>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

glm::vec3 RodriguesRotation(glm::vec3 v, float teta, glm::vec3 k);
// glm::dvec3 RodriguesRotation(glm::dvec3 v, double teta, glm::dvec3 k);

std::vector<std::string> GetAllFilesInDirectory(const std::string& directoryPath);