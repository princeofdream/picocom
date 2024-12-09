#include "InterCommunication_serviceimpl.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

using namespace v1::ara_api::InterCommunication;
int fdRead = 0;

void dump_hex_strip(const uint8_t* data, size_t size, size_t display,const char *split)
{
    size_t loop;
    size_t display_sz;
    char value[1024];

    display_sz = display;
    if (display == 0 || display >= size) {
        display_sz = size;
    }

    loop = 0;
    memset(value, 0x0, sizeof(value));
    while (loop < size) {
        if (loop == 0) {
            sprintf(value,"%s","\n");
            sprintf(value, "%s%s",value, "----------------------------------------------");
            // sprintf(value, "%s%s", value, "\n");
            printf("%s\n", value);
            memset(value, 0x0, sizeof(value));
            sprintf(value, "%s %02x0 | ", split, loop / 16);
        }

        sprintf(value, "%s %02x", value, data[loop]);

        if (loop % 16 == 15) {
            // sprintf(value, "%s%s", value, "\n");
            if (loop/16 <= display_sz/16 || loop/16+2 >= size/16) {
                printf("%s\n", value);
            }
            if (loop/16-1 == display_sz/16 && loop/16+2 != size/16) {
                printf("...\n");
            }
            memset(value, 0x0, sizeof(value));
            if (value[0] == 0x0) {
                sprintf(value, "%s %02x0 | ", split, loop / 16 + 1);
            } else {
                sprintf(value, "%s%s %02x0 | ", value, split, loop / 16 + 1);
            }
        } else if (loop % 8 == 7) {
            sprintf(value, "%s%s", value, "  ");
        }
        loop++;
    }
    // sprintf(value, "%s%s", value, "\n");
    printf("%s\n", value);
}

void dump_hex(const uint8_t* data, size_t size, const char *split)
{
    dump_hex_strip(data, size, size, split);
}

void dump_hex_vec_strip(std::vector<uint8_t> vec_data, size_t display, const char *split)
{
    uint8_t* data = vec_data.data();
    size_t len = vec_data.size();

    dump_hex_strip(data, len, display, split);
}

void dump_hex_vec(std::vector<uint8_t> vec_data, const char *split)
{
    uint8_t* data = vec_data.data();
    size_t len = vec_data.size();

    dump_hex_strip(data, len, len, split);
}

void InterCommunication_serviceimpl::InterRequest(Array_InterComm& Request)
{
    string str;
    std::vector<Uint8_Rawdata> vec = Request.getValue();
#if 1
    str.assign(vec.begin(),vec.end());
    soaSlogI("[ srv ][ Method   ] Recv %s: [ %s ]", __func__, str.c_str());
#else
    dump_hex_data(vec.data(), vec.size(), "REQ" ,32);
#endif
}

ara::core::Future<Array_InterComm> InterCommunication_serviceimpl::InterRequestRR(Array_InterComm& RequestRR)
{
    ara::core::Promise<Array_InterComm> _promise;
    ara::core::Future<Array_InterComm> _future = _promise.get_future();

    string str;
    std::vector<Uint8_Rawdata> vec = RequestRR.getValue();
    Array_InterComm arr_ret;
    string str_ret = "RR_Okay";
    std::vector<Uint8_Rawdata> vec_ret;

#if 0
    str.assign(vec.begin(),vec.end());
    soaSlogI("[ srv ][ Method   ] Recv %s: [ %s ]", __func__, str.c_str());
#else
    dump_hex_vec_strip(vec, 32, "get");
#endif

#if 1
    if (fdRead > 0) {
        int rlen;
        uint8_t buf[2];
        lseek(fdRead, 0, SEEK_SET);
        while (1) {
            memset(buf, 0x0, sizeof(buf));
            rlen = read(fdRead, &buf, sizeof(buf[0]));
            if (rlen <= 0) {
                break;
            }
            vec_ret.push_back(buf[0]);
        }
    }
    dump_hex_vec_strip(vec_ret, 128, "send");
#else
    vec_ret.assign(str_ret.begin(),str_ret.end());
#endif

    arr_ret.setValue(vec_ret);
    _promise.set_value(arr_ret);

    // to do something...
    return _future;
}

