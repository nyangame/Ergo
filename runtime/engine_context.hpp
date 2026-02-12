#pragma once
#include "game_interface/game_interface.h"

// Forward declarations
class VulkanRenderer;
class DesktopInput;

// Build the C API bridge between engine and game DLL
ErgoEngineAPI build_engine_api(VulkanRenderer& renderer, DesktopInput& input);
