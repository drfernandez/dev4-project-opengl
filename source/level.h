#pragma once
#include <string>
#include <vector>
#include "model.h"
#include <fstream>

class Level
{
public:
	std::string name;
	std::vector<Model> models;
	std::ifstream input;

	Level();
	~Level();
	Level(const Level& copy);
	Level& operator=(const Level& copy);
	bool LoadFromFile(const char* fileName);
private:
	std::string GetFileName(std::string file);
	void LoadMeshFromFile();
	matrix ReadMatrixData();
};