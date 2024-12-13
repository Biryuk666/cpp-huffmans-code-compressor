#include <iostream>
#include <map>
#include <string>
#include <vector>

// Serialization

template <typename T>
void Serialize(T val, std::ostream& out) {
    out.write(reinterpret_cast<const char*>(&val), sizeof(val));    
    if (out.fail()) {
        throw std::runtime_error("Error writing data to stream");
    }
}

void Serialize(const std::string& str, std::ostream& out) {
    Serialize(str.size(), out);
    out.write(reinterpret_cast<const char*>(str.data()), str.size());
    if (out.fail()) {
        throw std::runtime_error("Error writing string data to stream");
    }
}

template <typename T>
void Serialize(const std::vector<T>& val, std::ostream& out) {
    Serialize(val.size(), out);
    out.write(reinterpret_cast<const char*>(val.data()), val.size());
    if (out.fail()) {
        throw std::runtime_error("Error writing vector data to stream");
    }
}

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& val, std::ostream& out) {
    Serialize(val.size(), out);
    for (const auto& [key, value] : val) {
        Serialize(key, out);
        Serialize(value, out);
    }
    if (out.fail()) {
        throw std::runtime_error("Error writing map data to stream");
    }
}

// Deserialization

template <typename T>
void Deserialize(std::istream& in, T& val) {
    in.read(reinterpret_cast<char*>(&val), sizeof(val));
    if (in.fail()) {
        throw std::runtime_error("Error reading data from stream");
    }
}

void Deserialize(std::istream& in, std::string& str) {
    size_t size;
    Deserialize(in, size);
    str.resize(size);
    in.read(reinterpret_cast<char*>(str.data()), str.size());
    if (in.fail()) {
        throw std::runtime_error("Error reading string data from stream");
    }
}

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& val) {
    size_t size;
    Deserialize(in, size);
    val.resize(size);
    in.read(reinterpret_cast<char*>(val.data()), val.size());
    if (in.fail()) {
        throw std::runtime_error("Error reading vector data from stream");
    }
}

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& val) {
    size_t size;
    Deserialize(in, size);
    while (size > 0) {
        T1 key;
        Deserialize(in, key);
        T2 value;        
        Deserialize(in, value);
        val.emplace(key, value);
        if (in.fail()) {
            throw std::runtime_error("Error reading map data from stream");
        }
        --size;
    }
}