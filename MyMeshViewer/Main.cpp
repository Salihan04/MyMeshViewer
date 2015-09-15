#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//Function prototypes
void parseFile(char* fileName);

int main(int argc, char **argv)
{
	parseFile("dummy.txt");

	cout << endl;

	//The below code is just to test converting a string to a float
	string str = "777";
	float num = atof(str.c_str());	//Convert string to float
	num = num + 1.2f;
	cout << num << endl;

	system("pause");

	return 0;
}

void parseFile(char* fileName)
{
	string str;
	ifstream infile;
	infile.open(fileName);

	if (!infile.is_open())
	{
		cout << "Cannot open dummy.txt" << endl;
	}
	
	while (infile)
	{
		getline(infile, str);
		if (infile)
			cout << str << endl;
	}

	infile.close();
}