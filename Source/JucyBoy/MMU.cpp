#include "MMU.h"

MMU::MMU() :
	memory_{} // Value-initialize to all-zeroes
{

}

MMU::~MMU()
{

}

uint8_t MMU::Read(Address address) const
{
	return memory_[address];
}

void MMU::Write(Address address, uint8_t value)
{
	memory_[address] = value;
}
