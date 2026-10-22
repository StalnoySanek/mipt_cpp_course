#pragma once
#include <string>

bool TextEqualsIgnoreCase(const std::string& left, const std::string& right);

bool TextContainsIgnoreCase(const std::string& text, const std::string& needle);

bool TextEndsWithIgnoreCase(const std::string& text, const std::string& suffix);