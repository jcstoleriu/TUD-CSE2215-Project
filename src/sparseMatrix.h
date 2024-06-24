#pragma once

#include "disable_all_warnings.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <vector>
#include <map>

class SparseMatrix {
private:
	int n, m;
	// (row, col) -> (scale, offset) map to represent the matrix
	std::map<std::tuple<uint16_t, uint16_t>, std::tuple<glm::vec3, glm::vec3>> matrix;

public:
	SparseMatrix(uint16_t n, uint16_t m);
	SparseMatrix(uint16_t n);

	std::tuple<glm::vec3, glm::vec3> get(uint16_t i, uint16_t j);
	std::vector<std::tuple<glm::vec3, glm::vec3>> getRow(uint16_t i);

	void set(uint16_t i, uint16_t j, std::tuple<glm::vec3, glm::vec3> val);
	//void setRow(uint16_t i, std::vector<glm::vec3> val);
};