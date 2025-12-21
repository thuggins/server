/**
@file common.h
@brief Shared constants and small utilities used across modules.
*/
#pragma once

// Maximum size for small HTTP headers we compose (snprintf buffer)
#define COMMON_HEADER_BUF_SIZE 256

// Windows MAX_PATH equivalent for our local path buffers
#define COMMON_MAX_PATH 260
