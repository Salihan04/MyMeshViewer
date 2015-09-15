#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//Function prototypes
void parseFile(char* fileName);

int main(int argc, char **argv)
{
	parseFile("dummy.txt");

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