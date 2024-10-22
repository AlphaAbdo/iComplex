#include <beta_helper.hpp>

#ifdef __USE_IMGUI



void ColorPicker() {
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayRGB 
                                | ImGuiColorEditFlags_PickerHueWheel
                                | ImGuiColorEditFlags_NoAlpha
                                | ImGuiColorEditFlags_NoSidePreview;
    float constantWidth = 200.0f;
    ImGui::PushItemWidth(constantWidth); // Set a fixed width for the color picker
    ImGui::BeginGroup();
    // Set a fixed width for the color picker
    ImGui::ColorPicker4("##Colorize", color_factor, flags);
    ImGui::PopItemWidth(); // Restore the item width to its previous state
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::VSliderFloat("##scaler",ImVec2(22, 180), &color_scaler, 0.5f, 255.0f, "",ImGuiSliderFlags_Logarithmic);
    ImGui::Text("%.1f", color_scaler);
    ImGui::EndGroup();
    if (ImGui::Button("Randomize Color")) {
        SetRandomColor();
    }
    // Move to the same line
    ImGui::SameLine();

    // Create the text widget with tooltip
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted("Randomize Color, ");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }


   // Group for the "Animation Step" input
    {
        ImGui::AlignTextToFramePadding();          // Align text vertically to the widget
        ImGui::Text("Animation Step:");
        ImGui::SameLine();
        ImGui::PushItemWidth(125);
        ImGui::InputInt("##animat", &animationStep, 1, 10);
        ImGui::PopItemWidth();
    }

    ImGui::EndGroup();
    

}


#endif