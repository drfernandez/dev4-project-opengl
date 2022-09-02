#include "model.h"

Model::Model()
{
	name = "";
	info.Clear();
	worldMatrix.clear();
	vertex_array_object = 0;
	vertex_buffer_object = 0;
	index_buffer_object = 0;
}

Model::~Model()
{
	name = "";
	info.Clear();
	worldMatrix.clear();
	vertex_array_object = 0;
	vertex_buffer_object = 0;
	index_buffer_object = 0;
}

Model::Model(const Model& copy)
{
	*this = copy;
}

Model& Model::operator=(const Model& copy)
{
	if (this != &copy)
	{
		name = copy.name;
		info = copy.info;	// deep copy texture names from material??
		worldMatrix = copy.worldMatrix;
		vertex_array_object = copy.vertex_array_object;
		vertex_buffer_object = copy.vertex_buffer_object;
		index_buffer_object = copy.index_buffer_object;
	}
	return *this;
}

bool Model::LoadFromFile(const char* fileName)
{
	name.append(fileName);
	info.Parse(fileName);
	return true;
}
