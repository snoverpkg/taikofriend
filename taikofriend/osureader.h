#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <mutex>
#include <thread>

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
	NoteData noteData;
	MetaData metaData;
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
void writePaths(std::ofstream* file);

void threadPathLoader(const std::string& path, const std::string& ext, const std::regex& regexFilter);
void fullProcess();
void differentialProcess();
void loadPaths();

//we need to exterminate undesirable characters
std::string stringCleaner(std::string string);

std::string chartFinder(MetaData zoinkers);

Score readScore(std::string scoreData);

std::string getPathFromTable(int index);