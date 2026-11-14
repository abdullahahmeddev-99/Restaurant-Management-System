#include "validation.h"
#include <cctype>

bool validName(const std::string& name) {
    if (name.empty())
        return false;

    for (char c : name) {
        if (!isalpha(static_cast<unsigned char>(c)) && c != ' ')
            return false;
    }
    return true;
}

bool validPhone(const std::string& phone) {
    if (phone.length() != 11)
        return false;

    if (phone[0] != '0' || phone[1] != '1')
        return false;

    for (char c : phone) {
        if (!isdigit(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}
