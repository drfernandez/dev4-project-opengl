#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <gl/GL.h>
#include "h2bParser.h"


class Model
{
public:
	std::string name;
	H2B::Parser info;

	GLuint	vertex_array_object;
	GLuint	vertex_buffer_object;
	GLuint	index_buffer_object;

	Model();
	~Model();
	Model(const Model& copy);
	Model& operator=(const Model& copy);
	bool LoadFromFile(const char* szFileName);

};