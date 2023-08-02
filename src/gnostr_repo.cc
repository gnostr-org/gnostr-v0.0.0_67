// C++ program to create a directory in Linux
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
int main(int argc, const char *argv[])
{
    // Create .gnostr folder
    if (mkdir(".gnostr", 0777) == -1)
        cerr << "Error :  " << strerror(errno) << endl;
    else
        cout << ".gnostr created";
}
