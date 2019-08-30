#pragma once

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"


using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
    static int getNumberOfCores();
    // must be static ?
    static float getSysActiveCpuTime(vector<string> values);
    
    static float getSysIdleCpuTime(vector<string> values);
    static vector<string> splitByWhiteSpace(string stringToSplit);
};

// TODO: Define all of the above functions below:
string ProcessParser::getCmd(string pid){
    string line;
    ifstream stream;
    Util::getStream((Path::basePath()+ pid + Path::cmdPath()),stream);
    getline(stream, line);
    return line;
}

vector<string> ProcessParser::getPidList(){
    DIR* dir;

    vector<string> pidList;
    if(!(dir = opendir("/proc")))
        throw runtime_error(strerror(errno));
    
    while(dirent* dirp = readdir(dir)){
        // test if it is a directory
        if(dirp->d_type != DT_DIR)
            continue;
        // test if directory's name contains only digits
        if(all_of(dirp->d_name,dirp->d_name + strlen(dirp->d_name), [](char c){return isdigit(c);})){
            pidList.push_back(dirp->d_name);
        }
    }

    //is directory closed
    if(closedir(dir))
        throw runtime_error(strerror(errno));
    
    return pidList;
}

std::string ProcessParser::getVmSize(string pid){
    string line;
    // Declaring search attribute for file
    string name = "VmData";
    string value;
    float result;
    
    // Opening stream for specific file
    ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::statusPath()), stream);
    while(getline(stream, line)){
        if(line.compare(0, name.size(), name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            result = (stof(splittedVector[1])/float(1024));
            break;
        }
    }

    return to_string(result);
}

std::string ProcessParser::getCpuPercent(string pid){

    string line;
    string value;
    float result;
    ifstream stream;
    Util::getStream((Path::basePath() + pid + "/" + Path::statPath()),stream);
    getline(stream, line);
    string str = line;
    vector<string> splittedVector = splitByWhiteSpace(line);

    // acquiring relevant times for calculation of active occupation of CPU for selected process
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(splittedVector[14]);
    float cutime = stof(splittedVector[15]);
    float cstime = stof(splittedVector[16]);
    float starttime = stof(splittedVector[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);

    result = 100.0*((total_time/freq)/seconds);

    return to_string(result);
}

long int ProcessParser::getSysUpTime(){
    string line;

    ifstream stream;
    Util::getStream((Path::basePath()+Path::upTimePath()), stream);
    getline(stream, line);
    string str = line;
    vector<string> splittedVector = splitByWhiteSpace(line);

    return stoi(splittedVector[0]);
}

std::string ProcessParser::getProcUpTime(string pid){
    string line;
    string value;
    float result;
   
    ifstream stream;
    Util::getStream((Path::basePath()+pid+"/"+Path::statPath()), stream);
    getline(stream,line);
    string str = line;
    vector<string> splittedVector = splitByWhiteSpace(line);

    return to_string(float(stof(splittedVector[13])/sysconf(_SC_CLK_TCK)));
}

string ProcessParser::getProcUser(string pid){
    string line;
    string name = "Uid:";
    string result;
   
    ifstream stream;
    Util::getStream((Path::basePath()+pid+Path::statusPath()),stream);

     while(getline(stream, line)){
        if(line.compare(0, name.size(), name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            result = splittedVector[1];
            break;
        }
    }

    ifstream stream2;
    Util::getStream("/etc/passwd",stream2);
    name = ("x:"+result);
    while(getline(stream2,line)){
        if(line.find(name) != string::npos){
            result = line.substr(0,line.find(":"));
            return result;
        }
    }

    return "";
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber){
    string line;
    string name = "cpu"+coreNumber;
    vector<string> sys_cpu_percent;

    ifstream stream;
    Util::getStream((Path::basePath()+Path::statPath()),stream);
    while(getline(stream,line)){
        if(line.compare(0, name.size(), name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            return splittedVector;
        }
    }
    
    return (vector<string>());
}

float ProcessParser::getSysRamPercent(){
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";

    float totalMem = 0;
    float freeMem = 0;
    float buffers = 0;

    ifstream stream;
    Util::getStream((Path::basePath()+Path::memInfoPath()),stream);
    while(getline(stream,line)){
        if(totalMem !=0 && freeMem!=0)
            break;
        if(line.compare(0, name1.size(), name1)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            totalMem = stof(splittedVector[1]);
        }
        if(line.compare(0, name2.size(), name2)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            freeMem = stof(splittedVector[1]);
        }
        if(line.compare(0, name3.size(), name3)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            buffers = stof(splittedVector[1]);
        }
    }

    return float(100.0*(1-(freeMem/(totalMem-buffers))));
}

string ProcessParser::getSysKernelVersion(){
    string line;
    string name = "Linux version:";
    string result;

    ifstream stream;
    Util::getStream((Path::basePath()+Path::versionPath()),stream);
    while(getline(stream, line)){
        if(line.compare(0, name.size(), name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            return splittedVector[2];
        }
    }
    return "";
}

int ProcessParser::getTotalThreads(){ 
    string name = "Threads:";
    string line;
    int total_threads = 0;
    vector<string> pidList = getPidList();

    for(string pid : pidList){
        ifstream stream;
        Util::getStream((Path::basePath()+pid+Path::statusPath()),stream);
        while(getline(stream, line)){
            if(line.compare(0,name.size(),name)==0){
                vector<string> splittedVector = splitByWhiteSpace(line);
                total_threads += stoi(splittedVector[1]);
                break;
            }
        }
    }

    return total_threads;
}

int ProcessParser::getTotalNumberOfProcesses(){
    string line;
    string name = "processes";
    int total_number_of_processes = 0;

    ifstream stream;
    Util::getStream((Path::basePath()+Path::statPath()),stream);
    while(getline(stream, line)){
        if(line.compare(0,name.size(),name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            total_number_of_processes += stoi(splittedVector[1]);
            break;
        }
    }
    
    return total_number_of_processes;
}

int ProcessParser::getNumberOfRunningProcesses(){
    int number_of_running_processes = 0;
    string line;
    string name = "procs_running";

    ifstream stream;
    Util::getStream((Path::basePath()+Path::statPath()),stream);
    while(getline(stream, line)){
        if(line.compare(0,name.size(),name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            number_of_running_processes += stoi(splittedVector[1]);

            break;
        }
    }
    return number_of_running_processes;
}

string ProcessParser::getOSName(){
    string line;
    string name = "PRETTY_NAME=";
    ifstream stream;
    Util::getStream("/etc/os-release",stream);
    while(getline(stream, line)){
        if(line.compare(0, name.size(), name)==0){
            size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(remove(result.begin(), result.end(),'"'), result.end());
            return result;
        }
    }
    return "";
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    float active_time = getSysActiveCpuTime(values2)-getSysActiveCpuTime(values1);
    float idle_time = getSysIdleCpuTime(values2)-getSysIdleCpuTime(values1);
    float total_time = active_time + idle_time;
    float result = 100.0*(active_time/ total_time);
    return to_string(result);
}

bool ProcessParser::isPidExisting(string pid){
    bool is_pid_existing;
    // TODO implement
    return is_pid_existing;
}

int ProcessParser::getNumberOfCores(){
    string line;
    string name = "cpu cores";
    ifstream stream;
    Util::getStream((Path::basePath()+"cpuinfo"),stream);

    while(getline(stream, line)){
        if(line.compare(0, name.size(),name)==0){
            vector<string> splittedVector = splitByWhiteSpace(line);
            return stoi(splittedVector[3]);
            
        }
    }

    return 0;
    
}

float ProcessParser::getSysActiveCpuTime(vector<string> values){
    return((stof(values[S_USER])+
            stof(values[S_NICE])+
            stof(values[S_SYSTEM])+
            stof(values[S_IRQ])+
            stof(values[S_IRQ])+
            stof(values[S_SOFTIRQ])+
            stof(values[S_STEAL])+
            stof(values[S_GUEST])+
            stof(values[S_GUEST_NICE])));
}
    
float ProcessParser::getSysIdleCpuTime(vector<string> values){
    return (stof(values[S_IDLE])+stof(values[S_IOWAIT]));
}

vector<string> ProcessParser::splitByWhiteSpace(string stringToSplit){
    istringstream buf(stringToSplit);
    istream_iterator<string> beg(buf), end;
    return vector<string>(beg,end);
}