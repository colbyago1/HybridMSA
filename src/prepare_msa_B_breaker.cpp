#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

std::vector<std::string> read_fasta(const std::string& filename) {
    std::vector<std::string> sequences;
    std::ifstream file(filename);
    std::string line;
    std::string current_seq;

    while (std::getline(file, line)) {
        if (line[0] == '>') {
            if (!current_seq.empty()) {
                sequences.push_back(current_seq);
                current_seq.clear();
            }
        } else {
            current_seq += line;
        }
    }
    if (!current_seq.empty()) {
        sequences.push_back(current_seq);
    }
    return sequences;
}

std::string repeat_char(char c, int n) {
    return std::string(n, c);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <fasta_file>" << std::endl;
        return 1;
    }

    std::string fa = argv[1];
    std::vector<std::string> sequences = read_fasta(fa);

    std::string substring1 = "KIEPLGVAPTRCKRR";
    std::string substring2 = "LGFLGAAGSTMGAASMTLTVQARNLLS";

    for (int i = 0; i < sequences.size(); ++i) {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << i + 1;
        std::string i_formatted = ss.str();

        std::string seq = sequences[i];

        size_t pos1 = seq.find(substring1);
        size_t pos2 = seq.find(substring2);

        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            std::cerr << "Error: Substrings not found in sequence " << i + 1 << std::endl;
            continue;
        }

        int length1 = pos1 + substring1.length();
        int length2 = seq.length() - pos2;

        std::string seq1 = seq.substr(0, length1);
        std::string seq2 = seq.substr(pos2);

        std::string dash_string1 = repeat_char('-', length1);
        std::string dash_string2 = repeat_char('-', length2);

        std::string output_filename = fa.substr(0, fa.length() - 3) + "_seq" + i_formatted + ".a3m";
        std::ofstream output_file(output_filename);

        // Write header and query to file
        output_file << "#" << length1 << "," << length2 << "\t3,3" << std::endl;
        output_file << ">101\t102" << std::endl;
        output_file << seq << std::endl;

        // Append content of original a3m file
        std::ifstream original_a3m(fa.substr(0, fa.length() - 3) + ".a3m");
        output_file << original_a3m.rdbuf();

        // Write 101 sequence
        output_file << ">101" << std::endl;
        output_file << seq1 << dash_string2 << std::endl;

        // Append content of 101.a3m file
        std::ifstream a3m_101(fa.substr(0, fa.length() - 3) + "_101.a3m");
        output_file << a3m_101.rdbuf();

        // Write 102 sequence
        output_file << ">102" << std::endl;
        output_file << dash_string1 << seq2 << std::endl;

        // Append content of 102.a3m file
        std::ifstream a3m_102(fa.substr(0, fa.length() - 3) + "_102.a3m");
        output_file << a3m_102.rdbuf();

        output_file.close();
    }

    return 0;
}
