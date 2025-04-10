#include <iostream>
#include <memory>
#include "User.h"
using namespace std;


void main() {
	shared_ptr<User> user = make_shared<User>();
	cout << "Hello World!" << endl;
}