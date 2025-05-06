#include "msgpak.h"

MsgPak::MsgPak(int idx, int identifier, const std::string& msg)
    : index(idx), id(identifier), message(msg) {}

int MsgPak::getIndex() const {
    return index;
}

int MsgPak::getId() const {
    return id;
}

std::string MsgPak::getMessage() const {
    return message;
}

void MsgPak::setIndex(int idx) {
    index = idx;
}

void MsgPak::setId(int identifier) {
    id = identifier;
}

void MsgPak::setMessage(const std::string& msg) {
    message = msg;
}

std::string MsgPak::serialize() const {
    // Implement serialization logic here
    return std::to_string(index) + "|" + std::to_string(id) + "|" + message;
}

void MsgPak::deserialize(const std::string& data) {
    // Implement deserialization logic here
    size_t first_delim = data.find('|');
    size_t second_delim = data.find('|', first_delim + 1);

    index = std::stoi(data.substr(0, first_delim));
    id = std::stoi(data.substr(first_delim + 1, second_delim - first_delim - 1));
    message = data.substr(second_delim + 1);
}