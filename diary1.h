#pragma once
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <vector>

using namespace std;

class diary1
{
    char date[11];
    double time;
    vector<char> event;

public:
    diary1();
    //diary1(const diary1& a);
    //diary1(diary1&& a);
    //~diary1();

    void setRand();
    friend void print(const diary1 &a);
    friend int toChar(diary1 &a, vector<char> &v);
    friend void toDiary1(diary1 &a, vector<char> &v);
};
