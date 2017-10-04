
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int startCapture(const char * cmdName,const char * argv[]);

int main() 
{
    const char * cmdName="/home/ubuntu/vpnserver/checkCapture.sh";
    const char * argv[]={"checkCapture.sh","tun0","4cd4ccc"};
    startCapture(cmdName,argv);
    return 0;
}

int startCapture(const char * cmdName,const char * argv[])
{
    pid_t pid;
    if((pid=fork())<0) {
        cout << "fork error" << endl;
        return pid;
    }
    if(pid==0) {
        cout << " this child process, pid is [" << getpid() << "]\n";
        execlp(cmdName, argv[0], argv[1], argv[2], NULL);
        exit(1);
    } else {
        cout << "father ok!\n" << "father pid is [" << getpid() << "]child pid is [" << pid << "]" << endl;
    }
    return pid;
}
