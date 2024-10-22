#include <beta_helper.hpp>

#ifdef __USE_IMGUI

std::string formatDateTime(const std::tm& timeStruct) {
    std::ostringstream oss;
    
    // Format as MM/DD/YY HH24/MI/SS
    oss << std::setw(2) << std::setfill('0') << (timeStruct.tm_mon + 1) << '/'  // MM
        << std::setw(2) << std::setfill('0') << timeStruct.tm_mday << '/'      // DD
        << std::setw(2) << std::setfill('0') << (timeStruct.tm_year % 100) << ' ' // YY
        << std::setw(2) << std::setfill('0') << timeStruct.tm_hour << '/'        // HH24
        << std::setw(2) << std::setfill('0') << timeStruct.tm_min << '/'         // MI
        << std::setw(2) << std::setfill('0') << timeStruct.tm_sec;               // SS
    
    return oss.str();
}

const char* DockingPositionToString(DockingPosition pos) {
    switch (pos) {
        case DockingPosition::Left: return "Left";
        case DockingPosition::Right: return "Right";
        case DockingPosition::Custom: return "Custom";
        default: return "Unknown";
    }
}


#endif