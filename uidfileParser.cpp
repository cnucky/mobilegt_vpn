#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

string getNameByUid(int uid,string fileName);

multimap<int,string> fileCacheMap;
int main()
{
    int uid;
    string fileName="/home/ubuntu/vpnserver/test.app";
    //string fileName="/home/lab/vpnserver/test.app";
    while(true)
    {
    cout << "please uid: " << endl;
    cin >> uid;
    if(uid<=0)
        break;
    string name=getNameByUid(uid,fileName);
    cout << "uid is: " << uid << "\t";
    cout << "name is: " << name << endl;
    }
}

//字符串分割函数
std::vector<std::string> split(std::string str,std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str+=pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}

template <typename Type>
Type stringToNum(const string & str)
{
    istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

void fileCacheInit(string fileName)
{
    int uidhead;
    fileCacheMap.clear();
    //string appName,version,processName;
    string name;
    string line;
    ifstream infile(fileName.c_str(),ios::in);
    //inflie.getline();
    while(getline(infile,line))
    {
        if(line.length()<=0)
            break;
        vector<string> result=split(line,"\t");
        uidhead=stringToNum<int>(result[0]);
        //name=result[1]+"\t"+result[2]+"\t"+result[3];
        name=result[1];
        fileCacheMap.insert(make_pair(uidhead,name));
        //cout << "insert " << uidhead << "-->" << name << endl;
    }
    infile.close();
}
string getNameByUid(int uid,string fileName)
{
    if(fileCacheMap.empty())
        fileCacheInit(fileName);
    //cout << fileCacheMap.size() << endl;
    //cout << "find " << uid << endl;
    multimap<int,string>::iterator it;
    int count = fileCacheMap.count(uid);
    it=fileCacheMap.find(uid);
    //if(it!=fileCacheMap.end())
    string name;
    if(count>0) {
        for(int i=0;i<count;i++,it++)
            name+=it->second+"|";
    } else {
        ostringstream convert;
        convert << uid;
        name +="NOTFOUND["+convert.str()+"]";
    }
    return name;
} 


