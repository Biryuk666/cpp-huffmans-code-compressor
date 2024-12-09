#include "Huffmans_code_compressor.h"

#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
using filesystem::path;

int main() {
    cout << "Enter the path for the input file to compress:"s << endl;
    string input_file_path;
    getline(cin, input_file_path);
    path path = input_file_path;
    HuffmansCodeCompressor compressor(input_file_path);
    compressor.CompressDocument();
    string output_file = path.parent_path().string() + "\\output.hcc";
    input_file_path = output_file;
    HuffmansCodeCompressor decompressor(input_file_path);
    decompressor.DecompressDocument();
}