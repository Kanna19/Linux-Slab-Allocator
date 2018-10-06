#include <iostream>
#include <thread>
#include <mutex>
#include "libmymem.cpp"
#include <cstring>

using namespace std;

int n;
int t;

// This function converts String to Integer and terminates if an invalid character is detected
int conv(char* str)
{
    int x = 0;
    int len = strlen(str);

    for(int i = 0; i < len; i++)
    {
        if('0' <= str[i] && str[i] <= '9')
        {
            x = (x *10) +(str[i] -'0');
        }

        else
        {
            cerr << "Invalid character detected" << endl;
            exit(EXIT_FAILURE);
        }
    }

    return x;
}

void threadFunction()
{
    for(int i = 0; i < n; i++)
    {
        // Generate a random integer in the range [1, 8192]
        int size = (rand() % 8192) +1;
        int* p = (int*)mymalloc(size);
        *p = (rand() % 1000);
        usleep(100);
        myfree(p);
    }
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        cerr << "Please use flag -n followed by an integer in [1, 10000]" << endl;
        exit(EXIT_FAILURE);
    }

    // Getting information regarding number of iterations, threads from the arguments to main
    n = t = -1;
    for(int i = 1; i < argc; i++)
    {
        if(i +1 != argc)
        {
            if(strcmp(argv[i], "-n") == 0)
            {
                n = conv(argv[i +1]);
            }

            else if(strcmp(argv[i], "-t") == 0)
            {
                t = conv(argv[i +1]);
            }
        }
    }

    if(n == -1 || n < 1 || n > 10000)
    {
        cerr << "Please use flag -n followed by an integer in [1, 10000]" << endl;
        exit(EXIT_FAILURE);
    }

    if(t != -1)
    {
        if(t < 1 || t > 64)
        {
            cerr << "Please use threads in the range [1, 64]" << endl;
            exit(EXIT_FAILURE);
        }
    }

    else
    {
        t = 1;
    }

    thread thr[t];

    for(int i = 0; i < t; i++)
    {
        thr[i] = thread(threadFunction);
    }

    for(int i = 0; i < t; i++)
    {
        thr[i].join();
    }

    return 0;
}
