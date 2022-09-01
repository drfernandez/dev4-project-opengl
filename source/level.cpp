#include "level.h"

Level::Level()
{
	name = "";
	models.clear();
}

Level::~Level()
{
	name = "";
	models.clear();
}

Level::Level(const Level& copy)
{
	*this = copy;
}

Level& Level::operator=(const Level& copy)
{
	if (this != &copy)
	{
		name = copy.name;
		models = copy.models;
	}
	return *this;
}

bool Level::LoadFromFile(const char* fileName)
{
	Model test; 
	bool success = test.LoadFromFile("../assets/shibainu.h2b");
	if (success) 
	{
		models.push_back(test);
	}
	return success;
}