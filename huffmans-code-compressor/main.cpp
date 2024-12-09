#include "Huffmans_code_compressor.h"

#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
using filesystem::path;

int main() {
    cout << "Enter the path for the input file to compress:"s << endl;
    string input_file;
    getline(cin, input_file);
    path path = input_file;
    string output_file = path.parent_path().string() + "\\output.hcc";
    HuffmansCodeCompressor compressor(input_file, output_file);
    compressor.CompressDocument();

    input_file = output_file;
    path = input_file;
    output_file = path.parent_path().string() + "\\output.txt";
    HuffmansCodeCompressor decompressor(input_file, output_file);
    decompressor.DecompressDocument();
}