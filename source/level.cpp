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

std::string Level::GetFileName(std::string file)
{
	std::string tokenize = file;
	std::string delim = "/\\";
	std::string submit = "";
	size_t first = 0;
	while (first < tokenize.size())
	{
		// find first delimiter
		size_t second = tokenize.find_first_of(delim, first);
		// check to see if it's the end of the string
		if (second == std::string::npos)
		{
			// set the second location to the size of the entire string
			second = tokenize.size();
		}
		// store the value of the tokenized string
		submit = tokenize.substr(first, second - first);
		// adjust the location to search
		first = second + 1;
	}

	// Remove extension
	delim = ".";
	first = submit.find_first_of(delim, 0);
	submit = submit.substr(0, first);
	return submit;
}

void Level::LoadMeshFromFile()
{
	char buffer[256] = { 0 };
	input.getline(buffer, 256);

	matrix world = ReadMatrixData();

	std::string assetName = GetFileName(buffer);
	std::string path = "../assets/" + assetName + ".h2b";

	bool found = false;
	auto& iter = models.begin();
	for (; iter != models.end(); iter++)
	{
		if (strcmp(iter->name.c_str(), path.c_str()) == 0)
		{
			found = true;
			break;
		}
	}
	if (found)
	{
		Model& m = *iter;
		m.worldMatrix.push_back(world);
	}
	else
	{
		Model m;
		m.LoadFromFile(path.c_str());
		m.worldMatrix.push_back(world);
		models.push_back(m);
	}
}

matrix Level::ReadMatrixData()
{
	char buffer[4][256] = { 0 };
	int r = -1;
	input.getline(buffer[0], 256);
	input.getline(buffer[1], 256);
	input.getline(buffer[2], 256);
	input.getline(buffer[3], 256);
	matrix m = { 0 };
	r = sscanf_s(buffer[0], "<Matrix 4x4 (%f, %f, %f, %f)", &m[0], &m[1], &m[2], &m[3]);
	r = sscanf_s(buffer[1], "            (%f, %f, %f, %f)", &m[4], &m[5], &m[6], &m[7]);
	r = sscanf_s(buffer[2], "            (%f, %f, %f, %f)", &m[8], &m[9], &m[10], &m[11]);
	r = sscanf_s(buffer[3], "            (%f, %f, %f, %f)>", &m[12], &m[13], &m[14], &m[15]);

	return m;
}

bool Level::LoadFromFile(const char* fileName)
{
	std::string fileToOpen = "../levels/" + GetFileName(fileName) + ".txt";
	input.open(fileToOpen.c_str(), std::ios_base::in);
	if (input.is_open())
	{
		name = GetFileName(fileName);
		while (!input.eof())
		{
			char buffer[256] = { 0 };
			input.getline(buffer, 256);
			if (strcmp(buffer, "MESH") == 0)
			{
				LoadMeshFromFile();
			}
		}
	}
	return true;
}
