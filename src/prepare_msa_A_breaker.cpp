#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <map>
#include <regex>
#include <cstdlib>

namespace fs = std::filesystem;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string get_sequence_from_file(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line); // Skip first line
    std::getline(file, line); // Get second line (sequence)
    return line;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    if (argc < 3) { // Minimum arguments: program name, fasta file, MSA path, and at least one substring
        std::cerr << "Usage: " << argv[0] << " <fasta_file> <msa path> <substrings>" << std::endl;
        return 1;
    }

    // store substring and get length
    std::string fa = argv[1];
    std::string seq = get_sequence_from_file(fa);
    int length = seq.length();

    // Get MSA directory path
    std::string msa_path = argv[2];

    // Collect substrings from command-line arguments
    std::vector<std::string> substrings;
    for (int i = 3; i < argc; ++i) {
        substrings.push_back(argv[i]);
    }

    // Print substrings for debugging
    std::cout << "fasta file: " << fa << std::endl;
    std::cout << "MSA directory: " << msa_path << std::endl;
    std::cout << "Substrings:" << std::endl;
    for (const auto& substring : substrings) {
        std::cout << " - " << substring << std::endl;
    }

    // for each substring get start and end postitions
    std::map<std::string, int> start_positions;
    std::vector<int> starts, ends;

    for (const auto& substring : substrings) {
        size_t pos = seq.find(substring);
        if (pos != std::string::npos) {
            starts.push_back(pos);
            start_positions[substring] = pos;
            ends.push_back(pos + substring.length());
        } else {
            std::cerr << "Warning: No positions found for substring '" << substring << "'" << std::endl;
        }
    }

    // sort start and end postitions
    std::sort(starts.begin(), starts.end());
    std::sort(ends.begin(), ends.end());

    // Add N- and C-terminal linkers
    std::string Nlinker_dash_string = starts[0] == 0 ? "" : std::string(starts[0], '-');
    std::string Clinker_dash_string = ends.back() == length ? "" : std::string(length - ends.back(), '-');

    // linker lengths
    std::vector<int> linker_lengths;
    for (size_t i = 1; i < starts.size(); ++i) {
        linker_lengths.push_back(starts[i] - ends[i-1]);
    }

    // linkers
    std::vector<std::string> linkers;
    for (int len : linker_lengths) {
        linkers.push_back(std::string(len, '-'));
    }

    std::vector<std::string> sorted_substrings;
    for (int pos : starts) {
        for (const auto& pair : start_positions) {
            if (pair.second == pos) {
                sorted_substrings.push_back(pair.first);
                break;
            }
        }
    }

    int breaker = 0;
    for (size_t i = 0; i < sorted_substrings.size(); ++i) {
        if (sorted_substrings[i] == "KIEPLGVAPTRCKRR") {
            breaker = i;
            break;
        }
    }

    std::vector<std::string> target_names;
    std::ifstream target_file(msa_path + "/target_names.txt");
    std::string line;
    while (std::getline(target_file, line)) {
        target_names.push_back(line);
    }

    std::vector<std::vector<std::string>> contigs;
    for (const auto& substring : sorted_substrings) {
        std::vector<std::string> temp;
        std::ifstream contig_file(msa_path + "/" + substring + ".txt");
        while (std::getline(contig_file, line)) {
            temp.push_back(line);
        }
        contigs.push_back(temp);
    }

    std::ofstream output_file(fa.substr(0, fa.length() - 3) + ".a3m");
    std::ofstream output_file_101(fa.substr(0, fa.length() - 3) + "_101.a3m");
    std::ofstream output_file_102(fa.substr(0, fa.length() - 3) + "_102.a3m");

    std::string substring1 = "KIEPLGVAPTRCKRR";
    std::string substring2 = "LGFLGAAGSTMGAASMTLTVQARNLLS";

    size_t pos1 = seq.find(substring1);
    size_t pos2 = seq.find(substring2);

    int length1 = pos1 + substring1.length();
    int length2 = seq.length() - pos2;

    std::string dash_string1(length1, '-');
    std::string dash_string2(length2, '-');

    for (size_t i = 0; i < target_names.size(); ++i) {
        std::string line = Nlinker_dash_string;
        std::string line101 = Nlinker_dash_string;
        std::string line102 = dash_string1;

        for (size_t j = 0; j < sorted_substrings.size(); ++j) {
            size_t index = i + j * target_names.size();
            if (j > breaker) {
                line102 += contigs[j][index] + linkers[j];
            } else {
                line101 += contigs[j][index] + linkers[j];
            }
            line += contigs[j][index] + linkers[j];
        }

        line += Clinker_dash_string;
        line101 += dash_string2;
        line102 += Clinker_dash_string;

        int checkSum = line.length() - std::count_if(line.begin(), line.end(), [](char c) { return std::islower(c); });

        std::string target = split(target_names[i], '\t')[0];

        if (line101.find_first_not_of('-') != std::string::npos) {
            output_file_101 << target << "\t" << target.substr(1) << std::endl;
            output_file_101 << line101 << std::endl;
        }

        if (line102.find_first_not_of('-') != std::string::npos) {
            output_file_102 << target << "\t" << target.substr(1) << std::endl;
            output_file_102 << line102 << std::endl;
        }

        output_file << target << "\t" << target.substr(1) << std::endl;
        output_file << line << std::endl;
    }

    return 0;
}
