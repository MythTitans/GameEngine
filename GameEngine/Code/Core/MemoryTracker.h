#pragma once

class MemoryTracker
{
public:
	MemoryTracker();

	void Display();

private:
	bool m_bDisplayMemoryTracker;
};

extern MemoryTracker* g_pMemoryTracker;