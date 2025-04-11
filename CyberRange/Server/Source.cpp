#include <iostream>
#include <memory>
#include "JWTEnc.h"
using namespace std;


void main() {

	JSONEnc jsonEncoder(true);
	map<string, string> data;
	data["name"] = "John Doe";
	data["age"] = "30";
	data["city"] = "New York";

	string jsonString = jsonEncoder.encode(data);

	cout << jsonString << endl;

	jsonEncoder.setEscapeUnicode(true);
	data["unicode"] = "Hello \u2603";

	jsonString = jsonEncoder.encode(data);
	cout << jsonString << endl;
	cout << JWTEnc("my-secret-key", "HS256").encode(jsonString) << endl;
}