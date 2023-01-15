#include <Windows.h>
#include <cmath>
#include <chrono>
#include <iostream>
#include <fstream>
#include <nlohmann\json.hpp>
#include <string>
#include <sstream>
#include <vector>


using namespace std;
using json = nlohmann::json;


ifstream effortTableJson("keyboard_effort_table.json");
json EffortTable = json::parse(effortTableJson);
ifstream monogramJson("n1gram_frequency.json");
json Monogram = json::parse(monogramJson);
ifstream bigramJson("n2gram_frequency.json");
json Bigram = json::parse(bigramJson);
ifstream trigramJson("n3gram_frequency.json");
json Trigram = json::parse(trigramJson);


vector<vector<int>> keyboard_location_map(string &keys) {
    vector<vector<int>> kb_map(30);
    for (int i = 0; i < 30; i++) {
        kb_map[i].resize(3);
    }
    for (int i = 0; i < 30; i++) {
        int ordinal = (int)keys[i] - 97;
        if (ordinal < 0 || ordinal > 25)
            continue;

        int layer = (int)floor((double)i / (double)15);
        int row = (int)floor((double)(i % 15) / (double)5);
        int col = i % 15 % 5;
        kb_map[ordinal][0] = layer;
        kb_map[ordinal][1] = row;
        kb_map[ordinal][2] = col;
    }
    for (int i = 26; i < 30; i++) {
        for (int j = 0; j < 3; j++) {
            kb_map[i][j] = -1;
        }
    }
    return kb_map;
}

vector<double> get_performance(string &keys) {
    double sfb_counter = 0.0;
    double lsb_counter = 0.0;
    double inroll_counter = 0.0;
    double outroll_counter = 0.0;
    double effort_counter = 0.0;
    vector<vector<int>> kb_map = keyboard_location_map(keys);

    // Get Effort
    int ordinal;
    for (int i = 0; i < 30; i++) {
        ordinal = (int)keys[i];
        if (ordinal < 97 || ordinal > 123)
            continue;
        string key = "";
        key += keys[i];
        double letter_freq = Monogram[key];
        double letter_effort = EffortTable["key" + to_string(i)];
        effort_counter += letter_freq * letter_effort;
    }

    // Get Sfb and Lsb

    for (auto& bigram : Bigram.items()) {
        vector<int> pos0 = kb_map[(int)(bigram.key()[0]) - 97];
        vector<int> pos1 = kb_map[(int)(bigram.key()[1]) - 97];

        int row0 = pos0[1];
        int row1 = pos1[1];
        int col0 = pos0[2];
        int col1 = pos1[2];

        if ((col0 == col1) || (col0 == 0 && col1 == 1) || (col0 == 1 && col1 == 0)) {
            sfb_counter += (double)bigram.value();
        }
        if (abs(row0 - row1) == 2 || (col0 == 0 && row0 != 1 || col1 == 0 && row1 != 1 &&
            abs(row0 - row1) >= 1)) {
            lsb_counter += (double)bigram.value();
        }
    }

    // Get Inroll and Outroll

    for (auto& trigram : Trigram.items()) {
        vector<int> pos0 = kb_map[(int)(trigram.key()[0]) - 97];
        vector<int> pos1 = kb_map[(int)(trigram.key()[1]) - 97];
        vector<int> pos2 = kb_map[(int)(trigram.key()[2]) - 97];

        int col0 = pos0[2];
        int col1 = pos1[2];
        int col2 = pos2[2];

        if ((col0 == 0 || col1 == 0 || col2 == 0) && (col0 == 1 || col1 == 1 || col2 == 1))
            continue;
        if (col0 > col1 && col1 > col2) {
            inroll_counter += (double)trigram.value();
        }
        if (col0 < col1 && col1 < col2) {
            outroll_counter += (double)trigram.value();
        }
    }

    vector<double> performance = { sfb_counter, lsb_counter, inroll_counter,
    outroll_counter, inroll_counter - outroll_counter,
    inroll_counter + outroll_counter, effort_counter };
    return performance;
}

void print_percentile(ofstream &file, vector<double> &sorted_data, int &data_len, string &category) {
    file << category << ':' << endl;
    file << "\tTop 1%: " << sorted_data[(int)(data_len * 0.99)] << endl;
    file << "\tTop 10%: " << sorted_data[(int)(data_len * 0.90)] << endl;
    file << "\tTop 15%: " << sorted_data[(int)(data_len * 0.85)] << endl;
    file << "\tUpper quartile: " << sorted_data[(int)(data_len * 0.75)] << endl;
    file << "\tMedian: " << sorted_data[(int)(data_len * 0.5)] << endl;
    file << "\tLower quartile: " << sorted_data[(int)(data_len * 0.25)] << endl;
    file << "\tBottom 15%: " << sorted_data[(int)(data_len * 0.15)] << endl;
    file << "\tBottom 10%: " << sorted_data[(int)(data_len * 0.10)] << endl;
    file << "\tBottom 1%: " << sorted_data[(int)(data_len * 0.01)] << endl;

    cout << category << ':' << endl;
    cout << "\tTop 1%: " << sorted_data[(int)(data_len * 0.99)] << endl;
    cout << "\tTop 10%: " << sorted_data[(int)(data_len * 0.90)] << endl;
    cout << "\tTop 15%: " << sorted_data[(int)(data_len * 0.85)] << endl;
    cout << "\tUpper quartile: " << sorted_data[(int)(data_len * 0.75)] << endl;
    cout << "\tMedian: " << sorted_data[(int)(data_len * 0.5)] << endl;
    cout << "\tLower quartile: " << sorted_data[(int)(data_len * 0.25)] << endl;
    cout << "\tBotton 15%: " << sorted_data[(int)(data_len * 0.15)] << endl;
    cout << "\tBotton 10%: " << sorted_data[(int)(data_len * 0.10)] << endl;
    cout << "\tBottom 1%: " << sorted_data[(int)(data_len * 0.01)] << endl;
}

void frogpad_control() {
    string keys = "farwpoehtduinsy--bmjqzlcv--kgx";
    vector<double> performance = get_performance(keys);
    cout << "Sfb:\t" << performance[0] << endl <<
            "Lsb:\t" << performance[1] << endl <<
            "I/O Difference:\t" << performance[4] << endl <<
            "Rolls:\t" << performance[5] << endl <<
            "Effort:\t" << performance[6] << endl;
}

void gen1_stats(int &iterations) {
    vector<double> sfb_data(iterations);
    vector<double> lsb_data(iterations);
    vector<double> io_diff_data(iterations);
    vector<double> rolls_data(iterations);
    vector<double> effort_data(iterations);
    ofstream stats_file;
    stats_file.open("gen1_stats.txt");

    string keys_str = "abcdefghijklmnopqrstuvwxyz----";

    for (int i = 0; i < iterations; i++) {
        random_shuffle(keys_str.begin(), keys_str.end());

        string keys_combination = keys_str.c_str();
        vector<double> performance = get_performance(keys_combination);

        sfb_data[i] = performance[0];
        lsb_data[i] = performance[1];
        // performance [2], [3] are inroll and outroll
        io_diff_data[i] = performance[4];
        rolls_data[i] = performance[5];
        effort_data[i] = performance[6];
    }
    

    sort(sfb_data.begin(), sfb_data.end());
    sort(lsb_data.begin(), lsb_data.end());
    sort(io_diff_data.begin(), io_diff_data.end());
    sort(rolls_data.begin(), rolls_data.end());
    sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, iterations, sfb_text);
    print_percentile(stats_file, lsb_data, iterations, lsb_text);
    print_percentile(stats_file, io_diff_data, iterations, iodiff_text);
    print_percentile(stats_file, rolls_data, iterations, rolls_text);
    print_percentile(stats_file, effort_data, iterations, effort_text);
}

void gen1_write(int &iterations, vector<double> &thresholds) {
    string keys_str = "abcdefghijklmnopqrstuvwxyz----";
    ofstream kb_file;
    kb_file.open("gen1.txt");
    for (int i = 0; i < iterations; i++) {
        random_shuffle(keys_str.begin(), keys_str.end());

        vector<double> performance = get_performance(keys_str);

        if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
            thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
            thresholds[4] < performance[6]) {
            continue;
        }

        kb_file << "Keyboard layout: " << keys_str << '\n' <<
            "Sfb: " << performance[0] << '\n' <<
            "Lsb: " << performance[1] << '\n' <<
            "Inward roll: " << performance[2] << ' ' <<
            "Outward roll: " << performance[3] << '\n' <<
            "I/O Difference: " << performance[4] << '\n' <<
            "Total roll: " << performance[5] << '\n' <<
            "Effort: " << performance[6] << endl;
    }
}

void gen2_stats(int& iterations) {
    vector<double> sfb_data(iterations);
    vector<double> lsb_data(iterations);
    vector<double> io_diff_data(iterations);
    vector<double> rolls_data(iterations);
    vector<double> effort_data(iterations);
    ofstream stats_file;
    stats_file.open("gen2_stats.txt");

    string kb_text = "_______e_t____________________";
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    string nr_str = "nr";
    vector<int> nr_loc = { 5, 6, 8, 20, 21, 22, 23, 24 };
    int o_loc; int dash_loc;

    string remaining_keys = "abcdfghijklmpuvwxyz";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(s_loc.begin(), s_loc.end()); }
        while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';

        for (int j = 0; j < nr_str.length(); j++) {
            do { random_shuffle(nr_loc.begin(), nr_loc.end()); }
            while (filled_kb_text[nr_loc[0]] != '_');
            filled_kb_text[nr_loc[0]] = nr_str[j];
        }

        do { random_shuffle(q_loc.begin(), q_loc.end()); }
        while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        // rand o_loc between 0 and 14
        do { o_loc = rand() % 15; }
        while (filled_kb_text[o_loc] != '_');
        filled_kb_text[o_loc] = 'o';
        // rand dash_loc 4 times between 15 and 29
        for (int j = 0; j < 4; j++) {
            do { dash_loc = rand() % 15 + 15; }
            while (filled_kb_text[dash_loc] != '_');
            filled_kb_text[dash_loc] = '-';
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }
        
        vector<double> performance = get_performance(filled_kb_text);

        sfb_data[i] = performance[0];
        lsb_data[i] = performance[1];
        // performance [2], [3] are inroll and outroll
        io_diff_data[i] = performance[4];
        rolls_data[i] = performance[5];
        effort_data[i] = performance[6];
    }


    sort(sfb_data.begin(), sfb_data.end());
    sort(lsb_data.begin(), lsb_data.end());
    sort(io_diff_data.begin(), io_diff_data.end());
    sort(rolls_data.begin(), rolls_data.end());
    sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, iterations, sfb_text);
    print_percentile(stats_file, lsb_data, iterations, lsb_text);
    print_percentile(stats_file, io_diff_data, iterations, iodiff_text);
    print_percentile(stats_file, rolls_data, iterations, rolls_text);
    print_percentile(stats_file, effort_data, iterations, effort_text);
}

void gen2_write(int& iterations, vector<double>& thresholds) {
    ofstream kb_file;
    kb_file.open("gen2.txt");

    string kb_text = "_______e_t____________________";
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    string nr_str = "nr";
    vector<int> nr_loc = { 5, 6, 8, 20, 21, 22, 23, 24 };
    int o_loc; int dash_loc;

    string remaining_keys = "abcdfghijklmpuvwxyz";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(s_loc.begin(), s_loc.end()); } while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';

        for (int j = 0; j < nr_str.length(); j++) {
            do { random_shuffle(nr_loc.begin(), nr_loc.end()); } while (filled_kb_text[nr_loc[0]] != '_');
            filled_kb_text[nr_loc[0]] = nr_str[j];
        }

        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        // rand o_loc between 0 and 14
        do { o_loc = rand() % 15; } while (filled_kb_text[o_loc] != '_');
        filled_kb_text[o_loc] = 'o';
        // rand dash_loc 4 times between 15 and 29
        for (int j = 0; j < 4; j++) {
            do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
            filled_kb_text[dash_loc] = '-';
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
            thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
            thresholds[4] < performance[6]) {
            continue;
        }

        kb_file << "Keyboard layout: " << filled_kb_text << '\n' <<
            "Sfb: " << performance[0] << '\n' <<
            "Lsb: " << performance[1] << '\n' <<
            "Inward roll: " << performance[2] << ' ' <<
            "Outward roll: " << performance[3] << '\n' <<
            "I/O Difference: " << performance[4] << '\n' <<
            "Total roll: " << performance[5] << '\n' <<
            "Effort: " << performance[6] << endl;
    }
}

void gen3_stats(int& iterations) {
    vector<double> sfb_data(iterations);
    vector<double> lsb_data(iterations);
    vector<double> io_diff_data(iterations);
    vector<double> rolls_data(iterations);
    vector<double> effort_data(iterations);
    ofstream stats_file;
    stats_file.open("gen3_stats.txt");

    string kb_text = "_______e_t____________________";
    vector<int> a_loc = { 7, 22 };
    vector<int> n_loc = { 6, 21 };
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> o_loc = { 2, 7, 12 };
    vector<int> h_loc = { 3, 8, 13 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    string ir_str = "ir";
    vector<int> ir_loc = { 5, 6, 8, 20, 21, 22, 23, 24 };
    int x_loc; int dash_loc;

    string remaining_keys = "bcdfgjklmpuvwyz";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(a_loc.begin(), a_loc.end()); } while (filled_kb_text[a_loc[0]] != '_');
        filled_kb_text[a_loc[0]] = 'a';
        do { random_shuffle(n_loc.begin(), n_loc.end()); } while (filled_kb_text[n_loc[0]] != '_');
        filled_kb_text[n_loc[0]] = 'n';
        do { random_shuffle(s_loc.begin(), s_loc.end()); } while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';
        do { random_shuffle(o_loc.begin(), o_loc.end()); } while (filled_kb_text[o_loc[0]] != '_');
        filled_kb_text[o_loc[0]] = 'o';
        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        for (int j = 0; j < ir_str.length(); j++) {
            do { random_shuffle(ir_loc.begin(), ir_loc.end()); } while (filled_kb_text[ir_loc[0]] != '_');
            filled_kb_text[ir_loc[0]] = ir_str[j];
        }

        // rand x_loc between 0 and 14
        do { x_loc = rand() % 15; } while (filled_kb_text[x_loc] != '_');
        filled_kb_text[x_loc] = 'x';
        // rand dash_loc 4 times between 15 and 29
        for (int j = 0; j < 4; j++) {
            do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
            filled_kb_text[dash_loc] = '-';
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        sfb_data[i] = performance[0];
        lsb_data[i] = performance[1];
        // performance [2], [3] are inroll and outroll
        io_diff_data[i] = performance[4];
        rolls_data[i] = performance[5];
        effort_data[i] = performance[6];
    }


    sort(sfb_data.begin(), sfb_data.end());
    sort(lsb_data.begin(), lsb_data.end());
    sort(io_diff_data.begin(), io_diff_data.end());
    sort(rolls_data.begin(), rolls_data.end());
    sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, iterations, sfb_text);
    print_percentile(stats_file, lsb_data, iterations, lsb_text);
    print_percentile(stats_file, io_diff_data, iterations, iodiff_text);
    print_percentile(stats_file, rolls_data, iterations, rolls_text);
    print_percentile(stats_file, effort_data, iterations, effort_text);
}

void gen3_write(int& iterations, vector<double>& thresholds) {
    ofstream kb_file;
    kb_file.open("gen3.txt");

    string kb_text = "_______e_t____________________";
    vector<int> a_loc = { 7, 22 };
    vector<int> n_loc = { 6, 21 };
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> o_loc = { 2, 7, 12 };
    vector<int> h_loc = { 3, 8, 13 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    string ir_str = "ir";
    vector<int> ir_loc = { 5, 6, 8, 20, 21, 22, 23, 24 };
    int x_loc; int dash_loc;

    string remaining_keys = "bcdfgjklmpuvwyz";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(a_loc.begin(), a_loc.end()); } while (filled_kb_text[a_loc[0]] != '_');
        filled_kb_text[a_loc[0]] = 'a';
        do { random_shuffle(n_loc.begin(), n_loc.end()); } while (filled_kb_text[n_loc[0]] != '_');
        filled_kb_text[n_loc[0]] = 'n';
        do { random_shuffle(s_loc.begin(), s_loc.end()); } while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';
        do { random_shuffle(o_loc.begin(), o_loc.end()); } while (filled_kb_text[o_loc[0]] != '_');
        filled_kb_text[o_loc[0]] = 'o';
        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        for (int j = 0; j < ir_str.length(); j++) {
            do { random_shuffle(ir_loc.begin(), ir_loc.end()); } while (filled_kb_text[ir_loc[0]] != '_');
            filled_kb_text[ir_loc[0]] = ir_str[j];
        }

        // rand o_loc between 0 and 14
        do { x_loc = rand() % 15; } while (filled_kb_text[x_loc] != '_');
        filled_kb_text[x_loc] = 'x';
        // rand dash_loc 4 times between 15 and 29
        for (int j = 0; j < 4; j++) {
            do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
            filled_kb_text[dash_loc] = '-';
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
            thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
            thresholds[4] < performance[6]) {
            continue;
        }

        kb_file << "Keyboard layout: " << filled_kb_text << '\n' <<
            "Sfb: " << performance[0] << '\n' <<
            "Lsb: " << performance[1] << '\n' <<
            "Inward roll: " << performance[2] << ' ' <<
            "Outward roll: " << performance[3] << '\n' <<
            "I/O Difference: " << performance[4] << '\n' <<
            "Total roll: " << performance[5] << '\n' <<
            "Effort: " << performance[6] << endl;
    }
}

void gen4_stats(int& iterations) {
    vector<double> sfb_data(iterations);
    vector<double> lsb_data(iterations);
    vector<double> io_diff_data(iterations);
    vector<double> rolls_data(iterations);
    vector<double> effort_data(iterations);
    ofstream stats_file;
    stats_file.open("gen4_stats.txt");

    string kb_text = "_______e_t____________a_______";
    vector<int> n_loc = { 6, 21 };
    vector<int> i_loc = { 8, 23 };
    vector<int> o_loc = { 2, 12 };
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> h_loc = { 3, 8, 13 };
    vector<int> x_loc = { 15, 20, 25 };
    vector<int> l_loc = { 1, 6, 11, 16, 21, 26 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    vector<int> r_loc = { 5, 6, 8, 20, 21, 23, 24 };
    string cw_str = "cw";
    vector<int> cw_loc = { 4, 14, 19, 24, 29 };
    string jkz_str = "jkz";
    int jkz_loc; int dash_loc; int vacancy;

    string remaining_keys = "bdfgmpuvy";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(n_loc.begin(), n_loc.end()); } while (filled_kb_text[n_loc[0]] != '_');
        filled_kb_text[n_loc[0]] = 'n';
        do { random_shuffle(i_loc.begin(), i_loc.end()); } while (filled_kb_text[i_loc[0]] != '_');
        filled_kb_text[i_loc[0]] = 'i';
        do { random_shuffle(o_loc.begin(), o_loc.end()); } while (filled_kb_text[o_loc[0]] != '_');
        filled_kb_text[o_loc[0]] = 'o';
        do { random_shuffle(s_loc.begin(), s_loc.end()); } while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';
        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(x_loc.begin(), x_loc.end()); } while (filled_kb_text[x_loc[0]] != '_');
        filled_kb_text[x_loc[0]] = 'x';
        do { random_shuffle(l_loc.begin(), l_loc.end()); } while (filled_kb_text[l_loc[0]] != '_');
        filled_kb_text[l_loc[0]] = 'l';
        do { random_shuffle(r_loc.begin(), r_loc.end()); } while (filled_kb_text[r_loc[0]] != '_');
        filled_kb_text[r_loc[0]] = 'r';

        // rand jkz between 15 and 29
        for (int j = 0; j < jkz_str.length(); j++) {
            do { jkz_loc = rand() % 15 + 15; } while (filled_kb_text[jkz_loc] != '_');
            filled_kb_text[jkz_loc] = jkz_str[j];
        }

        // rand dash_loc 4 times between 15 and 29
        vacancy = 0;
        for (int j = 15; j < 30; j++) {
            if (filled_kb_text[j] == '_') { vacancy++; }
        }
        if (vacancy != 4) {
            for (int j = 0; j < 4; j++) {
                do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
                filled_kb_text[dash_loc] = '-';
            }
        } else {
            for (int j = 15; j < 30; j++) {
                if (filled_kb_text[j] == '_')
                    filled_kb_text[dash_loc] = '-';
            }
        }

        for (int j = 0; j < cw_str.length(); j++) {
            do { random_shuffle(cw_loc.begin(), cw_loc.end()); } while (filled_kb_text[cw_loc[0]] != '_');
            filled_kb_text[cw_loc[0]] = cw_str[j];
        }

        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        sfb_data[i] = performance[0];
        lsb_data[i] = performance[1];
        // performance [2], [3] are inroll and outroll
        io_diff_data[i] = performance[4];
        rolls_data[i] = performance[5];
        effort_data[i] = performance[6];
    }


    sort(sfb_data.begin(), sfb_data.end());
    sort(lsb_data.begin(), lsb_data.end());
    sort(io_diff_data.begin(), io_diff_data.end());
    sort(rolls_data.begin(), rolls_data.end());
    sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, iterations, sfb_text);
    print_percentile(stats_file, lsb_data, iterations, lsb_text);
    print_percentile(stats_file, io_diff_data, iterations, iodiff_text);
    print_percentile(stats_file, rolls_data, iterations, rolls_text);
    print_percentile(stats_file, effort_data, iterations, effort_text);
}

void gen4_write(int& iterations, vector<double>& thresholds) {
    ofstream kb_file;
    kb_file.open("gen4.txt");

    string kb_text = "_______e_t____________a_______";
    vector<int> n_loc = { 6, 21 };
    vector<int> i_loc = { 8, 23 };
    vector<int> o_loc = { 2, 12 };
    vector<int> s_loc = { 1, 6, 11 };
    vector<int> h_loc = { 3, 8, 13 };
    vector<int> x_loc = { 15, 20, 25 };
    vector<int> l_loc = { 1, 6, 11, 16, 21, 26 };
    vector<int> q_loc = { 0, 5, 10, 15, 20, 25 };
    vector<int> r_loc = { 5, 6, 8, 20, 21, 23, 24 };
    string cw_str = "cw";
    vector<int> cw_loc = { 4, 14, 19, 24, 29 };
    string jkz_str = "jkz";
    int jkz_loc; int dash_loc; int vacancy;

    string remaining_keys = "bdfgmpuvy";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(n_loc.begin(), n_loc.end()); } while (filled_kb_text[n_loc[0]] != '_');
        filled_kb_text[n_loc[0]] = 'n';
        do { random_shuffle(i_loc.begin(), i_loc.end()); } while (filled_kb_text[i_loc[0]] != '_');
        filled_kb_text[i_loc[0]] = 'i';
        do { random_shuffle(o_loc.begin(), o_loc.end()); } while (filled_kb_text[o_loc[0]] != '_');
        filled_kb_text[o_loc[0]] = 'o';
        do { random_shuffle(s_loc.begin(), s_loc.end()); } while (filled_kb_text[s_loc[0]] != '_');
        filled_kb_text[s_loc[0]] = 's';
        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(x_loc.begin(), x_loc.end()); } while (filled_kb_text[x_loc[0]] != '_');
        filled_kb_text[x_loc[0]] = 'x';
        do { random_shuffle(l_loc.begin(), l_loc.end()); } while (filled_kb_text[l_loc[0]] != '_');
        filled_kb_text[l_loc[0]] = 'l';
        do { random_shuffle(r_loc.begin(), r_loc.end()); } while (filled_kb_text[r_loc[0]] != '_');
        filled_kb_text[r_loc[0]] = 'r';

        // rand jkz between 15 and 29
        for (int j = 0; j < jkz_str.length(); j++) {
            do { jkz_loc = rand() % 15 + 15; } while (filled_kb_text[jkz_loc] != '_');
            filled_kb_text[jkz_loc] = jkz_str[j];
        }

        // rand dash_loc 4 times between 15 and 29
        vacancy = 0;
        for (int j = 15; j < 30; j++) {
            if (filled_kb_text[j] == '_') { vacancy++; }
        }
        if (vacancy != 4) {
            for (int j = 0; j < 4; j++) {
                do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
                filled_kb_text[dash_loc] = '-';
            }
        }
        else {
            for (int j = 15; j < 30; j++) {
                if (filled_kb_text[j] == '_')
                    filled_kb_text[dash_loc] = '-';
            }
        }

        for (int j = 0; j < cw_str.length(); j++) {
            do { random_shuffle(cw_loc.begin(), cw_loc.end()); } while (filled_kb_text[cw_loc[0]] != '_');
            filled_kb_text[cw_loc[0]] = cw_str[j];
        }

        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
            thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
            thresholds[4] < performance[6]) {
            continue;
        }

        kb_file << "Keyboard layout: " << filled_kb_text << '\n' <<
            "Sfb: " << performance[0] << '\n' <<
            "Lsb: " << performance[1] << '\n' <<
            "Inward roll: " << performance[2] << ' ' <<
            "Outward roll: " << performance[3] << '\n' <<
            "I/O Difference: " << performance[4] << '\n' <<
            "Total roll: " << performance[5] << '\n' <<
            "Effort: " << performance[6] << endl;
    }
}

void gen5_stats(int& iterations) {
    vector<double> sfb_data(iterations);
    vector<double> lsb_data(iterations);
    vector<double> io_diff_data(iterations);
    vector<double> rolls_data(iterations);
    vector<double> effort_data(iterations);
    ofstream stats_file;
    stats_file.open("gen5_stats.txt");

    string kb_text = "_____rneit__________x_a_______";
    vector<int> h_loc = { 3, 13 };
    vector<int> q_loc = { 0, 10 };
    vector<int> p_loc = { 3, 13, 18, 23, 28 };
    string ou_str = "ou";
    vector<int> ou_loc = { 2, 12 };
    string ds_str = "ds";
    vector<int> ds_loc = { 1, 11 };
    vector<int> l_loc = { 11, 16, 21, 26 };
    string cmw_str = "cmw";
    vector<int> cmw_loc = { 4, 14, 19, 24, 29 };
    string jkz_str = "jkz";
    int jkz_loc; int dash_loc; int vacancy;

    string remaining_keys = "bfgvy";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';
        do { random_shuffle(p_loc.begin(), p_loc.end()); } while (filled_kb_text[p_loc[0]] != '_');
        filled_kb_text[p_loc[0]] = 'p';

        for (int j = 0; j < ou_str.length(); j++) {
            do { random_shuffle(ou_loc.begin(), ou_loc.end()); } while (filled_kb_text[ou_loc[0]] != '_');
            filled_kb_text[ou_loc[0]] = ou_str[j];
        }
        for (int j = 0; j < ds_str.length(); j++) {
            do { random_shuffle(ds_loc.begin(), ds_loc.end()); } while (filled_kb_text[ds_loc[0]] != '_');
            filled_kb_text[ds_loc[0]] = ds_str[j];
        }
        for (int j = 0; j < cmw_str.length(); j++) {
            do { random_shuffle(cmw_loc.begin(), cmw_loc.end()); } while (filled_kb_text[cmw_loc[0]] != '_');
            filled_kb_text[cmw_loc[0]] = cmw_str[j];
        }

        do { random_shuffle(l_loc.begin(), l_loc.end()); } while (filled_kb_text[l_loc[0]] != '_');
        filled_kb_text[l_loc[0]] = 'l';

        // rand jkz between 15 and 29
        for (int j = 0; j < jkz_str.length(); j++) {
            do { jkz_loc = rand() % 15 + 15; } while (filled_kb_text[jkz_loc] != '_');
            filled_kb_text[jkz_loc] = jkz_str[j];
        }

        // rand dash_loc 4 times between 15 and 29
        // substitute if 15~29 remaining is 4
        vacancy = 0;
        for (int j = 15; j < 30; j++) {
            if (filled_kb_text[j] == '_') { vacancy++; }
        }
        if (vacancy != 4) {
            for (int j = 0; j < 4; j++) {
                do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
                filled_kb_text[dash_loc] = '-';
            }
        }
        else {
            for (int j = 15; j < 30; j++) {
                if (filled_kb_text[j] == '_')
                    filled_kb_text[dash_loc] = '-';
            }
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        sfb_data[i] = performance[0];
        lsb_data[i] = performance[1];
        // performance [2], [3] are inroll and outroll
        io_diff_data[i] = performance[4];
        rolls_data[i] = performance[5];
        effort_data[i] = performance[6];
    }


    sort(sfb_data.begin(), sfb_data.end());
    sort(lsb_data.begin(), lsb_data.end());
    sort(io_diff_data.begin(), io_diff_data.end());
    sort(rolls_data.begin(), rolls_data.end());
    sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, iterations, sfb_text);
    print_percentile(stats_file, lsb_data, iterations, lsb_text);
    print_percentile(stats_file, io_diff_data, iterations, iodiff_text);
    print_percentile(stats_file, rolls_data, iterations, rolls_text);
    print_percentile(stats_file, effort_data, iterations, effort_text);
}

void gen5_write(int& iterations, vector<double>& thresholds) {
    ofstream kb_file;
    kb_file.open("gen5.txt");

    string kb_text = "_____rneit__________x_a_______";
    vector<int> h_loc = { 3, 13 };
    vector<int> q_loc = { 0, 10 };
    vector<int> p_loc = { 3, 13, 18, 23, 28 };
    string ou_str = "ou";
    vector<int> ou_loc = { 2, 12 };
    string ds_str = "ds";
    vector<int> ds_loc = { 1, 11 };
    vector<int> l_loc = { 11, 16, 21, 26 };
    string cmw_str = "cmw";
    vector<int> cmw_loc = { 4, 14, 19, 24, 29 };
    string jkz_str = "jkz";
    int jkz_loc; int dash_loc; int vacancy;

    string remaining_keys = "bfgvy";

    for (int i = 0; i < iterations; i++) {
        string filled_kb_text = kb_text;

        do { random_shuffle(h_loc.begin(), h_loc.end()); } while (filled_kb_text[h_loc[0]] != '_');
        filled_kb_text[h_loc[0]] = 'h';
        do { random_shuffle(q_loc.begin(), q_loc.end()); } while (filled_kb_text[q_loc[0]] != '_');
        filled_kb_text[q_loc[0]] = 'q';
        do { random_shuffle(p_loc.begin(), p_loc.end()); } while (filled_kb_text[p_loc[0]] != '_');
        filled_kb_text[p_loc[0]] = 'p';

        for (int j = 0; j < ou_str.length(); j++) {
            do { random_shuffle(ou_loc.begin(), ou_loc.end()); } while (filled_kb_text[ou_loc[0]] != '_');
            filled_kb_text[ou_loc[0]] = ou_str[j];
        }
        for (int j = 0; j < ds_str.length(); j++) {
            do { random_shuffle(ds_loc.begin(), ds_loc.end()); } while (filled_kb_text[ds_loc[0]] != '_');
            filled_kb_text[ds_loc[0]] = ds_str[j];
        }
        for (int j = 0; j < cmw_str.length(); j++) {
            do { random_shuffle(cmw_loc.begin(), cmw_loc.end()); } while (filled_kb_text[cmw_loc[0]] != '_');
            filled_kb_text[cmw_loc[0]] = cmw_str[j];
        }

        do { random_shuffle(l_loc.begin(), l_loc.end()); } while (filled_kb_text[l_loc[0]] != '_');
        filled_kb_text[l_loc[0]] = 'l';

        // rand jkz between 15 and 29
        for (int j = 0; j < jkz_str.length(); j++) {
            do { jkz_loc = rand() % 15 + 15; } while (filled_kb_text[jkz_loc] != '_');
            filled_kb_text[jkz_loc] = jkz_str[j];
        }

        // rand dash_loc 4 times between 15 and 29
        // substitute if 15~29 remaining is 4
        vacancy = 0;
        for (int j = 15; j < 30; j++) {
            if (filled_kb_text[j] == '_') { vacancy++; }
        }
        if (vacancy != 4) {
            for (int j = 0; j < 4; j++) {
                do { dash_loc = rand() % 15 + 15; } while (filled_kb_text[dash_loc] != '_');
                filled_kb_text[dash_loc] = '-';
            }
        }
        else {
            for (int j = 15; j < 30; j++) {
                if (filled_kb_text[j] == '_')
                    filled_kb_text[dash_loc] = '-';
            }
        }

        // Substitute remaining keys
        random_shuffle(remaining_keys.begin(), remaining_keys.end());
        int remaining_counter = 0;
        for (int j = 0; j < 30; j++) {
            if (filled_kb_text[j] == '_') {
                filled_kb_text[j] = remaining_keys[remaining_counter];
                remaining_counter++;
            }
        }

        vector<double> performance = get_performance(filled_kb_text);

        if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
            thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
            thresholds[4] < performance[6]) {
            continue;
        }

        kb_file << "Keyboard layout: " << filled_kb_text << '\n' <<
            "Sfb: " << performance[0] << '\n' <<
            "Lsb: " << performance[1] << '\n' <<
            "Inward roll: " << performance[2] << ' ' <<
            "Outward roll: " << performance[3] << '\n' <<
            "I/O Difference: " << performance[4] << '\n' <<
            "Total roll: " << performance[5] << '\n' <<
            "Effort: " << performance[6] << endl;
    }
}

void gen6_stats_and_write(vector<double>& thresholds) {
    vector<double> sfb_data, lsb_data, io_diff_data, rolls_data, effort_data;
    int data_len = 0;
    ofstream stats_file, kb_file;
    stats_file.open("gen6_stats.txt");
    kb_file.open("gen6.txt");

    string kb_text = "__o__rneit__u____-__xla____-__";
    vector<int> h_loc = { 3, 13 };
    vector<int> p_loc = { 18, 23, 28 };
    vector<int> w_loc = { 19, 24, 29 };
    vector<int> b_loc = { 3, 13, 18, 23, 28 };
    string qy_str = "qy";
    vector<int> qy_loc = { 0, 10 };
    string ds_str = "ds";
    vector<int> ds_loc = { 1, 11 };
    string cgm_str = "cgm";
    vector<int> cgm_loc = { 4, 14, 19, 24, 29 };
    string jkz_str = "jkz--";
    int jkz_loc;

    string remaining_keys = "fgv";

    for (int _h = 0; _h < 2; _h++) {
        string filled_kb_text_h = kb_text;
        filled_kb_text_h[h_loc[_h]] = 'h';
        for (int _p = 0; _p < 3; _p++) {
        string filled_kb_text_p = filled_kb_text_h;
        filled_kb_text_p[p_loc[_p]] = 'p';
        for (int _w = 0; _w < 3; _w++) {
            string filled_kb_text_w = filled_kb_text_p;
            filled_kb_text_w[w_loc[_w]] = 'w';
            for (int _b = 0; _b < 5; _b++) {
            string filled_kb_text_b = filled_kb_text_w;
            if (filled_kb_text_b[b_loc[_b]] != '_') { continue; }
            filled_kb_text_b[b_loc[_b]] = 'b';
            do {
                string filled_kb_text_qy = filled_kb_text_b;
                filled_kb_text_qy[qy_loc[0]] = qy_str[0];
                filled_kb_text_qy[qy_loc[1]] = qy_str[1];
                do {
                string filled_kb_text_ds = filled_kb_text_qy;
                filled_kb_text_ds[ds_loc[0]] = ds_str[0];
                filled_kb_text_ds[ds_loc[1]] = ds_str[1];
                do {
                    string filled_kb_text_cgm = filled_kb_text_ds;
                    bool _cgm_ok = true;
                    for (int _cgm = 0; _cgm < 3; _cgm++) {
                        if (filled_kb_text_cgm[cgm_loc[_cgm]] != '_') {_cgm_ok = false; break;}
                    }
                    if (!_cgm_ok) { continue; }
                    filled_kb_text_cgm[cgm_loc[0]] = cgm_str[0];
                    filled_kb_text_cgm[cgm_loc[1]] = cgm_str[1];
                    filled_kb_text_cgm[cgm_loc[2]] = cgm_str[2];

                    // rand jkz between 15 and 29
                    for (int j = 0; j < jkz_str.length(); j++) {
                        do { jkz_loc = rand() % 15 + 15; } while (filled_kb_text_cgm[jkz_loc] != '_');
                        filled_kb_text_cgm[jkz_loc] = jkz_str[j];
                    }
                    // Substitute remaining keys
                    random_shuffle(remaining_keys.begin(), remaining_keys.end());
                    int remaining_counter = 0;
                    for (int j = 0; j < 30; j++) {
                        if (filled_kb_text_cgm[j] == '_') {
                            filled_kb_text_cgm[j] = remaining_keys[remaining_counter];
                            remaining_counter++;
                        }
                    }
                    vector<double> performance = get_performance(filled_kb_text_cgm);

                    sfb_data.push_back(performance[0]);
                    lsb_data.push_back(performance[1]);
                    // performance [2], [3] are inroll and outroll
                    io_diff_data.push_back(performance[4]);
                    rolls_data.push_back(performance[5]);
                    effort_data.push_back(performance[6]);
                    data_len++;

                    if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
                        thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
                        thresholds[4] < performance[6]) {
                        continue;
                    }
                    kb_file << "Keyboard layout: " << filled_kb_text_cgm << '\n' <<
                        "Sfb: " << performance[0] << '\n' <<
                        "Lsb: " << performance[1] << '\n' <<
                        "Inward roll: " << performance[2] << ' ' <<
                        "Outward roll: " << performance[3] << '\n' <<
                        "I/O Difference: " << performance[4] << '\n' <<
                        "Total roll: " << performance[5] << '\n' <<
                        "Effort: " << performance[6] << endl;

                } while (next_permutation(cgm_loc.begin(), cgm_loc.end()));
                } while (next_permutation(ds_loc.begin(), ds_loc.end()));
            } while (next_permutation(qy_loc.begin(), qy_loc.end()));
            }
        }
        }
    }

    std::sort(sfb_data.begin(), sfb_data.end());
    std::sort(lsb_data.begin(), lsb_data.end());
    std::sort(io_diff_data.begin(), io_diff_data.end());
    std::sort(rolls_data.begin(), rolls_data.end());
    std::sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, data_len, sfb_text);
    print_percentile(stats_file, lsb_data, data_len, lsb_text);
    print_percentile(stats_file, io_diff_data, data_len, iodiff_text);
    print_percentile(stats_file, rolls_data, data_len, rolls_text);
    print_percentile(stats_file, effort_data, data_len, effort_text);
}

void gen7_stats_and_write(vector<double>& thresholds) {
    vector<double> sfb_data, lsb_data, io_diff_data, rolls_data, effort_data;
    int data_len = 0;
    ofstream stats_file, kb_file;
    stats_file.open("gen7_stats.txt");
    kb_file.open("gen7.txt");

    string kb_text = "q_o__rneity_u__-_-b_xlap___-kg";
    vector<int> h_loc = { 3, 13 };
    vector<int> w_loc = { 19, 24 };
    vector<int> f_loc = { 3, 4, 13, 14 };
    string ds_str = "ds";
    vector<int> ds_loc = { 1, 11 };
    string cm_str = "cm";
    vector<int> cm_loc = { 4, 14, 19, 24, 29 };

    string remaining_keys = "jzv-";

    for (int _h = 0; _h < 2; _h++) {
        string filled_kb_text_h = kb_text;
        filled_kb_text_h[h_loc[_h]] = 'h';
        for (int _w = 0; _w < 2; _w++) {
            string filled_kb_text_w = filled_kb_text_h;
            filled_kb_text_w[w_loc[_w]] = 'w';
            for (int _f = 0; _f < 4; _f++) {
                string filled_kb_text_f = filled_kb_text_w;
                if (filled_kb_text_f[f_loc[_f]] != '_') { continue; }
                filled_kb_text_f[f_loc[_f]] = 'f';
                do {
                    string filled_kb_text_ds = filled_kb_text_f;
                    filled_kb_text_ds[ds_loc[0]] = ds_str[0];
                    filled_kb_text_ds[ds_loc[1]] = ds_str[1];
                    do {
                        string filled_kb_text_cm = filled_kb_text_ds;
                        bool _cm_ok = true;
                        for (int _cm = 0; _cm < 2; _cm++) {
                            if (filled_kb_text_cm[cm_loc[_cm]] != '_') { _cm_ok = false; break; }
                        }
                        if (!_cm_ok) { continue; }
                        filled_kb_text_cm[cm_loc[0]] = cm_str[0];
                        filled_kb_text_cm[cm_loc[1]] = cm_str[1];
                        do {
                            string filled_kb_text_remaining = filled_kb_text_cm;
                            int remaining_counter = 0;
                            for (int j = 0; j < 30; j++) {
                                if (filled_kb_text_remaining[j] == '_') {
                                    filled_kb_text_remaining[j] = remaining_keys[remaining_counter];
                                    remaining_counter++;
                                }
                            }

                            vector<double> performance = get_performance(filled_kb_text_remaining);

                            sfb_data.push_back(performance[0]);
                            lsb_data.push_back(performance[1]);
                            // performance [2], [3] are inroll and outroll
                            io_diff_data.push_back(performance[4]);
                            rolls_data.push_back(performance[5]);
                            effort_data.push_back(performance[6]);
                            data_len++;

                            if (thresholds[0] < performance[0] || thresholds[1] < performance[1] ||
                                thresholds[2] > performance[4] || thresholds[3] > performance[5] ||
                                thresholds[4] < performance[6]) {
                                continue;
                            }
                            kb_file << "Keyboard layout: " << filled_kb_text_remaining << '\n' <<
                                "Sfb: " << performance[0] << '\n' <<
                                "Lsb: " << performance[1] << '\n' <<
                                "Inward roll: " << performance[2] << ' ' <<
                                "Outward roll: " << performance[3] << '\n' <<
                                "I/O Difference: " << performance[4] << '\n' <<
                                "Total roll: " << performance[5] << '\n' <<
                                "Effort: " << performance[6] << endl;
                        } while (next_permutation(remaining_keys.begin(), remaining_keys.end()));
                    } while (next_permutation(cm_loc.begin(), cm_loc.end()));
                } while (next_permutation(ds_loc.begin(), ds_loc.end()));
            }
        }
    }

    std::sort(sfb_data.begin(), sfb_data.end());
    std::sort(lsb_data.begin(), lsb_data.end());
    std::sort(io_diff_data.begin(), io_diff_data.end());
    std::sort(rolls_data.begin(), rolls_data.end());
    std::sort(effort_data.begin(), effort_data.end());

    string sfb_text = "Sfb";
    string lsb_text = "Lsb";
    string iodiff_text = "Inroll/Outroll difference";
    string rolls_text = "Total rolls";
    string effort_text = "Effort";

    print_percentile(stats_file, sfb_data, data_len, sfb_text);
    print_percentile(stats_file, lsb_data, data_len, lsb_text);
    print_percentile(stats_file, io_diff_data, data_len, iodiff_text);
    print_percentile(stats_file, rolls_data, data_len, rolls_text);
    print_percentile(stats_file, effort_data, data_len, effort_text);
}


int main() {
    typedef chrono::high_resolution_clock Time;
    typedef chrono::milliseconds ms;
    typedef chrono::duration<float> fsec;
    auto t0 = Time::now();

    int stats_iterations = 1000;
    int run_iterations = 1'000'000;

    // --- Frogpad Control test --- //
    // frogpad_control();

    // --- Generation 1 --- //
    vector<double> thresholds1 = { 21.3519, 24.2245, 2.94915, 14.968, 86.4051 };
    // gen1_stats(stats_iterations);
    // gen1_write(run_iterations, thresholds1);

    // --- Generation 2 --- //
    vector<double> thresholds2 = { 19.1193, 13.9626, 5.46179, 17.4018, 68.6403 };
    // gen2_stats(stats_iterations);
    // gen2_write(run_iterations, thresholds2);

    // --- Generation 3 --- //
    vector<double> thresholds3 = { 14.3443, 9.8855, 9.9382, 23.0943, 64.429 };
    // gen3_stats(stats_iterations);
    // gen3_write(run_iterations, thresholds3);

    // --- Generation 4 --- //
    vector<double> thresholds4 = { 13.0735, 9.5251, 11.9855, 24.8525, 59.8484 };
    // gen4_stats(stats_iterations);
    // gen4_write(run_iterations, thresholds4);

    // --- Generation 5 --- //
    vector<double> thresholds5 = { 12.2616, 9.8230, 13.4736, 26.6954, 60.3695 };  // Best 15%
    // gen5_stats(stats_iterations);
    // gen5_write(run_iterations, thresholds5);

    // --- Generation 6 --- //
    vector<double> thresholds6 = { 12.0108, 10.1053, 13.9112, 26.8418, 60.1379 };  // Best 25%
    // gen6_stats_and_write(thresholds6);

    // --- Generation 7 --- //
    vector<double> thresholds7 = { 12.0064, 8.83365, 13.9789, 26.8823, 60.0466 };  // Best 50%
    gen7_stats_and_write(thresholds7);

    auto t1 = Time::now();
    fsec fs = t1 - t0;
    ms d = chrono::duration_cast<ms>(fs);
    cout << "Time elapsed: " << d.count() << "ms\n";
    cin.get();
    return 0;
}