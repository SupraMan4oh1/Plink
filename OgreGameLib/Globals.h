#pragma once

#include <cstddef>

class Application;

/** @brief Keeps global values as static members.
@note Globals should only be values that are often accessed across the entire program, to the point where it's 
prohibitive to access or a performance issue to continually get references to them. */
class Globals
{
public:

	static Application *app;
};