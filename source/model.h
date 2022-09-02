#pragma once
#include <string>
#include <array>
#include <vector>
#include <Windows.h>
#include <gl/GL.h>
#include "h2bParser.h"

using matrix = std::array<float, 16>;

const matrix identity =
{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

class Model
{
public:
	std::string name;
	H2B::Parser info;
	std::vector<matrix> worldMatrix;

	GLuint	vertex_array_object;
	GLuint	vertex_buffer_object;
	GLuint	index_buffer_object;

	Model();
	~Model();
	Model(const Model& copy);
	Model& operator=(const Model& copy);
	bool LoadFromFile(const char* szFileName);

};