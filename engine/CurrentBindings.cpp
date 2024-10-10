#include <atomic>
#include "utils/types.h"

std::atomic<u32> currently_bound_program = 0;
std::atomic<u32> currently_bound_uniform_buffer = 0;
std::atomic<u32> current_vao_binding = 0;