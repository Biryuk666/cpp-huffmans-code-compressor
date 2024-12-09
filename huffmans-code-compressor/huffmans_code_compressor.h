#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct Node {
    char ch = '\0';
    int freq = 0;
    Node* left = nullptr;
    Node* right = nullptr;    
};

Node* CreateNode(char ch, int freq, Node* left, Node* right);
Node* InsertNode(Node* root, char ch, char value);
Node* InsertLeftNode(Node* root, char ch);
Node* InsertRightNode(Node* root, char ch);

struct NodeComp {
    bool operator() (Node* lhs, Node* rhs);
};



class BinaryTree {
public:
    ~BinaryTree();

    Node* GetRoot();
    void SetRoot(Node* root);    

private:
    Node* root_;

    void DeleteNode(Node* root);
};

class HuffmansCodeCompressor {
public:
    HuffmansCodeCompressor() = default;
    HuffmansCodeCompressor(std::filesystem::path input_file_path, std::filesystem::path output_file_path);
    void SetIputFilePath(std::filesystem::path input_file_path);
    void SetOutputFilePath(std::filesystem::path output_file_path);
    bool CompressDocument();
    bool DecompressDocument();
    
    
private:
    std::filesystem::path input_file_path_, output_file_path_;
    std::map<char,unsigned int> char_to_freq_;
    std::unordered_map<char, std::string> char_to_code_;
    BinaryTree tree_;

    void Encode(Node* root, std::string str);
    void Decode(Node* root, int& index, const std::string& str, std::ostream& output);

    std::vector<std::string> ReadDocument(const std::filesystem::path& doc);
    std::string ReadCompressedDocument(const std::filesystem::path& doc);
    void SetCompressor(const std::string& settings);
    std::string CreateCompressorSettings();    
    void ComputeCharFreq(const std::vector<std::string>& strings);
    void BuildTree();
    void BuildTreeByCode();
    const std::string& GetCodeByChar(char ch) const;
};