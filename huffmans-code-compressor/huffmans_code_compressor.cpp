#include "Huffmans_code_compressor.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <queue>
#include <sstream>
#include <stdexcept>

using namespace std;
using filesystem::path;

Node* CreateNode(char ch, int freq, Node* left, Node* right) {
    Node* node = new Node{ch, freq, left, right};

    return node;
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

Node* BinaryTree::GetRoot() {
    return root_;
}

void BinaryTree::SetRoot(Node* root) {
    root_ = root;
}

BinaryTree::~BinaryTree() {
    DeleteNode(root_);
}

void BinaryTree::DeleteNode(Node* root) {
    if (root->left) {
        DeleteNode(root-> left);
    }
    if (root->right) {
        DeleteNode(root->right);
    }

    delete root;
}

Node* InsertLeftNode(Node* root, char ch) {
    root->left = CreateNode(ch, 0, nullptr, nullptr);
    return root->left;
}

Node* InsertRightNode(Node* root, char ch) {
    root->right = CreateNode(ch, 0, nullptr, nullptr);
    return root->right;
}


bool NodeComp::operator()(Node* lhs, Node* rhs) {
    return lhs->freq > rhs->freq;
}

HuffmansCodeCompressor::HuffmansCodeCompressor(path input_file_path, path output_file_path)
    : input_file_path_(input_file_path), output_file_path_(output_file_path) {}

void HuffmansCodeCompressor::SetIputFilePath(path input_file_path){
    input_file_path_ = input_file_path;
}

void HuffmansCodeCompressor::SetOutputFilePath(path output_file_path) {
    output_file_path_ = output_file_path;
}

void HuffmansCodeCompressor::Encode(Node* root, string str) {
    if (!root) return;

    if (!root->left && !root->right) {
        char_to_code_[root->ch] = str;
    }

    Encode(root->left, str + '0');
    Encode(root->right, str + '1');
}

void HuffmansCodeCompressor::Decode(Node* root, int& index, const string& str, ostream& output) {
    if (!root) return;
    if (!root->left && !root->right) {
        output << root->ch;
        return;
    }
    int current_index = index++;
    char ch = str[current_index];
    if (ch == '0') {
        Decode(root->left, index, str, output);
    } else {
        Decode(root->right, index, str, output);
    }
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

void HuffmansCodeCompressor::BuildTreeByCode() {
    for (const auto& [ch, code] : char_to_code_) {
        Node* root = tree_.GetRoot();
        for (char bit : code) {
            root = OpenOrInsertNode(root, '\0', bit);
        }
        root->ch = ch;
    }
}

vector<string> HuffmansCodeCompressor::ReadDocument(const path& doc) {
    ifstream input(doc, ios::in);
    if (!input) {
        throw ios_base::failure("Failed to open document "s + doc.filename().string());
    }
    vector<string> result;
    string str;
    while (getline(input, str)) {
        result.push_back(str);
    }

    return result;
}

string HuffmansCodeCompressor::ReadCompressedDocument(const path& doc) {
    string settings;
    ifstream input(doc, ios::binary);
    if (!input) {
        throw ios_base::failure("Failed to open document "s + doc.filename().string());
    }

    getline(input, settings);
    if (settings.empty()) {
        cout << "The compressor settings could not be read. Select the file compressed by HuffmansCodeCompressor"s << endl;
        return {};
    }
    SetCompressor(settings);

    string result;
    char byte;

    while (input.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        for (int i = 7; i >= 0; --i) {
            result += ((byte >> i) & 1) ? '1' : '0';
        }
    }
    input.close();

    return result;
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
        //code_to_char_[code] = ch;
    }

    tree_.SetRoot(new Node());
    BuildTreeByCode();
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

void HuffmansCodeCompressor::ComputeCharFreq(const vector<string>& strings) {
    for (const auto& str : strings) {
        for (char ch : str) {
            ++char_to_freq_[ch];
        }
    }
}

const std::string& HuffmansCodeCompressor::GetCodeByChar(char ch) const{
    static string empty_string;
    auto it = char_to_code_.find(ch);
    if (it != char_to_code_.end()) {
        return it->second;
    }    
    return empty_string;
}

bool HuffmansCodeCompressor::CompressDocument() {
    if (input_file_path_.empty() || output_file_path_.empty()) {
        cout << "Set the input and output files paths"s << endl;
        return false;
    }
    auto text = ReadDocument(input_file_path_);
    ComputeCharFreq(text);
    BuildTree();
    Encode(tree_.GetRoot(), "");
    ofstream output (output_file_path_, ios::binary);
    if (!output) {
        cout << "Failed to create document: " + output_file_path_.filename().string() << endl;
        return false;
       
    }
    string settings = CreateCompressorSettings();
    output << settings;

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
    output.write(reinterpret_cast<char*>(binary_buffer.data()), binary_buffer.size());
    output.close();

    return true;
}

bool HuffmansCodeCompressor::DecompressDocument() {
    if (input_file_path_.empty() || output_file_path_.empty()) {
        cout << "Set the input and output files paths"s << endl;
        return false;
    }
    
    string compressed_text = ReadCompressedDocument(input_file_path_);

    ofstream output(output_file_path_, ios::out);
    if (!output) {
        cout << "Failed to create document: " + output_file_path_.filename().string() << endl;
        return false;
    }
    int index = 0;
    while (index != compressed_text.size() - 1) {
        Decode(tree_.GetRoot(), index, compressed_text, output);
    }
    output.close();

    return true;
}