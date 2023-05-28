#include "diary1.h"

diary1::diary1():time(0)
{
    date[0] = date[3] = date[6] = date[7] = date[8] = '0';
    date[1] = date[4] = date[9] = '1';
    date[2] = date[5] = '.';
    date[10] = '\0';

    event.resize(8);
    event[0] = 'U';
    event[1] = 'n';
    event[2] = 'k';
    event[3] = 'n';
    event[4] = 'o';
    event[5] = 'w';
    event[6] = 'n';
    event[7] = 0;
}

/*
diary1::diary1(const diary1& a):
	time(a.time),
	event()
{
    //for(int i = 0; i < 11; i++)
    //    date[i] = a.date[i];

    memcpy(&date[0], &(a.date)[0], 11);	


    time = a.time;

    event = a.event;

    
    event.resize(a.event.size());

//     memcpy(event[0], a.event[0], a.event.size());
    for(size_t i = 0; i < a.event.size(); i++)
        event[i] = a.event[i];
    
}
*/

/*
diary1::diary1(diary1&& a)
{
    for(int i = 0; i < 11; i++)
        date[i] = a.date[i];
    time = a.time;
    event.resize(a.event.size());
//     memcpy(event[0], a.event[0], a.event.size());
    for(size_t i = 0; i < a.event.size(); i++)
        event[i] = a.event[i];

    diary1 b;
    for(int i = 0; i < 11; i++)
        a.date[i] = b.date[i];
    a.time = b.time;
    a.event.clear();
    a.event.resize(1);
    a.event[0] = 0;
}
*/
/*
diary1::~diary1()
{
    event.clear();
}
*/

void diary1::setRand()
{
    event.clear();
    //Random date
    int n;

    n = 1 + rand() % 31; //random day
    if (n < 10)
    {
        date[0] = '0';
        date[1] = to_string(n)[0];
    }
    else
    {
        date[0] = to_string(n)[0];
        date[1] = to_string(n)[1];
    }

    date[2] = '.';

    n = 1 + rand() % 12; //random month
    if (n < 10)
    {
        date[3] = '0';
        date[4] = to_string(n)[0];
    }
    else
    {
        date[3] = to_string(n)[0];
        date[4] = to_string(n)[1];
    }

    date[5] = '.';

    n = 1 + rand() % 2023; //random year
    if (n < 10)
    {
        date[6] = date[7] = date[8] = '0';
        date[9] = to_string(n)[0];
    } else if (n < 100)
    {
        date[6] = date[7] = '0';
        date[8] = to_string(n)[0];
        date[9] = to_string(n)[1];
    } else if (n < 1000)
    {
        date[6] = '0';
        date[7] = to_string(n)[0];
        date[8] = to_string(n)[1];
        date[9] = to_string(n)[2];
    } else
    {
        date[6] = to_string(n)[0];
        date[7] = to_string(n)[1];
        date[8] = to_string(n)[2];
        date[9] = to_string(n)[3];
    }

    date[10] = '\0';

    //Random time
    n = rand() % 24;
    time = n;
    n = rand() % 60;
    time += (double)n / 100;

    //Random event
    int lenght = 2 + rand() % 10;
    event.resize(lenght + 1);

    event[0] = 'A' + rand() % 26;
    for(int i = 1; i < lenght; i++)
        event[i] = 'a' + rand() % 26;
    event[lenght] = '\0';
}

void print(const diary1 &a)
{
    [[maybe_unused]]char* event1 = new char[a.event.size()];
    //memcpy(event1, a.event[0], a.event.size());
    for(size_t i = 0; i < a.event.size(); i++)
        event1[i] = a.event[i];
    
    if(a.time < 10)
        fprintf(stdout, "%s | 0%.2lf | %s\n", a.date, a.time, event1);
    else
        fprintf(stdout, "%s | %.2lf | %s\n", a.date, a.time, event1);
    delete[] event1;
}

int toChar(diary1 &a, vector<char> &v)
{
    size_t l = v.size();
    v.resize(l + sizeof a.date);
    memcpy(&v[0] + l, &(a.date), sizeof a.date);

    l = v.size();
    v.resize(l + sizeof a.time);
    memcpy(&v[0] + l, &(a.time), sizeof a.time);

    l = v.size();
    v.resize(l + a.event.size() + 1);
    //memcpy(&v[0] + l, a.event[0], (a.event).size());
    for(size_t i = 0; i < a.event.size(); i++)
        v[i + l] = a.event[i];
    return a.event.size();
}

void toDiary1(diary1 &a, vector<char> &v)
{
    a.event.clear();

    size_t l = 0;
//    memcpy(&(a.date), &v, 11);
    for(int i = 0; i < 11; i++)
        a.date[i] = v[i];

    l += sizeof a.date;
    memcpy(&(a.time), &v[0] + l, sizeof(a.time));

    l += sizeof(double);
    a.event.resize(v.size() - l);
//    memcpy(a.event, &v[0] + l, sizeof v - l - 1);
    for(size_t i = 0; i < v.size() - l; i++)
        a.event[i] = v[i + l];
}
