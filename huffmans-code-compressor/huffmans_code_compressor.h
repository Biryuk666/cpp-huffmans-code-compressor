#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std::literals;

struct Node {
    char ch = '\0';
    int freq = 0;
    Node* left = nullptr;
    Node* right = nullptr;    
};

struct NodeComp {
    bool operator() (Node* lhs, Node* rhs);
};

class BinaryTree {
public:
    ~BinaryTree();

    Node* GetRoot();
    void SetRoot(Node* root);
    void DeleteNode(Node* root);

private:
    Node* root_ = nullptr;
};

class HuffmansCodeCompressor {
public:
    HuffmansCodeCompressor() = default;
    HuffmansCodeCompressor(std::filesystem::path input_file_path);
    HuffmansCodeCompressor(std::filesystem::path input_file_path, std::string output_file_name);    

    void SetIputFilePath(std::filesystem::path input_file_path);
    void SetOutputFileName(std::string output_file_name);
    
    bool CompressDocument();
    bool DecompressDocument();    
    
private:
    std::filesystem::path input_file_path_;
    std::string output_file_name_;
    std::map<char,unsigned int> char_to_freq_;
    std::unordered_map<char, std::string> char_to_code_;
    BinaryTree tree_;
    std::string key_ = "@HCCKeyCheck@"s;

    std::vector<std::string> ReadDocument(const std::filesystem::path& doc);    
    void ComputeCharFreq(const std::vector<std::string>& strings);
    void BuildTree();
    void Encode(Node* root, std::string str);
    std::string CreateCompressorSettings();
    const std::string& GetCodeByChar(char ch) const;

    std::string ReadCompressedDocument(const std::filesystem::path& doc);
    void SetCompressor(const std::string& settings);
    void BuildTreeByCode();
    void Decode(Node* root, int& index, const std::string& str, std::ostream& output);
};