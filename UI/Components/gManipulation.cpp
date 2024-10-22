#include <beta_helper.hpp>

#ifdef __USE_IMGUI
void gManipulation()
{
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0)); 
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0)); 
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);

        ImGui::Text("cOff[pX]: {");
        ImGui::SameLine(0,2);
        ImGui::SetNextItemWidth(67);


        double decoy_cOff_pX = cOff[pX];
        if(ImGui::DragScalar("##cOff[pX]", ImGuiDataType_Double, &decoy_cOff_pX, 1, nullptr, nullptr, "%.6f")){
            betaWindow::DeferredResults::tmp_cOff[pX] = std::round(decoy_cOff_pX - cOff[pX]);
        }

        ImGui::SameLine(0,2);
        ImGui::Text(", ");
        ImGui::SameLine(0,2);
        ImGui::SetNextItemWidth(67);
        double decoy_cOff_pY = cOff[pY];
        if(ImGui::DragScalar("##cOff[pY]", ImGuiDataType_Double, &decoy_cOff_pY, 1, nullptr, nullptr, "%.6f")){
            betaWindow::DeferredResults::tmp_cOff[pY] = std::round(decoy_cOff_pY - cOff[pY]);
        }
        ImGui::SameLine(0,1);
        ImGui::Text("}");
        
        ImGui::Text("scale: ");
        ImGui::SameLine(0,2);
        ImGui::SetNextItemWidth(200);


        double decoy_scale = scale;
        if(ImGui::DragScalar("##scale", ImGuiDataType_Double, &decoy_scale, 1, nullptr, nullptr, "%.9f")){
            betaWindow::DeferredResults::tmp_scale = scale - decoy_scale;
        }



        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

        ImGui::Text("depth: ");
        ImGui::SameLine(0,2);
        ImGui::SetNextItemWidth(150);

        const double depth_minValue = 1.0; 
        const double depth_maxValue = (1 << 16);

        // Convert integer value to logarithmic scale
        double double_depth = static_cast<double>(crunch);

        // Create the slider with logarithmic scaling
        if (ImGui::SliderScalar("##depthhhh", ImGuiDataType_Double, &double_depth, &depth_minValue, &depth_maxValue, "%.0f", ImGuiSliderFlags_Logarithmic)) {
           printf("depth: %lf\n", double_depth);
           betaWindow::DeferredResults::temp_depth = double_depth;
        }
    }
    
    if(ImGui::BeginMenu("precision")) {
        if(ImGui::MenuItem("Float", NULL, precisionMode == 0))
        {
            betaWindow::DeferredResults::precisionMode = 0;
        }
        if(ImGui::MenuItem("Double-Float", NULL, precisionMode == 1))
        {
            betaWindow::DeferredResults::precisionMode = 1;
        }
        if(ImGui::MenuItem("Double", NULL, precisionMode == 2))
        {
            betaWindow::DeferredResults::precisionMode = 2;
        }
        ImGui::EndMenu();
    }
}
#endif