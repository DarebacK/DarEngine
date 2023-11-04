#define DAR_MODULE "Concurrency"

#include "Core/Concurrency.hpp"

thread_local ThreadType threadType = ThreadType::Unknown;
