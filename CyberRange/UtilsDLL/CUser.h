#pragma once
#include <iostream>
#include "User.h"
class CUser :
    public User
{
public:
	void Print()
	{
		std::cout << "Hello from CUser!" << std::endl;
	}
};

