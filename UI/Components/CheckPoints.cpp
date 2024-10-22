#include <beta_helper.hpp>

#ifdef __USE_IMGUI


void saveCheckpoint(const std::string& filename) {
    using namespace rapidjson;

    // Read existing JSON data
    Document doc;
    doc.Parse(R"([])"); // Initialize with an empty array
    FILE* fp = fopen(filename.c_str(), "r");
    if (fp) {
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        doc.ParseStream(is);
        fclose(fp);
        if (!doc.IsArray()) {
            std::cerr << "Existing data is not an array or is malformed" << std::endl;
            return;
        }
    } else {
        // If file does not exist, we start with an empty array
        doc.SetArray();
    }

    // Create a new checkpoint object
    Document::AllocatorType& allocator = doc.GetAllocator();
    Value checkpoint(kObjectType);

    // Add cOff to the checkpoint
    if (cOff.size() >= 2) {
        Value cOffArray(kArrayType);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6) << cOff[0];
        cOffArray.PushBack(Value(ss.str().c_str(), allocator), allocator);
        ss.str("");  // Clear the stringstream
        ss << std::fixed << std::setprecision(6) << cOff[1];
        cOffArray.PushBack(Value(ss.str().c_str(), allocator), allocator);
        checkpoint.AddMember("cOff", cOffArray, allocator);
    } else {
        std::cerr << "cOff vector does not have enough elements. No update made to 'cOff'." << std::endl;
        return;
    }

    // Add scale to the checkpoint
    std::stringstream ss;
    ss.str("");  // Clear the stringstream
    ss << std::fixed << std::setprecision(9) << scale;
    checkpoint.AddMember("scale", Value(ss.str().c_str(), allocator), allocator);

    // Add description to the checkpoint
    std::string description = "";  // Default description if not set
    if (!description.empty()) {
        checkpoint.AddMember("description", Value(description.c_str(), allocator), allocator);
    } else {
        // Generate a description based on current date and time
        std::time_t t = std::time(nullptr);
        std::tm timeStruct = *std::localtime(&t);
        std::string dateString = formatDateTime(timeStruct);
        checkpoint.AddMember("description", Value(dateString.c_str(), allocator), allocator);
    }

    // Create a new array with the new checkpoint at the beginning
    Value newArray(kArrayType);
    newArray.PushBack(checkpoint, allocator);
    for (SizeType i = 0; i < doc.Size(); ++i) {
        newArray.PushBack(doc[i], allocator);
    }

    // Swap the new array with the old one
    doc.Swap(newArray);

    // Write the updated JSON data to the file with pretty printing
    FILE* outputFile = fopen(filename.c_str(), "w");
    if (!outputFile) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    char writeBuffer[65536];
    FileWriteStream os(outputFile, writeBuffer, sizeof(writeBuffer));
    PrettyWriter<FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(outputFile);
}

void ManageCheckpoints() {
    static int selectedCheckpoint = -1;
    static std::deque<Checkpoint> checkpoints;
    static bool initialized = false;

    const std::string filename = "checkpoint.json";
    static bool showModal = false;  // Flag to trigger the modal window

    // Button to open the modal window
    if (ImGui::Button("Manage Checkpoints")) {
        showModal = true;
        if (!initialized) {
            checkpoints.clear();

            // Open the JSON file
            FILE* fp = fopen(filename.c_str(), "r");
            if (!fp) {
                std::cerr << "File does not exist or cannot be opened: " << filename << std::endl;
                initialized = true;  // Ensure initialization flag is set
                return;
            }

            // Read the JSON content
            char readBuffer[65536];
            rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
            rapidjson::Document doc;
            doc.ParseStream(is);
            fclose(fp);

            // Check for parsing errors
            if (doc.HasParseError()) {
                std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(doc.GetParseError()) << std::endl;
                initialized = true;  // Ensure initialization flag is set
                return;
            }

            // Initialize the JSON document if it's empty or invalid
            if (!doc.IsArray()) {
                std::cerr << "JSON root is not an array. Initializing as empty array." << std::endl;
                doc.SetArray();
            }

            // Handle empty JSON array
            if (doc.GetArray().Size() == 0) {
                std::cerr << "JSON array is empty. No checkpoints found." << std::endl;
                initialized = true;  // Ensure initialization flag is set
                return;
            }
            
            // Iterate through all checkpoints
            for (const auto& checkpoint : doc.GetArray()) {
                if (!checkpoint.IsObject()) {
                    std::cerr << "Checkpoint is not an object" << std::endl;
                    continue;
                }

                Checkpoint cp;

                // Load cOff
                auto cOffIt = checkpoint.FindMember("cOff");
                if (cOffIt != checkpoint.MemberEnd() && cOffIt->value.IsArray()) {
                    const rapidjson::Value& cOffArray = cOffIt->value;
                    if (cOffArray.Size() >= 2) {
                        if (cOffArray[0].IsString() && cOffArray[1].IsString()) {
                            try {
                                cp.loaded_cOff[0] = std::stod(cOffArray[0].GetString());
                                cp.loaded_cOff[1] = std::stod(cOffArray[1].GetString());
                            } catch (const std::exception& e) {
                                std::cerr << "Error parsing cOff values: " << e.what() << std::endl;
                                continue;
                            }
                        } else {
                            std::cerr << "cOff array elements are not strings" << std::endl;
                            continue;
                        }
                    } else {
                        std::cerr << "cOff array does not have enough elements" << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "cOff not found or is not an array in the checkpoint" << std::endl;
                    continue;
                }

                // Load scale
                auto scaleIt = checkpoint.FindMember("scale");
                if (scaleIt != checkpoint.MemberEnd() && scaleIt->value.IsString()) {
                    try {
                        cp.loaded_scale = std::stod(scaleIt->value.GetString());
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing scale value: " << e.what() << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "scale not found or is not a string in the checkpoint" << std::endl;
                    continue;
                }

                // Load description
                auto descriptionIt = checkpoint.FindMember("description");
                if (descriptionIt != checkpoint.MemberEnd() && descriptionIt->value.IsString()) {
                    cp.loaded_description = descriptionIt->value.GetString();
                } else {
                    std::cerr << "Description not found or is not a string in the checkpoint" << std::endl;
                }
                
                checkpoints.push_back(cp);
            }

            initialized = true;
        }
    }

    // Render the modal window
    if (showModal) {
        ImGui::OpenPopup("Manage Checkpoints");

        if (ImGui::BeginPopupModal("Manage Checkpoints", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Add new checkpoint
            static double newX = cOff[pX], newY = cOff[pY], newScale = scale;
            static char BufferDescription[256] = "";
            
            if (ImGui::BeginMenu("Settings")) {

                double footstep = newScale / 1.0; // Replace 1.0 with actual value if needed
                ImGui::BeginGroup();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("New COff[pX]:");
                ImGui::InputDouble("##NewX", &newX, footstep, footstep * 10, "%.10f");
                ImGui::EndGroup();

                ImGui::SameLine();
                footstep = newScale / 1.0; // Replace 1.0 with actual value if needed
                ImGui::BeginGroup();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("New COff[pY]:");
                ImGui::InputDouble("##NewY", &newY, footstep, footstep * 10, "%.10f");
                ImGui::EndGroup();

                ImGui::Text("New Scale:");
                ImGui::InputDouble("##NewScale", &newScale, 0.000001, 0.00001, "%.9f");

                ImGui::Text("New Description:");
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextUnformatted("Optional, by default the description is the date");
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
                ImGui::InputText("##Description", BufferDescription, sizeof(BufferDescription));
                
                if (ImGui::Button("Add Checkpoint")) {
                    std::string newDescription = BufferDescription;
                    if (!*BufferDescription) {
                        std::time_t t = std::time(nullptr);
                        std::tm timeStruct = *std::localtime(&t);

                        // Format the date and time
                        newDescription = formatDateTime(timeStruct);
                    }
                    Checkpoint cp = {{newX, newY}, newScale, newDescription};
                    checkpoints.push_front(cp);
                }
                ImGui::SameLine();
                if (ImGui::Button("Go-to Checkpoint")) {
                    cOff[pX] = newX;
                    cOff[pY] = newY;
                    scale = newScale;
                    frameState = fSTATE::FULL_COMPUTE;
                }
                {
                    static bool show_Inside_Modal = false;
                    static std::chrono::steady_clock::time_point startTime;
                    static const int countdownSeconds = 10; // Countdown duration in seconds

                    // Start the modal when the button is pressed
                    if (ImGui::Button("Clear All")) {
                        show_Inside_Modal = true;
                        startTime = std::chrono::steady_clock::now(); // Reset countdown
                    }

                    // Define the modal dialog
                    if (show_Inside_Modal) {
                        ImGui::OpenPopup("Warning");
                    }

                    if (ImGui::BeginPopupModal("Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        // Calculate elapsed time
                        auto now = std::chrono::steady_clock::now();
                        std::chrono::duration<float> elapsed = now - startTime;
                        int secondsLeft = countdownSeconds - static_cast<int>(elapsed.count());

                        if (secondsLeft <= 0) {
                            secondsLeft = 0;
                            checkpoints.clear();
                            ImGui::CloseCurrentPopup(); // Close the modal when countdown finishes
                            show_Inside_Modal = false;
                        }

                        // Display the countdown message
                        ImGui::Text("Hesitation is Defeat: %02d Seconds Left", secondsLeft);

                        // Add some space before the buttons
                        ImGui::Spacing();

                        // Create a row with two buttons
                        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Confirm").x - ImGui::CalcTextSize("Revoke").x - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
                        if (ImGui::Button("Confirm")) {
                            checkpoints.clear();
                            ImGui::CloseCurrentPopup(); // Close the modal
                            show_Inside_Modal = false;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Revoke")) {
                            ImGui::CloseCurrentPopup(); // Close the modal
                            show_Inside_Modal = false;
                        }

                        ImGui::EndPopup();
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            // List all checkpoints with their positions and scale
            for (size_t i = 0; i < checkpoints.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));

                std::string checkpointLabel = checkpoints[i].loaded_description; //"Checkpoint " + std::to_string(i + 1);
                if(checkpointLabel.size() == 0)
                {
                    checkpointLabel = "Checkpoint " + std::to_string(i + 1);
                }
                if (ImGui::TreeNode(checkpointLabel.c_str())) {
                    ImGui::Text("Position: (%.6f, %.6f), Scale: %.9f", 
                        checkpoints[i].loaded_cOff[0], checkpoints[i].loaded_cOff[1], checkpoints[i].loaded_scale);

                    // Edit checkpoint
                    double editX = checkpoints[i].loaded_cOff[0], editY = checkpoints[i].loaded_cOff[1], editScale = checkpoints[i].loaded_scale;
                    if (ImGui::InputDouble("Edit X", &editX, 0.000001, 0.00001, "%.6f") ||
                        ImGui::InputDouble("Edit Y", &editY, 0.000001, 0.00001, "%.6f") ||
                        ImGui::InputDouble("Edit Scale", &editScale, 0.000001, 0.00001, "%.9f")) {
                        checkpoints[i] = {{editX, editY}, editScale, ""};
                    }

                    // Remove button
                    if (ImGui::Button("Remove")) {
                        checkpoints.erase(checkpoints.begin() + i);
                        if (selectedCheckpoint == static_cast<int>(i)) {
                            selectedCheckpoint = -1;
                        } else if (selectedCheckpoint > static_cast<int>(i)) {
                            selectedCheckpoint--;
                        }
                        ImGui::TreePop();
                        ImGui::PopID();
                        break;  // Exit the loop as we've modified the vector
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Go-to")) {
                        cOff[pX] = editX;
                        cOff[pY] = editY;
                        scale = editScale;
                        frameState = fSTATE::FULL_COMPUTE;
                    }

                    ImGui::TreePop();
                }

                // Handle drag-and-drop reordering
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("DND_CHECKPOINT", &i, sizeof(size_t));
                    ImGui::Text("Dragging %s", checkpointLabel.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CHECKPOINT")) {
                        size_t draggedIndex = *(size_t*)payload->Data;
                        if (draggedIndex != i) {
                            std::swap(checkpoints[draggedIndex], checkpoints[i]);
                            if (selectedCheckpoint == static_cast<int>(draggedIndex)) {
                                selectedCheckpoint = static_cast<int>(i);
                            } else if (selectedCheckpoint == static_cast<int>(i)) {
                                selectedCheckpoint = static_cast<int>(draggedIndex);
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();
            }

            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                showModal = false;  // Reset modal flag
            }

            ImGui::EndPopup();
        }
    }

    // Save checkpoints back to JSON file when the window is closed
    if (!showModal && initialized) {
        FILE* fp = fopen(filename.c_str(), "w");
        if (fp) {
            rapidjson::Document doc;
            doc.SetArray();
            auto& allocator = doc.GetAllocator();

            for (const auto& cp : checkpoints) {
                rapidjson::Value checkpoint(rapidjson::kObjectType);
                rapidjson::Value cOff(rapidjson::kArrayType);

                // Format cOff values with 6 decimal places
                char cOffX[20], cOffY[20];
                snprintf(cOffX, sizeof(cOffX), "%.6f", cp.loaded_cOff[0]);
                snprintf(cOffY, sizeof(cOffY), "%.6f", cp.loaded_cOff[1]);

                cOff.PushBack(rapidjson::Value(cOffX, allocator), allocator);
                cOff.PushBack(rapidjson::Value(cOffY, allocator), allocator);

                // Format scale value with 9 decimal places
                char scaleStr[20];
                snprintf(scaleStr, sizeof(scaleStr), "%.9f", cp.loaded_scale);

                checkpoint.AddMember("cOff", cOff, allocator);
                checkpoint.AddMember("scale", rapidjson::Value(scaleStr, allocator), allocator);
                checkpoint.AddMember("description", rapidjson::Value(cp.loaded_description.c_str(), allocator), allocator);

                doc.PushBack(checkpoint, allocator);
            }

            char writeBuffer[65536];
            rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
            doc.Accept(writer);
            fclose(fp);
        } else {
            std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        }
        initialized = false;  // Reset initialization flag
    }
}

bool loadCheckpoint(const std::string& filename) {
    using namespace rapidjson;

    // Open the JSON file
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    // Read the JSON content
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document doc;
    doc.ParseStream(is);
    fclose(fp);

    // Check for parsing errors
    if (doc.HasParseError()) {
        std::cerr << "Error parsing JSON: " << GetParseError_En(doc.GetParseError()) << std::endl;
        return false;
    }

    // Validate that the root is an array and is not empty
    if (!doc.IsArray()) {
        std::cerr << "JSON root is not an array" << std::endl;
        return false;
    }

    const Value& checkpoints = doc;
    if (checkpoints.Size() == 0) {
        std::cerr << "No checkpoints found in the file" << std::endl;
        return false;
    }

    // Get the first checkpoint object
    const Value& checkpoint = checkpoints[0];
    if (!checkpoint.IsObject()) {
        std::cerr << "First checkpoint is not an object" << std::endl;
        return false;
    }

    // Load cOff
    auto cOffIt = checkpoint.FindMember("cOff");
    if (cOffIt != checkpoint.MemberEnd() && cOffIt->value.IsArray()) {
        const Value& cOffArray = cOffIt->value;
        if (cOffArray.Size() >= 2) {
            // Extract and convert each element correctly
            if (cOffArray[0].IsString() && cOffArray[1].IsString()) {
                try {
                    cOff[0] = std::stod(cOffArray[0].GetString());
                    cOff[1] = std::stod(cOffArray[1].GetString());
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument for cOff value: " << e.what() << std::endl;
                    return false;
                } catch (const std::out_of_range& e) {
                    std::cerr << "cOff value out of range: " << e.what() << std::endl;
                    return false;
                }
            } else {
                std::cerr << "cOff array elements are not strings" << std::endl;
                return false;
            }
        } else {
            std::cerr << "cOff array does not have enough elements" << std::endl;
            return false;
        }
    } else {
        std::cerr << "cOff not found or is not an array in the checkpoint" << std::endl;
        return false;
    }

    // Load scale
    auto scaleIt = checkpoint.FindMember("scale");
    if (scaleIt != checkpoint.MemberEnd() && scaleIt->value.IsString()) {
        try {
            scale = std::stod(scaleIt->value.GetString());
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument for scale value: " << e.what() << std::endl;
            return false;
        } catch (const std::out_of_range& e) {
            std::cerr << "Scale value out of range: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cerr << "scale not found or is not a string in the checkpoint" << std::endl;
        return false;
    }

    return true;
}


void CheckPoints(){
    ManageCheckpoints();

    if (ImGui::BeginMenu("SwiftAccess")) { // Start a menu
        
        if(ImGui::Selectable("save checkpoint")){
            saveCheckpoint("checkpoint.json");
        }

        if(ImGui::Selectable("load checkpoint"))
        {
            loadCheckpoint("checkpoint.json");
            frameState = fSTATE::FULL_COMPUTE;
        }
        ImGui::EndMenu(); // End the menu
    }
}
#endif