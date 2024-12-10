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

    input_file_path = path.parent_path().string() + "\\output.hcc"s;
    HuffmansCodeCompressor decompressor(input_file_path);
    decompressor.DecompressDocument();
    
    string output_file = "myfile"s;
    input_file_path = path.parent_path().string() + "\\output.txt"s;
    compressor.SetIputFilePath(input_file_path);
    compressor.SetOutputFileName(output_file);
    compressor.CompressDocument();
}