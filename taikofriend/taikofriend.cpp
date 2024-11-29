// FUCK MICROSOFT
// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important to understand: SDL_Renderer is an _optional_ component of SDL2.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#include "SDL.h"
#include "osureader.h"
#include "taikocalc.h"
#include <chrono>
#include <ctime>
#include "pmod.h"

const int programver = 4;

Chaos ChaosMod;
ConsecutiveDoubles CDMod;
Stamina StamMod;
StreamLengthBonus LengthMod;

std::string formatTime(std::time_t now) {
    struct std::tm tmTime = std::tm();
    localtime_s(&tmTime, &now);
    std::string mon = tmTime.tm_mon < 9 ? "0" + std::to_string(tmTime.tm_mon + 1) : std::to_string(tmTime.tm_mon + 1);
    std::string day = tmTime.tm_mday < 10 ? "0" + std::to_string(tmTime.tm_mday) : std::to_string(tmTime.tm_mday);
    std::string hour = tmTime.tm_hour < 10 ? "0" + std::to_string(tmTime.tm_hour) : std::to_string(tmTime.tm_hour);
    std::string min = tmTime.tm_min < 10 ? "0" + std::to_string(tmTime.tm_min) : std::to_string(tmTime.tm_min);
    std::string sec = tmTime.tm_sec < 10 ? "0" + std::to_string(tmTime.tm_sec) : std::to_string(tmTime.tm_sec);
    return std::to_string(tmTime.tm_year + 1900) + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
}

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// Main code
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("taikofriend", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static std::string artist;
    static std::string title;
    static std::string creator;
    static std::string diff;

    static const char* artistD;
    static const char* titleD;
    static const char* creatorD;
    static const char* diffD;

    MetaData currentWorkings;

    static std::string modString;

    bool ez = false;
    bool hr = false;
    bool ht = false;
    bool dt = false;

    float acc = 0.F;

    std::string chartPath;
    Chart chart;

    bool saveScore = true;

    float msdstyle = 0;
    float ppstyle = 0;
    float srstyle = 0;

    static char path[256];

    std::chrono::system_clock clock;

    loadPaths();

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("taikofriend");

            ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;

            if(ImGui::InputTextWithHint("Artist", "artist", &artist, flags)) currentWorkings.artist = '"' + artist + '"';
            if(ImGui::InputTextWithHint("Title", "title", &title, flags)) currentWorkings.title = '"' + title + '"';
            if(ImGui::InputTextWithHint("Creator", "creator", &creator, flags)) currentWorkings.creator = '"' + creator + '"';
            if(ImGui::InputTextWithHint("Difficulty", "diff", &diff, flags)) currentWorkings.diff = '"' + diff + '"';

            artistD = artist.c_str();
            titleD = title.c_str();
            creatorD = creator.c_str();
            diffD = diff.c_str();

            ImGui::Text("%s - %s (%s) [%s]", artistD, titleD, creatorD, diffD);

            ImGui::InputFloat("acc", &acc);

            ImGui::Checkbox("EZ", &ez);
            ImGui::SameLine();
            ImGui::Checkbox("HR", &hr);

            ImGui::Checkbox("HT", &ht);
            ImGui::SameLine();
            ImGui::Checkbox("DT", &dt);

            modString.clear();
            int mods = ez * Mods::EZ + hr * Mods::HR + ht * Mods::HT + dt * Mods::DT;
            if (ez) modString += "EZ";
            if (ht) modString += "HT";
            if (hr) modString += "HR";
            if (dt) modString += "DT";

            ImGui::SameLine();
            if (ImGui::Button("calc score")) {
                chartPath = chartFinder(currentWorkings);
                if (chartPath != "failed") {
                    chart = chartReader(chartPath, true);
                    Chart chart2 = chart;
                    msdstyle = calcMain(&chart, acc, (Mods)mods);
                    ppstyle = msdstyle * msdstyle * ppscaler;
                    srstyle = calcMain(&chart2, 0.95F, (Mods)mods) / 2;

                    if (saveScore) {
                        std::ofstream scoreFile("scores.csv", std::ios::app);
                        if (scoreFile.is_open()) {
                            scoreFile << chart.MetaData.artist << ","
                                << chart.MetaData.title << ","
                                << chart.MetaData.creator << ","
                                << chart.MetaData.diff << ","
                                << chart.MetaData.id << ","
                                << formatTime(clock.to_time_t(clock.now())) << ","
                                << modString << ","
                                << acc << ","
                                << ppstyle << ","
                                << calcver
                                << std::endl;
                        }
                        else {

                        }
                        scoreFile.close();
                    }
                }
                else {
                    std::cout << "could not locate beatmap data\n";
                }
            }
            ImGui::SameLine();
            ImGui::Checkbox("Save Score", &saveScore);

            ImGui::Text("pp: %f", ppstyle);
            ImGui::Text("msd: %f", msdstyle);
            ImGui::Text("sr: %f*", srstyle);

            if (ImGui::Button("recalc all scores")) {
                //fix potential problems with metadata later when it actually matters
                std::string line;
                std::ifstream scoreFile("scores.csv");
                if (!scoreFile.is_open()) {
                    std::cout << "unable to find scores.csv" << std::endl;
                }

                std::ofstream scoreBackup("scores_backup.csv");
                while (getline(scoreFile, line)) {
                    if (!scoreBackup.is_open()) {
                        std::cout << "unable to create backup file" << std::endl;
                    }
                    scoreBackup << line << std::endl;
                }
                scoreBackup.close();
                scoreFile.clear();
                scoreFile.seekg(0);

                std::vector<Score> scores;

                while (getline(scoreFile, line)) {
                    scores.push_back(readScore(line));
                }
                scoreFile.close();

                for (int i = 0; i < scores.size(); i++) {
                    scores[i].Rating = std::sqrt(scores[i].Rating / ppscaler);
                    std::string path = chartFinder(scores[i].Chart.MetaData);
                    if (path == "failed") continue;
                    scores[i].Chart = chartReader(path, true);
                    scores[i].Rating = calcMain(&scores[i].Chart, scores[i].Acc, (Mods)scores[i].Mods);
                    //remove chart data later when it matters (it won't)
                }

                std::ofstream scoreRecalc("scores.csv");
                if (!scoreRecalc.is_open()) {
                    std::cout << "unable to output recalculated scores";
                }
                else {
                    for (int i = 0; i < scores.size(); i++) {
                        scoreRecalc << scores[i].Chart.MetaData.artist << ","
                            << scores[i].Chart.MetaData.title << ","
                            << scores[i].Chart.MetaData.creator << ","
                            << scores[i].Chart.MetaData.diff << ","
                            << scores[i].Chart.MetaData.id << ","
                            << scores[i].Date << ","
                            << scores[i].ModString << ","
                            << scores[i].Acc << ","
                            << scores[i].Rating * scores[i].Rating * ppscaler << ","
                            << calcver
                            << std::endl;
                    }
                    scores.clear();
                }
                scoreRecalc.close();
            }

            if(ImGui::InputTextWithHint("osu! path", "osu! songs folder path", path, IM_ARRAYSIZE(path), flags)) setPath(path);
            std::string sPath = path;

            if (ImGui::Button("process all files")) {
                if (sPath.find("\\Songs") != std::string::npos) {
                    fullProcess();
                }
                else {
                    std::cout << "could not locate osu! songs folder" << std::endl;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("output calc debug")) {
                chartPath = chartFinder(currentWorkings);
                chart = chartReader(chartPath, true);

                float squd = calcMain(&chart, 0.95F, (Mods)mods);

                float rabbit = 1;

                std::ofstream calcDebugFile("debug.txt");
                for (int i = 0; i < chart.NoteData.baseDiffs.size(); i++) {
                    calcDebugFile << chart.NoteData.NoteInfo[i].first << "," 
                        << chart.NoteData.NoteInfo[i].second << "," 
                        << chart.NoteData.NoteMS[i] << "," 
                        << chart.NoteData.adjMS[i] << "," 
                        << chart.NoteData.adj_diffs[i] << "," 
                        << ptLoss(squd, chart.NoteData.adj_diffs[i]) << "," 
                        << ChaosMod.pmodValues[i] << "," 
                        << CDMod.pmodValues[i] << ","
                        << StamMod.pmodValues[i] << ","
                        << LengthMod.pmodValues[i] << std::endl;
                }

                calcDebugFile << "-------------" << std::endl;

                for (int i = 0; i < 1000; i++) {
                    rabbit = 1.F - (i / 1000.F);
                    squd = diffIteration(&chart.NoteData.adj_diffs, rabbit) * odAdjust(calcEffOD(&chart, (Mods)mods)) * accLinearExtrapolation(rabbit);
                    calcDebugFile << rabbit << "," << squd << "," << squd * squd * ppscaler << std::endl;
                }
                calcDebugFile.close();
            }

            if (ImGui::Button("load new files")) {
                if (sPath.find("\\Songs") != std::string::npos) {
                    differentialProcess();
                }
                else {
                    std::cout << "could not locate osu! songs folder" << std::endl;
                }
            }

            ImGui::Text("taikofriend v%d calc v%d", programver, calcver);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
