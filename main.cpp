#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <locale>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;
using namespace experimental::filesystem;

class Word {
private:
	string characters;
	unsigned long count;
public:
	Word(string value) {
		characters = value;
		count = 1;
	}
	string getCharacters() {
		return characters;
	}
	unsigned long getCount() {
		return count;
	}
	void operator++() {
		count++;
	}
	bool operator<(Word other) {
		return characters < other.getCharacters();
	}
	bool operator<(string other) {
		return characters < other;
	}
	bool operator==(string other) {
		return characters == other;
	}
	bool operator==(Word other) {
		return characters == other.getCharacters();
	}
	bool operator>(Word other) {
		return characters > other.getCharacters();
	}
	bool operator>(string other) {
		return characters > other;
	}

};

string fileToString(v1::directory_entry f) {
	stringstream buffer;
	buffer << f;
	string result = "";
	getline(buffer, result);
	return result;
}

string getWord(ifstream &file, locale loc) {
	string word = "";
	getline(file, word, ' ');
	for (int i = 0; i < word.length(); i++) {
		if (!(isalpha(word[i], loc) || word[i] == '\'')) {
			word.erase(i);
			i--;
		}
		else {
			word[i] = toupper(word[i], loc);
		}
	}
	return word;
}

void addWord(string word, vector<Word> &words) {
	int min = 0;
	int max = words.size() - 1;
	int cur = 0;
	if (words.size() > 0) {
		while (true) {
			cur = (max + min) / 2;
			if (words[cur] == word) {
				++words[cur];
				break;
			}
			else if (words[cur] > word) {
				max = cur - 1;
			}
			else {
				min = cur + 1;
			}

			if (max < min) {
				if (cur == min) {
					words.insert(words.begin() + cur, Word(word));
				}
				else if (cur == max) {
					if (cur >= words.size()-1) {
						words.emplace_back(word);
					}
					else {
						words.insert(words.begin() + (cur+1), Word(word));
					}
				}
				break;
			}
		}
	}
	else {
		words.emplace_back(word);
	}
}

bool sortByCount(Word a, Word b) {
	if (a.getCount() == b.getCount()) {
		return a < b;
	}
	else {
		return a.getCount() > b.getCount();
	}
}

int main(int argc, char* argv[]) {
	if (argc == 3) {
		vector<Word> words;
		ofstream outFile;
		ifstream inFile;
		locale loc("");
		int numberOfFiles = 0;
		outFile.open(argv[2]);
		if (outFile) {
			time_t begin = time(0);
			for (auto& f : directory_iterator(argv[1])) {
				inFile.open(fileToString(f));
				if (inFile) {
					numberOfFiles++;
					cout << "Parsing words from \"" << fileToString(f) << "\"..." << endl;
					string word = "";
					while (!inFile.eof()) {
						word = getWord(inFile, loc);
						if (word.length() > 0) {
							addWord(word, words);
						}
					}
					inFile.close();
				}
				else {
					cout << "Error: could not open \"" << fileToString(f) << "\" for reading." << endl;
				}
			}
			time_t duration = time(0) - begin;
			cout << "Parsed " << numberOfFiles << " file/s. Total time: "<<duration<<" seconds. Recording results..." << endl;
			sort(words.begin(), words.end(), sortByCount);
			outFile << "-----Results from "<<numberOfFiles<<" file/s in \""<<argv[1]<<"\"-----" << endl << endl;
			unsigned long long totalWords = 0;
			for (int i = 0; i < words.size(); i++) {
				totalWords += words[i].getCount();
			}
			outFile << "Duration: " << duration << " seconds." << endl;
			outFile << "Total number of words parsed: " << totalWords << endl;
			outFile << "Number of unique words: " << words.size() << endl << endl;

			outFile << "Rank,Word,Occurrences,Frequency,Expected Frequency,Percent Error," << endl << endl;

			long double firstFreq = 0;
			long double freq = 0;
			long double expected = 0;
			long double error = 0;
			for (int i = 0; i < words.size(); i++) {
				outFile <<setw(7)<<right<< (i + 1) << "," << setw(15) << right << words[i].getCharacters() << "," << setw(8) << right << words[i].getCount() << ",";
				if (i == 0) {
					firstFreq = (words[i].getCount() / (long double)totalWords);
				}
				freq = (words[i].getCount() / (long double)totalWords);
				expected = (1 / (long double)(i + 1))*firstFreq;
				error = abs((expected - freq) / expected)*(long double)100.0;
				outFile << setw(10) << right << freq << "," << setw(10) << right << expected << "," << setw(10) << right << error << "," << endl;
			}

			outFile << endl << "-----End Results-----";
			cout << "Finished recording results." << endl;

			outFile.close();
		}
		else {
			cout << "Error: could not open \"" << argv[2] << "\" for writting." << endl;
		}
	}
	else {
		if (argc > 0) {
			cout << "Invalid number of arguments. Usage: \n" << argv[0] << " \"Directory of text files to parse\" \"name of textfile to write results to\"" << endl;
		}
		else {
			cout << "Invalid number of arguments. Usage: \n(name of executable) \"Directory of text files to parse\" \"name of textfile to write results to\"" << endl;
		}
	}
	return 0;
}