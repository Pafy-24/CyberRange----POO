#include <iostream>
#include "Admin.h"
#include "Writer.h"
#include "Common.h"

void main() 
{
	Admin A1("username", "email");
	Writer W1("username", "email");
	Common C1("username", "email");
	std::cout << "TEST";
}