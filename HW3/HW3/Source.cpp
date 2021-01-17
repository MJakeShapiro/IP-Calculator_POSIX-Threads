#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <pthread.h>

using namespace std;

struct netParams
{
    string netLine;
    string ip;
    string subnet;
    int netadd[4] = { 0, 0, 0, 0 };
    int broadc[4] = { 0, 0, 0, 0 };
    int hostMin[4] = { 0, 0, 0, 0 };
    int hostMax[4] = { 0, 0, 0, 0 };
    int nHosts = 0;
};

struct changeData
{
    string ip;
    string subnet;
    int netadd = 0;
    int broadc = 0;
    int hostMin = 0;
    int hostMax = 0;
    int pos;
};

void SetupLine(netParams* PARAMS);
void* BroadcastChildThread(void* oct_void_ptr);
void* NetworkChildThread(void* oct_void_ptr);
void printToScreen(netParams params);
string snToBin(string sn);
string numArrToStr(int numArr[4]);
int numHostCal(string snid);

int main()
{
    static vector<netParams> PARAMS;
    vector<string> dataParams;
    string line;
    int linesCount;
    getline(cin, line);
    linesCount = stoi(line);
    while (getline(cin, line))
    {
        dataParams.push_back(line);
    }

    //linesCount = linesCount;

    for (int i = 0; i < linesCount; i++)
    {
        netParams temp;
        temp.netLine = dataParams[i];
        PARAMS.push_back(temp);
    }

    pthread_t tids[linesCount];

    for (int i = 0; i < linesCount; i++)
    {
        SetupLine(&PARAMS[i]);
    }

    //Print to screen
    for (int i = 0; i < linesCount; i++)
    {
        cout << "IP Address: " << PARAMS[i].ip << endl;
        cout << "Subnet: " << PARAMS[i].subnet << endl;
        cout << "Network: " << numArrToStr(PARAMS[i].netadd) << endl;
        cout << "Broadcast: " << numArrToStr(PARAMS[i].broadc) << endl;
        cout << "HostMin: " << numArrToStr(PARAMS[i].hostMin) << endl;
        cout << "HostMax: " << numArrToStr(PARAMS[i].hostMax) << endl;
        cout << "# Hosts: " << PARAMS[i].nHosts << endl
            << endl;
    }
    return 0;
}

void SetupLine(netParams* PARAMS)
{
    netParams* net_Params = PARAMS;
    string line = net_Params->netLine;
    string ip, subnet;
    stringstream sstream(line);
    sstream >> ip >> subnet;
    net_Params->ip = ip;
    net_Params->subnet = subnet;

    changeData chDa[4];
    for (int i = 0; i < 4; i++)
    {
        chDa[i].ip = ip;
        chDa[i].subnet = subnet;
        chDa[i].netadd = 0;
        chDa[i].broadc = 0;
        chDa[i].hostMax = 0;
        chDa[i].hostMin = 0;
        chDa[i].pos = i;
    }

    pthread_t gctids[4];
    for (int i = 0; i < 4; i++)
    {

        pthread_create(&gctids[i], NULL, NetworkChildThread, &chDa[i]);
        pthread_create(&gctids[i], NULL, BroadcastChildThread, &chDa[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        // CAN'T USE THIS
        pthread_join(gctids[i], NULL);
    }

    for (int i = 0; i < 4; i++)
    {
        net_Params->netadd[i] = chDa[i].netadd;
        net_Params->broadc[i] = chDa[i].broadc;
        net_Params->hostMax[i] = chDa[i].hostMax;
        net_Params->hostMin[i] = chDa[i].hostMin;
    }

    string sid;
    sid = snToBin(net_Params->subnet);
    net_Params->nHosts = numHostCal(sid);
}

void* BroadcastChildThread(void* chDa_void_ptr)
{
    changeData* chDa_ptr = (changeData*)chDa_void_ptr;
    int pos = chDa_ptr->pos;
    string ip, subnet;
    ip = chDa_ptr->ip;
    subnet = chDa_ptr->subnet;

    int ipo[4], subo[4];
    replace(ip.begin(), ip.end(), '.', ' ');
    replace(subnet.begin(), subnet.end(), '.', ' ');
    stringstream ssip(ip), sssubnet(subnet);
    for (int i = 0; i < 4; i++)
    {
        ssip >> ipo[i];
        sssubnet >> subo[i];
    }
    chDa_ptr->broadc = (ipo[pos] | ~subo[pos]) + 256;
    if (pos == 3) {
        chDa_ptr->hostMax = chDa_ptr->broadc - 1;
    }
    else {
        chDa_ptr->hostMax = chDa_ptr->broadc;
    }
    return NULL;
}

void* NetworkChildThread(void* chDa_void_ptr)
{
    changeData* chDa_ptr = (changeData*)chDa_void_ptr;
    int pos = chDa_ptr->pos;
    string ip, subnet;
    ip = chDa_ptr->ip;
    subnet = chDa_ptr->subnet;

    int ipo[4], subo[4];
    replace(ip.begin(), ip.end(), '.', ' ');
    replace(subnet.begin(), subnet.end(), '.', ' ');
    stringstream ssip(ip), sssubnet(subnet);
    for (int i = 0; i < 4; i++)
    {
        ssip >> ipo[i];
        sssubnet >> subo[i];
    }
    chDa_ptr->netadd = ipo[pos] & subo[pos];
    if (pos == 3) {
        chDa_ptr->hostMin = chDa_ptr->netadd + 1;
    }
    else {
        chDa_ptr->hostMin = chDa_ptr->netadd;
    }
    return NULL;
}

string snToBin(string sn)
{
    string bin = "";
    replace(sn.begin(), sn.end(), '.', ' ');
    int dec[4];
    stringstream sssubnet(sn);
    for (int i = 0; i < 4; i++)
    {
        sssubnet >> dec[i];
    }
    string temp[4];
    for (int i = 0; i < 4; i++)
    {
        char temp[i][8];
        for (int j = 7; j >= 0; j--)
        {
            if ((dec[i] % 2) == 0)
            {
                temp[i][j] = '0';
            }
            else
            {
                temp[i][j] = '1';
            }
            dec[i] = dec[i] / 2;
        }
        bin = bin + temp[i];
    }
    return bin;
}

string numArrToStr(int numArr[4])
{
    string finStr = "";
    for (int i = 0; i < 4; i++)
    {
        if (i < 3)
            finStr = finStr + to_string(numArr[i]) + ".";
        else
            finStr = finStr + to_string(numArr[i]);
    }
    return finStr;
}

int numHostCal(string snid)
{
    int nh, numZ = 0;
    for (int i = 0; i < snid.size(); i++)
    {
        if (snid[i] == '0')
            numZ++;
    }
    nh = pow(2, numZ) - 2;
    return nh;
}