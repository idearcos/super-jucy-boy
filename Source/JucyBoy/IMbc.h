#pragma once

#include "Memory.h"

class IMbc
{
public:
	virtual ~IMbc() {}
	virtual void OnRomWritten(Memory::Address address, uint8_t value) = 0;
};
