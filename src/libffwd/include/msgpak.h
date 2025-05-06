#ifndef LIBFFWD_MSGPAK_H
#define LIBFFWD_MSGPAK_H

#include <string>

class MsgPak
{
public:
    MsgPak(int idx, int identifier, const std::string &msg);
    int getIndex() const;
    int getId() const;
    std::string getMessage() const;
    void setIndex(int idx);
    void setId(int identifier);
    void setMessage(const std::string &msg);
    std::string serialize() const;
    void deserialize(const std::string &data);

private:
    int index;
    int id;
    std::string message;
};

struct MsgPakSt
{
    int index;
    int id;
    std::string message;

    MsgPakSt(int idx, int identifier, const std::string &msg)
        : index(idx), id(identifier), message(msg) {}
};

#endif // LIBFFWD_MSGPAK_H