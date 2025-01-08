// HybridMSA

// download https://github.com/soedinglab/hh-suite/blob/master/scripts/reformat.pl

// compilation
g++ -std=c++17 src/parse_msa_bfd_only.cpp -o parse_msa_bfd_only
g++ -std=c++17 src/prepare_msa_A.cpp -o prepare_msa_A -lstdc++fs
g++ -std=c++17 src/prepare_msa_B.cpp -o prepare_msa_B

// syntax
./parse_msa_bfd_only /path/to/msas/directory contig1 [contig2] ...
./prepare_msa_A input_file.fasta /path/to/output/of/run_divide_msa_bfd_only/directory contig1 [contig2] ...
./prepare_msa_B input_file.fasta fasta_file a3m_file cardinality
