#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>

struct NoteData {
	std::vector<std::pair<unsigned int, unsigned int>> NoteInfo;
	std::vector<float> NoteMS;
	std::vector<float> adjMS;
	std::vector<float> baseDiffs;
	std::vector<float> adj_diffs;
};

struct MetaData {
	std::string title;
	std::string artist;
	std::string creator;
	std::string diff;
	int id;
	double od;
};

struct Chart {
	NoteData NoteData;
	MetaData MetaData;
};

struct Score {
	Chart Chart;
	float Acc;
	int Mods = 0;
	std::string ModString;
	std::string Date;
	int CalcVer;
	float Rating = 0;
};

static std::unordered_map<std::string, std::string> chartPathsLookupTable;

Chart chartReader(std::string path, bool processNotes);
void noteInterpreter(Chart* c);

void setPath(std::string path);
void fullProcess();
void differentialProcess();
void loadPaths();

std::string chartFinder(MetaData zoinkers);

Score readScore(std::string scoreData);