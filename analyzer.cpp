#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann\json.hpp>
#include <string>
#include <sstream>
#include <vector>


using namespace std;
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

typedef chrono::high_resolution_clock Time;
typedef chrono::milliseconds ms;
typedef chrono::microseconds us;
typedef chrono::duration<float> fsec;

ifstream infile_if_file_exists(string filepath) {
    ifstream infile_object;
    infile_object.open(filepath);
    if (!infile_object)
        throw std::runtime_error("FileNotFoundError in infile_if_file_exists()");
    return infile_object;
}

ifstream effortTableJson = infile_if_file_exists("keyboard_effort_table.json");
json EffortTable = json::parse(effortTableJson);
ifstream monogramJson = infile_if_file_exists("n1gram_frequency.json");
json Monogram = json::parse(monogramJson);
ifstream bigramJson = infile_if_file_exists("n2gram_frequency.json");
json Bigram = json::parse(bigramJson);
ifstream trigramJson = infile_if_file_exists("n3gram_frequency.json");
json Trigram = json::parse(trigramJson);
ifstream constraintsJson = infile_if_file_exists("config.json");
ordered_json Constraints = ordered_json::parse(constraintsJson);
ifstream cutoffScoreJson = infile_if_file_exists("cutoff_score.json");
json CutoffScore = json::parse(cutoffScoreJson);


class Keyboard {
public:
	string KeyboardLayout = "";
	short KeyboardMap[30][3] = {};
	// -- Monogram stats
	double effort = 0.0;
	// -- Bigram stats
	double sfb = 0.0;
	double sfb_dist = 0.0;
	double lsb = 0.0;
	// -- Trigram stats
	double strong_alt = 0.0;
	double weak_alt = 0.0;
	double total_alt = 0.0;
	double strong_ratio = 0.0;
	double onelayer = 0.0;
	double onedir = 0.0;

	void make_keyboard(string& keys) {
		if (keys.length() != 30) {
			throw runtime_error("Expected 30 char for keys");
			exit(1);
		}
		this->KeyboardLayout = keys;
		for (int i = 0; i < 30; i++) {
			this->KeyboardMap[to_keyboard_loc(KeyboardLayout[i])][0] = i % 15 / 5;  // Row
			this->KeyboardMap[to_keyboard_loc(KeyboardLayout[i])][1] = i % 15 % 5;  // Column
			this->KeyboardMap[to_keyboard_loc(KeyboardLayout[i])][2] = i / 15;		// Layer
		}
	}

	void pretty_print() {
		string keys_pretty_print = "";
		for (int i = 0; i < 3; i++) {
			for (int j = i * 5; j < (i + 1) * 5; j++) {
				keys_pretty_print += this->KeyboardLayout[j];
				keys_pretty_print += ' ';
			}
			keys_pretty_print += "  ";
			for (int j = (i + 3) * 5; j < (i + 4) * 5; j++) {
				keys_pretty_print += this->KeyboardLayout[j];
				keys_pretty_print += ' ';
			}
			keys_pretty_print += '\n';
		}
		std::cout << "#   Keyboard Layout   #\n"
			<< "layer0\t\tlayer1\n"
			<< "-----------------------\n"
			<< keys_pretty_print << '\n'
			<< "# Keyboard Statistics #\n"
			<< "-----------------------\n";
		std::cout << "Effort: ";
		printf("%.5f\n", this->effort);
		std::cout << "Sfb: ";
		printf("%.5f\n", this->sfb);
		std::cout << "Sfb Average Distance: ";
		printf("%.5f\n", this->sfb_dist);
		std::cout << "Lsb: ";
		printf("%.5f\n", this->lsb);
		std::cout << "Alternates: ";
		printf("%.5f", this->strong_alt);
		std::cout << " | ";
		printf("%.5f\n", this->weak_alt);
		std::cout << "Total alternates: ";
		printf("%.5f\n", this->total_alt);
		std::cout << "Strong alt ratio: ";
		printf("%.5f\n", this->strong_ratio);
		std::cout << "OneLayer: ";
		printf("%.5f\n", this->onelayer);
		std::cout << "OneDirection: ";
		printf("%.5f\n", this->onedir);
	}

	/*
	Gets the keyboard stats of a given keyboard layout
	*/
	void get_stats() {
		this->get_monogram_stats();
		this->get_bigram_stats();
		this->get_trigram_stats();
	}

	/*
	Writes the keyboard stats to a text file
	*/
	void write_stats(ofstream& file) {
		file << "Keyboard Layout: " << this->KeyboardLayout << '\n'
			<< "Effort: " << this->effort << '\n'
			<< "Sfb: " << this->sfb << '\n'
			<< "SfbAvgDist: " << this->sfb_dist << '\n'
			<< "Lsb: " << this->lsb << '\n'
			<< "Alternates: " << this->strong_alt << " | " << this->weak_alt << '\n'
			<< "TotalAlt: " << this->total_alt << '\n'
			<< "StrongRatio: " << this->strong_ratio << '\n'
			<< "OneLayer: " << this->onelayer << '\n'
			<< "OneDirection: " << this->onedir << '\n';
	}

	short to_keyboard_loc(char key) {
		if (97 <= key && key < 123)
			return key - 97;
		switch (key) {
		case ';':
			return 26;
		case ',':
			return 27;
		case '.':
			return 28;
		case '\'':
			return 29;
		}
	}

	/*
	Gets the total typing effort of the whole keyboard and for individual fingers
	*/
	void get_monogram_stats() {
		double letter_freq;
		double letter_effort;

		for (int i = 0; i < 30; i++) {
			string key = "";
			key += this->KeyboardLayout[i];
			letter_freq = Monogram[key];

			letter_effort = EffortTable["key" + to_string(i)];
			this->effort += letter_freq * letter_effort;
		}
	}

	/*
	Sfb (Same finger bigram): pressing two keys in succession with the same finger
	Lsb (Lateral stretch bigram): a bigram typed with adjacent fingers, but that requires
	a lateral stretch since the two letters forming the bigram are far apart.
	*/
	void get_bigram_stats() {
		for (auto& bigram : Bigram.items()) {
			short row0 = this->KeyboardMap[to_keyboard_loc(bigram.key()[0])][0];
			short col0 = this->KeyboardMap[to_keyboard_loc(bigram.key()[0])][1];
			short row1 = this->KeyboardMap[to_keyboard_loc(bigram.key()[1])][0];
			short col1 = this->KeyboardMap[to_keyboard_loc(bigram.key()[1])][1];

			// Sfb: If same col or cols combination is 3,4, exclude same key
			if (col0 == col1 && bigram.key()[0] != bigram.key()[1]
					|| col0 == 3 && col1 == 4 || col0 == 4 && col1 == 3) {
				this->sfb += (double)bigram.value();
				// Sfb Dist: use distance formula sqrt(dX^2 + dY^2) * frequency
				this->sfb_dist += (double)sqrt(pow(row0 - row1, 2) + pow(col0 - col1, 2)) * (double)bigram.value();
			}

			// Lsb: Middle and Index on the middle column
			if (abs(col0 - col1) == 2 && (!col0 || !col1)) {
				this->lsb += (double)bigram.value();
			}
		}
		// Sfb Average distance : Total Sfb distance / Sfb %
		// Since self.sfb and freq are already in percentages, we don't have to convert them
		this->sfb_dist = this->sfb_dist / this->sfb;
	}

	/*
	Alternates: The trigrams typed by switching fingers
	Strong Ratio: The ratio of Strong Alternates to Total Alternates
	OneLayer: The trigram typed entirely on the Base Layer
	OneDirection: OneLayer trigrams that go one direction (either rolls inward or outward)
	*/
	void get_trigram_stats() {
		for (auto& trigram : Trigram.items()) {
			short col0 = this->KeyboardMap[to_keyboard_loc(trigram.key()[0])][1];
			short layer0 = this->KeyboardMap[to_keyboard_loc(trigram.key()[0])][2];
			short col1 = this->KeyboardMap[to_keyboard_loc(trigram.key()[1])][1];
			short layer1 = this->KeyboardMap[to_keyboard_loc(trigram.key()[1])][2];
			short col2 = this->KeyboardMap[to_keyboard_loc(trigram.key()[2])][1];
			short layer2 = this->KeyboardMap[to_keyboard_loc(trigram.key()[2])][2];

			short finger0 = this->finger_strength_index(col0);
			short finger1 = this->finger_strength_index(col1);
			short finger2 = this->finger_strength_index(col2);

			if (finger0 != finger1 && finger1 != finger2) {
				if ((finger0 + finger1 + finger2) <= 6)
					this->strong_alt += (double)trigram.value();
				else
					this->weak_alt += (double)trigram.value();
			}

			if (layer0 != 1 && layer1 != 1 && layer2 != 1) {
				this->onelayer += (double)trigram.value();
				if (col0 > col1 && col1 > col2 || col0 < col1 && col1 < col2)
					this->onedir += (double)trigram.value();
			}
		}

		this->total_alt = this->strong_alt + this->weak_alt;
		this->strong_ratio = this->strong_alt / this->total_alt;
	}

	short finger_strength_index(short col) {
		// Index finger col: 0/1 -> strength: 2
		if (col <= 1)
			return 2;
		// Ring/Pinky remain 3/4
		if (col >= 3)
			return col;
		// Middle finger col: 2 -> strength: 1
		if (col == 2)
			return 1;
	}
};

bool fullfilled_all_thres(Keyboard& keyboard, vector<double>& thresholds) {
	return (
		   keyboard.effort < thresholds[0]
		&& keyboard.sfb < thresholds[1]
		&& keyboard.sfb_dist < thresholds[2]
		&& keyboard.lsb < thresholds[3]
		&& keyboard.total_alt > thresholds[4]
		&& keyboard.strong_ratio > thresholds[5]
		&& keyboard.onelayer > thresholds[6]
		&& keyboard.onedir > thresholds[7]
		);
}

void auto_gen(void) {
	string remaining_str = Constraints["remaining"];
	string keyboard_str = Constraints["keyboard"];
	vector<vector<double>> keyboard_stats_lists(8);
	for (int i = 0; i < 8; i++) {
		keyboard_stats_lists[i].resize(1000);
	}
	vector<double> thresholds(8);

	ofstream write_file;
	string write_file_path = "gen" + to_string(Constraints["generation"]) + ".txt";
	write_file.open(write_file_path);
	int iterations = Constraints["iterations"] + 1000;

	// Set random seed to time
	srand(time(0));

	for (int i = 0; i < iterations; i++) {
		string filled_kb_text = keyboard_str;

		// Shuffle constrained keys
		for (auto& constraint : Constraints["constraints"].items()) {
			random_shuffle(constraint.value().begin(), constraint.value().end());
			int j;
			for (j = 0; j < constraint.value().size(); j++) {
				if (filled_kb_text[constraint.value()[j]] == '_')
					break;
			}
			char key = constraint.key()[0];
			if (j == constraint.value().size()) {
				cout << key;
				throw runtime_error("Letter busted");
			}
			filled_kb_text[constraint.value()[j]] = constraint.key()[0];
		}

		// Substitute remaining keys
		short remaining_counter = 0;
		random_shuffle(remaining_str.begin(), remaining_str.end());
		for (int j = 0; j < 30; j++) {
			if (filled_kb_text[j] == '_') {
				filled_kb_text[j] = remaining_str[remaining_counter];
				remaining_counter++;
			}
		}

		// Get Keyboard stats
		Keyboard keyboard;
		keyboard.make_keyboard(filled_kb_text);
		keyboard.get_stats();

		if (i <= 1000) {
			keyboard_stats_lists[0][i] = keyboard.effort;
			keyboard_stats_lists[1][i] = keyboard.sfb;
			keyboard_stats_lists[2][i] = keyboard.sfb_dist;
			keyboard_stats_lists[3][i] = keyboard.lsb;
			keyboard_stats_lists[4][i] = keyboard.total_alt;
			keyboard_stats_lists[5][i] = keyboard.strong_ratio;
			keyboard_stats_lists[6][i] = keyboard.onelayer;
			keyboard_stats_lists[7][i] = keyboard.onedir;

			if (i == 1000) {
				for (int j = 0; j < 8; j++) {
					sort(keyboard_stats_lists[j].begin(), keyboard_stats_lists[j].end());
				}
				thresholds[0] = keyboard_stats_lists[0][(int)(1000 * (double)CutoffScore["effort"])];
				thresholds[1] = keyboard_stats_lists[1][(int)(1000 * (double)CutoffScore["sfb"])];
				thresholds[2] = keyboard_stats_lists[2][(int)(1000 * (double)CutoffScore["sfb_dist"])];
				thresholds[3] = keyboard_stats_lists[3][(int)(1000 * (double)CutoffScore["lsb"])];
				thresholds[4] = keyboard_stats_lists[4][(int)(1000 * (1.0 - (double)CutoffScore["total_alt"]))];
				thresholds[5] = keyboard_stats_lists[5][(int)(1000 * (1.0 - (double)CutoffScore["strong_ratio"]))];
				thresholds[6] = keyboard_stats_lists[6][(int)(1000 * (1.0 - (double)CutoffScore["onelayer"]))];
				thresholds[7] = keyboard_stats_lists[7][(int)(1000 * (1.0 - (double)CutoffScore["onedir"]))];
				vector<string> thres_names = { "Effort: ", "Sfb: ", "Sfb Average Distance: ", "Lsb: ",
											   "Total alternates: ", "Strong alt ratio: ", "OneLayer: ", "OneDirection: " };
				for (int j = 0; j < 8; j++) {
					cout << thres_names[j] << thresholds[j] << endl;
				}
				continue;
			}
		}

		if (!fullfilled_all_thres(keyboard, thresholds))
			continue;
		keyboard.write_stats(write_file);
	}
}


int main() {
	cout << "Generation: " << Constraints["generation"] << endl;
	cout << "Iterations: " << Constraints["iterations"] << endl;
	time_t preTime = time(0) + (time_t)Constraints["iterations"] * 0.0004;
	int preTime_h = (preTime / 3600) % 24;
	int preTime_m = (preTime / 60) % 60;
	int preTime_s = preTime % 60;
	cout << "Estimated finish time: ";
	printf("UTC %02d:%02d:%02d\n", preTime_h, preTime_m, preTime_s);

    auto t0 = Time::now();

	auto_gen();

    auto t1 = Time::now();
    fsec fs = t1 - t0;
    ms d = chrono::duration_cast<ms>(fs);
    std::cout << "Time elapsed: " << d.count() << "ms\n";
    std::cin.get();
    return 0;
}