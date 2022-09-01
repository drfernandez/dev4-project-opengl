#pragma once
#include <string>
#include <vector>
#include "model.h"

class Level
{
public:
	std::string name;
	std::vector<Model> models;

	Level();
	~Level();
	Level(const Level& copy);
	Level& operator=(const Level& copy);
	bool LoadFromFile(const char* fileName);
};