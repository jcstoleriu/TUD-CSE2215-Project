#include "sparseMatrix.h"
#include <map>
#include <iostream>

SparseMatrix::SparseMatrix(uint16_t n, uint16_t m) {
	this->n = n;
	this->m = m;
	this->matrix = std::map<std::tuple<uint16_t, uint16_t>, std::tuple<glm::vec3, glm::vec3>>();
}

SparseMatrix::SparseMatrix(uint16_t n) {
	SparseMatrix(n, n);
}

std::tuple<glm::vec3, glm::vec3> SparseMatrix::get(uint16_t i, uint16_t j) {
	std::tuple<uint16_t, uint16_t> key = std::tuple(i, j);
	// key exists
	if (matrix.count(key) > 0)
		return matrix.at(key);
	
	// default scale is 1, default offset is 0
	if (i < n && j < m && i >= 0 && j >= 0)
		return std::tuple(glm::vec3(1.0), glm::vec3(0.0));

	throw std::out_of_range("Index out of range");
}

std::vector<std::tuple<glm::vec3, glm::vec3>> SparseMatrix::getRow(uint16_t i) {
	if (i >= n || i < 0)
		throw std::out_of_range("Index out of range");

	std::vector<std::tuple<glm::vec3, glm::vec3>> row;
	for (int j = 0; j < m; j++)
		row.push_back(get(i, j));

	return row;
}

void SparseMatrix::set(uint16_t i, uint16_t j, std::tuple<glm::vec3, glm::vec3> val) {
	if (i < n && j < m && i >= 0 && j >= 0) {
		matrix[std::tuple(i, j)] = val;
		return;
	}
	throw std::out_of_range("Index out of range");
}