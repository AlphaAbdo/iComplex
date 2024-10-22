#pragma once
#include <externVars.hpp>

#include <csignal>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <Computational_Loop.hpp>
#include <IMGUI_Loop.hpp>

// int ready = false;
bool shouldAllocateSSBO = false;
