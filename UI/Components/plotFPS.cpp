#include <beta_helper.hpp>

#ifdef __USE_IMGUI
#include <ThreadPool.hpp>

double observationTime = 10; // in seconds
double totalbatchTime = 0;
// std::vector<float> fpsHistory;
std::deque<float> fpsHistory{};

float firstRUN = 0;

double& currentTime = avgLoopTime; // Ensure this is updated elsewhere in your code

float minValue{};
float maxValue{};
    
float Ymargin = 4.0f; // Reduced margin

// ImVec2 plotSize;

std::vector<float> fpsHistoryDownsampled{};

std::future<void> dataReady = std::future<void>();

int minIndexDownsampled = 0;
int maxIndexDownsampled = 0;

int xPlotTotalPixels = 0;

std::vector<float> downsampleVector(const std::deque<float>& input, const int plotSizeX = -1) {
    size_t originalSize = input.size();
    size_t newSize;
    float samplerate;

    if (plotSizeX > 0) {
        newSize = std::min(static_cast<size_t>(plotSizeX * 1.2), originalSize);
        samplerate = static_cast<float>(originalSize) / newSize;
    } else {
        samplerate = std::sqrt(std::sqrt(originalSize));
        newSize = static_cast<size_t>(std::ceil(originalSize / samplerate));
    }

    std::vector<float> output;
    output.reserve(newSize);

    ::minValue = std::numeric_limits<float>::max();
    ::maxValue = std::numeric_limits<float>::lowest();
    minIndexDownsampled = 0;
    maxIndexDownsampled = 0;
    for (size_t i = 0; i < newSize; ++i) {
        float start = i * samplerate;
        float end = std::min((i + 1) * samplerate, static_cast<float>(originalSize));
        
        int startIdx = static_cast<int>(std::floor(start));
        int endIdx = static_cast<int>(std::ceil(end));
        
        float sum = 0.0f;
        float weightSum = 0.0f;

        for (int j = startIdx; j < endIdx; ++j) {
            float weight = 1.0f;
            if (j == startIdx) {
                weight = 1.0f - (start - std::floor(start));
            }
            else if (j == endIdx - 1) {
                weight = end - std::floor(end);
            }
            sum += input[j] * weight;
            weightSum += weight;

            if(input[j] < ::minValue) {
                ::minValue = input[j];
                minIndexDownsampled = i;
            }
            if(input[j] > ::maxValue) {
                ::maxValue = input[j];
                maxIndexDownsampled = i;
            }

        }
        float value = sum / weightSum;
        output.push_back(value);

    }
    return output;
}

void getDataReady(){
    const float BeginningTime = 8.0;
    if (currentTime > 0.0) {
    float fps = 1.0f / static_cast<float>(currentTime);
    fpsHistory.push_back(fps);
    if(firstRUN < BeginningTime) firstRUN += currentTime;
    totalbatchTime += currentTime;
    while (!fpsHistory.empty() && totalbatchTime > observationTime) {
        if(fpsHistory.size() == 1 && totalbatchTime){
            totalbatchTime = 0.0;
            continue;
        }
        totalbatchTime -= 1.0 / static_cast<double>(fpsHistory.front());
        fpsHistory.pop_front();
    }
    // Temporary use of Round Robin to erase first-degree launch artifacts
    if (!fpsHistory.empty() && firstRUN < BeginningTime) {
        static int eraseRound = 1;
        if (++eraseRound > 3) {
            totalbatchTime -= 1.0 / static_cast<double>(fpsHistory.front());
            fpsHistory.pop_front();
            eraseRound = 0;
        }
    }
}
    if(fpsHistory.size()>0){
        try{
            if(fpsHistoryDownsampled.size()>0) fpsHistoryDownsampled.erase(fpsHistoryDownsampled.begin(), fpsHistoryDownsampled.end());
            fpsHistoryDownsampled = downsampleVector(fpsHistory, xPlotTotalPixels);
        } catch (std::exception& e) {
            printf("Error: %s\n", e.what());
        }
        
    }
}
void RenderFPSPlot() {
    static ThreadPool pool(3);  // Create a thread pool with 4 threads

    bool canBeExecuted = false;
    
    if(dataReady.valid()) {
        dataReady.get();
        canBeExecuted = true & !fpsHistory.empty();
    }

    const float margin = 2.0f;
    
    xPlotTotalPixels = static_cast<int>(ImGui::GetContentRegionAvail().x);
    if (canBeExecuted && ImPlot::BeginPlot("##FPS Plot", ImVec2(xPlotTotalPixels, 120),
        ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect)) {
        
        ImPlot::SetupAxes(NULL, "##FPS", 
            ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines,
            ImPlotAxisFlags_NoGridLines);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, fpsHistoryDownsampled.size() - 1, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, std::max(0.0f, minValue - margin), maxValue + margin, ImGuiCond_Always);

        ImPlot::PlotLine("##FPSO", fpsHistoryDownsampled.data(), fpsHistoryDownsampled.size());
        
        {
            
            std::vector<float> xData = {static_cast<float>(minIndexDownsampled), static_cast<float>(maxIndexDownsampled)};
            std::vector<float> yData = {minValue, maxValue};
            ImPlot::PlotScatter("MinMaxMarkers", xData.data(), yData.data(), 2);

            ImPlot::PlotText(("Min: " + std::to_string(minValue)).c_str(), minIndexDownsampled, minValue, ImVec2(10,-10));
            ImPlot::PlotText(("Max: " + std::to_string(maxValue)).c_str(), maxIndexDownsampled, maxValue, ImVec2(10,10));
        }
        ImPlot::EndPlot();
    }
    

    ImGui::PushItemWidth(xPlotTotalPixels);
    double minValue = 0.0;
    double maxValue = 100;
    ImGui::SliderScalar("##Observation Time", ImGuiDataType_Double, &observationTime, &minValue, &maxValue, "%.1fs", ImGuiSliderFlags_Logarithmic);
    ImGui::PopItemWidth();

    dataReady = pool.enqueue(getDataReady);
    
}

#endif