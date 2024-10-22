#pragma once
#include <macros.hpp>

#ifdef __USE_IMGUI
#include <externVars.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <queue>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"

namespace betaWindow{
	namespace DeferredResults{
		inline fSTATE frameState = fSTATE::UNKNOWN_STATE;
		inline bool reloadDisDepGenShader = false;

		inline bool cOff_modified = false;
		inline double temp_depth = 0.0;
		inline int precisionMode = -1;
		inline double tmp_scale = 0;
		inline std::vector<long int> tmp_cOff = {0, 0};

	}
    inline bool OptimizeCUDA = true;
}

struct Checkpoint {
    std::array<double, 2> loaded_cOff;
    double loaded_scale;
    std::string loaded_description;
};


enum class DockingPosition {
    Left,
    Right,
    Custom
};

struct DockingState {
    DockingPosition position = DockingPosition::Left;
    ImVec2 customPosition = ImVec2(0, 0);
    ImVec2 size = ImVec2(250, 0);  // Default width is 250, height will be set to viewport height
    bool isDragging = false;
};

inline DockingState dockingState;


// defined functions

extern std::string formatDateTime(const std::tm& timeStruct);
extern const char* DockingPositionToString(DockingPosition pos);
extern void CheckPoints();
extern void gManipulation();
extern void RenderFPSPlot();
extern void ColorPicker();

#endif