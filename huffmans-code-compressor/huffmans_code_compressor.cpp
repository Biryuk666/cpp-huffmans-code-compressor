#include "huffmans_code_compressor.h"
#include "saveload.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <queue>
#include <sstream>
#include <stdexcept>

using namespace std;
using filesystem::path;

bool NodeComp::operator()(Node* lhs, Node* rhs) {
    return lhs->freq > rhs->freq;
}

BinaryTree::~BinaryTree() {
    DeleteNode(root_);
}

Node* BinaryTree::GetRoot() {
    return root_;
}

Node*& BinaryTree::GetRootRef() {
    return root_;
}

void BinaryTree::SetRoot(Node* root) {
    root_ = root;
}

void BinaryTree::DeleteNode(Node*& root) {
    if (root->left) {
        DeleteNode(root-> left);
    }
    if (root->right) {
        DeleteNode(root->right);
    }

    delete root;
    root = nullptr;
}

HuffmansCodeCompressor::HuffmansCodeCompressor(path input_file_path)
    : input_file_path_(input_file_path), output_file_name_("output"s) {}

HuffmansCodeCompressor::HuffmansCodeCompressor(path input_file_path, string output_file_name) 
    : input_file_path_(input_file_path), output_file_name_(output_file_name) {}


void HuffmansCodeCompressor::SetIputFilePath(path input_file_path){
    input_file_path_ = input_file_path;
    if (tree_.GetRoot()) {
        tree_.DeleteNode(tree_.GetRootRef());
    }
}

void HuffmansCodeCompressor::SetOutputFileName(string output_file_name) {
    output_file_name_ = output_file_name;
}

vector<string> HuffmansCodeCompressor::ReadDocument(const path& doc) {
    if (doc.extension().string() != ".txt"sv) {
        cout << "Invalid file type. Select the file with the '.txt' extension"sv << endl;
        return {};
    }
    ifstream input(doc, ios::in);
    if (!input) {
        throw ios_base::failure("Failed to open document "s + doc.filename().string());
    }
    vector<string> result;
    string str;
    while (getline(input, str)) {
        result.push_back(str + '\n');
    }
    result.back().back() = '/0';

    return result;
}

void HuffmansCodeCompressor::ComputeCharFreq(const vector<string>& strings) {
    for (const auto& str : strings) {
        for (char ch : str) {
            ++char_to_freq_[ch];
        }
    }
}

Node* CreateNode(char ch, int freq, Node* left, Node* right) {
    Node* node = new Node{ch, freq, left, right};

    return node;
}

void HuffmansCodeCompressor::BuildTree() {    
    priority_queue<Node*, vector<Node*>, NodeComp> nodes_queue; 
    for (const auto& [ch, freq] : char_to_freq_) {
        nodes_queue.push(CreateNode(ch, freq, nullptr, nullptr));            
    }
    
    while (nodes_queue.size() != 1) {
        Node* left = nodes_queue.top();
        nodes_queue.pop();
        Node* right = nodes_queue.top();
        nodes_queue.pop();

        int freq_sum = left->freq + right->freq;
        nodes_queue.push(CreateNode('\0', freq_sum, left, right));
    }
    tree_.SetRoot(nodes_queue.top());
}

void HuffmansCodeCompressor::Encode(Node* root, string str) {
    if (!root) return;

    if (!root->left && !root->right) {
        char_to_code_[root->ch] = str;
    }

    Encode(root->left, str + '0');
    Encode(root->right, str + '1');
}

string HuffmansCodeCompressor::CreateCompressorSettings() {
    string result;
    bool is_first = true;

    for (const auto& [ch, code] : char_to_code_) {
        if (!is_first) {
            result += ' ';
        }

        switch (ch) {
            case ('\n') :
                result += "\\n"s + code;
                break;
            case ('\r') :
                result += "\\r"s + code;
                break;   
            case ('\t') :
                result += "\\t"s + code;
                break;   
            case ('\\\\') :
                result += "\\\\"s + code;
                break;
            default :
                result += ch + code;
                break;
        }

        is_first = false;
    }

    result += '\n';
    return result;
}

const std::string& HuffmansCodeCompressor::GetCodeByChar(char ch) const{
    static string empty_string;
    auto it = char_to_code_.find(ch);
    if (it != char_to_code_.end()) {
        return it->second;
    }    
    return empty_string;
}

path SetOutputFilePath(string output_file_name, string extention, path input_file_path) {
    path result = input_file_path.parent_path().string() + "\\"s + output_file_name + extention;
    return result;
}

bool HuffmansCodeCompressor::CompressDocument() {
    if (input_file_path_.empty()) {
        cout << "Set the input file path"s << endl;
        return false;
    }
    auto text = ReadDocument(input_file_path_);
    if (text.empty()) {
        cout << "Failed to compress file: " + input_file_path_.filename().string() << endl;
        return false;
    }
    ComputeCharFreq(text);
    BuildTree();
    Encode(tree_.GetRoot(), "");
    path output_file_path = SetOutputFilePath(output_file_name_, ".hcc"s, input_file_path_);
    ofstream output (output_file_path, ios::binary);
    if (!output) {
        cout << "Failed to create document: " + output_file_path.filename().string() << endl;
        return false;
       
    }

    Serialize(key_, output);
    string settings = CreateCompressorSettings();
    Serialize(settings, output);

    vector<uint8_t> binary_buffer;
    uint8_t current_byte = 0;
    int bit_count = 0;
    for (const auto& str : text) {
        for (char ch : str) {
            string code = GetCodeByChar(ch);
            for (char bit : code) {      
                current_byte = (current_byte << 1) | (bit - '0');
                ++bit_count;
                if (bit_count == 8) {
                    binary_buffer.push_back(current_byte);
                    current_byte = 0;
                    bit_count = 0;
                }
            }
        }
    }
    if (bit_count > 0) {
        current_byte <<= (8 - bit_count);
        binary_buffer.push_back(current_byte);
    }
    Serialize(binary_buffer, output);
    output.close();

    return true;
}

string HuffmansCodeCompressor::ReadCompressedDocument(const path& doc) {
    if (doc.extension().string() != ".hcc"sv) {
        cout << "Invalid file type. Select the file with the '.hcc' extension"sv << endl;
        return {};
    }
    ifstream input(doc, ios::binary);
    if (!input) {
        throw ios_base::failure("Failed to open document "s + doc.filename().string());
    }
    string key;
    Deserialize(input, key);
    if (key != key_) {
        cout << "The key could not be read from the "s + input_file_path_.filename().string() + " document. Select the file compressed by Huffman's Code Compressor"s << endl;
        return {};
    }
    string settings;
    Deserialize(input, settings);
    if (settings.empty()) {
        cout << "The compressor settings could not be read. Select the file compressed by Huffman's Code Compressor"sv << endl;
        return {};
    }
    SetCompressor(settings);

    string compressed_text;
    Deserialize(input, compressed_text);
    input.close();

    string result;
    for (char byte : compressed_text) {
        for (int i = 7; i >= 0; --i) {
            result += ((byte >> i) & 1) ? '1' : '0';
        }
    }

    return result;
}

Node* InsertLeftNode(Node* root, char ch) {
    root->left = CreateNode(ch, 0, nullptr, nullptr);
    return root->left;
}

Node* InsertRightNode(Node* root, char ch) {
    root->right = CreateNode(ch, 0, nullptr, nullptr);
    return root->right;
}

Node* OpenOrInsertNode(Node* root, char ch, char value) {
    if (value == '0') {
        if (!root->left) {
            return InsertLeftNode(root, ch);
        }
        return root->left;
    }
    if (!root->right) {
        return InsertRightNode(root, ch);
    }
    return root->right;
}

void HuffmansCodeCompressor::BuildTreeByCode() {
    tree_.SetRoot(new Node());
    for (const auto& [ch, code] : char_to_code_) {
        Node* root = tree_.GetRoot();
        for (char bit : code) {
            root = OpenOrInsertNode(root, '\0', bit);
        }
        root->ch = ch;
    }
}

void HuffmansCodeCompressor::SetCompressor(const string& settings) {
    istringstream input(settings);

    while (input) {
        string str;
        input >> str;
        if (str.empty()) continue;

        char ch = str[0];
        
        if (ch == '\\') {

            switch(str[1]) {
                case ('n') :
                    ch = '\n';
                    str = str.substr(1);
                    break;
                case ('r') :
                    ch = '\r';
                    str = str.substr(1);
                    break;
                case ('t') :
                    ch = '\t';
                    str = str.substr(1);
                    break;
                case ('\\') :
                    ch = '\\\\';
                    str = str.substr(1);
                    break;
                default :
                    break;
            }
        } else if (ch == '0' || ch == '1') {
            ch = ' ';
            char_to_code_[ch] = str;
            continue;
        }
        
        string code = str.substr(1);

        char_to_code_[ch] = code;
    }

    BuildTreeByCode();
}

void HuffmansCodeCompressor::Decode(Node* root, int& index, const string& str, ostream& output) {
    if (!root) return;
    if (!root->left && !root->right) {
        output << root->ch;
        return;
    }
    char ch = str[index++];
    if (ch == '0') {
        Decode(root->left, index, str, output);
    } else {
        Decode(root->right, index, str, output);
    }
}

bool HuffmansCodeCompressor::DecompressDocument() {
    if (input_file_path_.empty()) {
        cout << "Set the input file path"s << endl;
        return false;
    }
    
    string compressed_text = ReadCompressedDocument(input_file_path_);
    path output_file_path = SetOutputFilePath(output_file_name_, ".txt"s, input_file_path_);

    ofstream output(output_file_path, ios::out);
    if (!output) {
        cout << "Failed to create document: " + output_file_path.filename().string() << endl;
        return false;
    }
    int index = 0;
    while (index < compressed_text.size()) {
        Decode(tree_.GetRoot(), index, compressed_text, output);
    }
    output.close();

    return true;
}