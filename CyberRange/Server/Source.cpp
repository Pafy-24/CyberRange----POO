#include <iostream>
#include <memory>
#include "CustomSerial.h"
#include <map>
#include <string>
using namespace std;


void main() {


	map<string, string> data;
	data["name"] = "John Doe";
	data["age"] = "30";
	data["city"] = "New York";

	string jsonString = CustomSerial::encodeJSON(data);
	string jwtString = CustomSerial::encodeJWT(data, "secret-Key");

	cout << jsonString << endl;
	cout << jwtString << endl;


	cout << CustomSerial::isValid(jwtString, "secret-Key") << endl;
	cout << CustomSerial::isValid(jsonString) << endl;
	data.clear();
	data=CustomSerial::decode(jwtString, "secret-Key");
	data.clear();
	data= CustomSerial::decode(jsonString);
}