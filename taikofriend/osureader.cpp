#include "osureader.h"
#include "taikocalc.h"

//i copy pasted this from an old project pls no judge it works
Chart chartReader(std::string path, bool processNotes) {
    Chart chart;

    if (path == "failed") return chart;

    std::vector<std::string> chartInfo;
    std::vector<std::string> curNoteData;

    std::string line;
    std::string x;
    bool readNotes = false;
    bool readInfo = false;
    int curLine = 0;
    bool checkBeatmapID = false;

    std::ifstream file(path);
    try {
        if (file.is_open() && file.peek() != EOF) {
            while (getline(file, line)) {
                if (line.size() > 1) {
                    if (line == "[Metadata]") {
                        readInfo = true;
                    }
                    if (line == "[Events]") {
                        readInfo = false;
                    }
                    if (line == "[HitObjects]") {
                        readNotes = true;
                        getline(file, line);
                    }
                    if (readNotes && processNotes) {
                        curNoteData.clear();
                        std::stringstream s(line);
                        while (getline(s, x, ',')) {
                            curNoteData.push_back(x);
                        }
                        if (line.length() > 0) {
                            chart.NoteData.NoteInfo.push_back(std::make_pair(std::stoi(curNoteData[2]), std::stoi(curNoteData[4])));
                        }
                    }
                    if (readInfo && line != "[Difficulty]") {
                        std::stringstream s(line);
                        while (getline(s, x, ':')) {
                            if (x == "Title" || x == "Artist" || x == "Creator" || x == "Version" || x == "BeatmapID" || x == "OverallDifficulty") {
                                if (checkBeatmapID) {
                                    if (x == "OverallDifficulty") {
                                        chartInfo.push_back("-1");
                                    }
                                    checkBeatmapID = false;
                                }
                                if (x == "Version") {
                                    checkBeatmapID = true;
                                }
                                getline(s, x, ':');
                                std::remove(x.begin(), x.end(), ';');
                                chartInfo.push_back(x);
                            }
                        }
                    }
                }
            }
        }
        else {
            return chart;
        }
    }
    catch(...){
        std::cout << "invalid .osu file" << std::endl;
        file.close();
        return chart;
    }
    file.close();

    if (chartInfo.size() == 6) {
        try {
            chart.MetaData.title = '"' + chartInfo[0] + '"';
            chart.MetaData.artist = '"' + chartInfo[1] + '"';
            chart.MetaData.creator = '"' + chartInfo[2] + '"';
            chart.MetaData.diff = '"' + chartInfo[3] + '"';
            chart.MetaData.id = std::stoi(chartInfo[4]);
            chart.MetaData.od = std::stod(chartInfo[5]);
        }
        catch (...) {  
        }
    }
    else {
        std::cout << "unable to process beatmap metadata or invalid .osu file" << std::endl;
    }

    return chart;
}

void noteInterpreter(Chart* c) {
    for (int i = 0; i < c->NoteData.NoteInfo.size(); i++) {
        bool color1 = c->NoteData.NoteInfo[i].second & 8;
        bool color2 = c->NoteData.NoteInfo[i].second & 2;
        bool color = color1 || color2;
        bool big = c->NoteData.NoteInfo[i].second & 4;
        unsigned int note = color + (2 * big);
        c->NoteData.NoteInfo[i].second = note;
    }
}

static std::string path;
static std::string ext(".osu");

void setPath(std::string _path) {
    path = _path;
}

static std::vector<std::string> paths;
std::mutex mtx;

void writePaths(std::ofstream* file) {
    for (auto c : paths) {
        Chart chart = chartReader(c, false);
        std::string chartInfo = chart.MetaData.artist + chart.MetaData.title + chart.MetaData.creator + chart.MetaData.diff;

        if (file->is_open()) {
            *file << chartInfo << std::endl << c << std::endl;
        }

        chartPathsLookupTable.insert(std::make_pair(chartInfo, c));
    }
}

void threadPathLoader(const std::string& path, const std::string& ext, const std::regex& regexFilter) {
    for (const auto& p : std::filesystem::recursive_directory_iterator(path)) {
        if (p.path().extension() == ext) {
            try {
                std::string folderName = p.path().parent_path().filename().string();
                if (std::regex_match(folderName, regexFilter)) {
                    const std::lock_guard<std::mutex> guard(mtx);
                    paths.push_back(p.path().string());
                }
            }
            catch (...) {
                continue;
            }
        }
    }
}

//processes all files
void fullProcess() {
    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) threadCount = 1;
    std::vector<std::regex> regexFilters;
    std::vector<std::thread> threads;

    //code worth shoving a fork in the electrical outlet
    if (threadCount <= 3) {
        regexFilters = { std::regex(".*") };
    }
    else if (threadCount >= 4 && threadCount <= 7) {
        regexFilters = {
            std::regex("^[0-3].*"),
            std::regex("^[4-6].*"),
            std::regex("^[7-9].*"),
            std::regex("^[^0-9].*")
        };
    }
    else {
        regexFilters = {
            std::regex("^[0-1].*"),
            std::regex("^[2].*"),
            std::regex("^[3].*"),
            std::regex("^[4].*"),
            std::regex("^[5].*"),
            std::regex("^[6-7].*"),
            std::regex("^[8-9].*"),
            std::regex("^[^0-9].*") };
    }

    size_t threadsToUse = regexFilters.size();
    chartPathsLookupTable.clear();

    for (size_t i = 0; i < threadsToUse; ++i) {
        threads.emplace_back(threadPathLoader, path, ext, regexFilters[i % threadsToUse]);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::ofstream pathFile("paths.txt");
    writePaths(&pathFile);
    pathFile.close();
    paths.clear();
}

void differentialProcess() {
    for (auto& p : std::filesystem::recursive_directory_iterator(path))
    {
        if (p.path().extension() == ext) {
            try {
                bool found = false;
                std::string pathString = p.path().string();
                for (auto& e : chartPathsLookupTable) {
                    if (pathString == e.second) {
                        found = true;
                        break;
                    }
                }
                if (found) continue;
                paths.push_back(pathString);
            }
            catch (...) {
                continue;
            }
        }
    }

    std::ofstream pathFile("paths.txt", std::ios::app);
    writePaths(&pathFile);
    pathFile.close();
    paths.clear();
}

void loadPaths() {
    std::string line;
    std::ifstream pathFile("paths.txt");
    if (!pathFile.is_open()) {
        std::cout << "failed to open path file" << std::endl;
        return;
    }
    while (getline(pathFile, line)) {
        if (!(line.size() > 1)) {
            getline(pathFile, line);
            continue;
        }
        std::string chartMetadata = line;
        getline(pathFile, line);
        std::string chartPath = line;
        chartPathsLookupTable.insert(std::make_pair(chartMetadata, chartPath));
    }
}

std::string chartFinder(MetaData zoinkers) {
    std::string path;
    std::string search = zoinkers.artist + zoinkers.title + zoinkers.creator + zoinkers.diff;
    std::unordered_map<std::string, std::string>::iterator find = chartPathsLookupTable.find(search);
    if (find == chartPathsLookupTable.end()) return "failed";
    path = find->second;
    return path;
}

Score readScore(std::string scoreData) {
    Score score;
    std::vector<std::string> metadata;

    std::string rx_string = "(\"[^\"]*\"|[^,]*)(?:,|$)";
    std::regex e(rx_string);
    std::regex_iterator<std::string::iterator> rit(scoreData.begin(), scoreData.end(), e);
    std::regex_iterator<std::string::iterator> rend;
    std::string iterString;

    while (rit != rend)
    {
        iterString = rit->str();
        if (iterString[iterString.size() - 1] == ',') iterString.pop_back();
        metadata.push_back(iterString);
        ++rit;
    }

    if (metadata.size() != 11) {
        std::cout << "unable to find beatmap data for this score: ";
        std::cout << scoreData << std::endl;
        return score;
    }

    score.Chart.MetaData.artist = metadata[0];
    score.Chart.MetaData.title = metadata[1];
    score.Chart.MetaData.creator = metadata[2];
    score.Chart.MetaData.diff = metadata[3];
    score.Chart.MetaData.id = std::stoi(metadata[4]);
    score.Date = metadata[5];
    score.ModString = metadata[6];
    score.Acc = std::stof(metadata[7]);
    score.Rating = std::stof(metadata[8]);
    score.CalcVer = std::stoi(metadata[9]);

    if (score.ModString.find("EZ") != std::string::npos) score.Mods += Mods::EZ;
    if (score.ModString.find("HR") != std::string::npos) score.Mods += Mods::HR;
    if (score.ModString.find("HT") != std::string::npos) score.Mods += Mods::HT;
    if (score.ModString.find("DT") != std::string::npos) score.Mods += Mods::DT;

    return score;
}