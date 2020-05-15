/* Name: Loc (Edward) Vo 
   PS ID: 1789932
   Due date: 02/19/20 (using 2 grace days)
   Description: This program simulates-based on input- a series of processes that
    uses CORE, SSD, and User I/O. It shows the order and time of said processes and
    give a summary at the end
*/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
using namespace std;

struct Event {
    int duration;
    string device;
    int processID;
    bool interactiveStatus;
    char completionStatus; // S or C
    int completionTime;
    
};

struct DataPair {
    string keyword;
    int time;
};

class CompareTime {
    public:
        bool operator()(Event& t1, Event& t2)
        {
        return t1.completionTime > t2.completionTime; // smaller time values go first
        }
};

int ncores;
int maxncores;
int ssd;
int global; // currentTime

int totalTime;
int totalSSDAccess;
int totalSSDTime;
int totalCoreTime;
int totalProcesses;
float SSDutil;
float avgBusyCore;

float round(float var) { 
    // 37.66666 * 100 =3766.66 
    // 3766.66 + .5 =3767.16    for rounding off value 
    // then type cast to int so value is 3767 
    // then divided by 100 so the value converted into 37.67 
    float value = (int)(var * 100 + .5); 
    return (float)value / 100; 
}
void  toUpperCase(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}
void coreRequest(Event & event, 
                 priority_queue<Event, vector<Event>, CompareTime> & pq, 
                 queue<Event> & iQ,
                 queue<Event> & niQ, 
                 queue<Event> & ssdQ)
    {
    if (ncores > 0) {
        ncores--;
        // schedule a completion event
        event.completionTime = global + event.duration;
        event.completionStatus = 'C';
        pq.push(event);
        //cout << "--Process " << event.processID << " requests a core at time " << global << "ms for " << event.duration << endl;
        //cout << "--Process " << event.processID << " will release a core at time " << event.completionTime << "ms" << endl;
    }
    else {
        //cout << "--Process " << event.processID << " must wait for a core" << endl;
        if (event.interactiveStatus == true) {
            iQ.push(event);
            //cout << "I Queue now contains " << iQ.size() << " process(es) waiting for a core" << endl;
        }
        else if (event.interactiveStatus == false) {
            niQ.push(event);
            //cout << "NI Queue now contains " << niQ.size() << " process(es) waiting for a core" << endl;
        }
    }
}

void coreRelease(Event & event, // only completion events go in here
                priority_queue<Event, vector<Event>, CompareTime> & pq,
                map<int, vector<Event> > & master, 
                queue<Event> & iQ,
                queue<Event> & niQ, 
                queue<Event> & ssdQ) {

    //cout << "--CORE completion event for process " << event.processID << " at time " << event.completionTime << "ms" << endl << endl;
    ncores++;
    if (iQ.size() > 0) {
        coreRequest(iQ.front(), pq, iQ, niQ, ssdQ);
        iQ.pop();
    }
    else if (niQ.size() > 0) {
        coreRequest(niQ.front(), pq, iQ, niQ, ssdQ);
        niQ.pop();        
    }
    else {
        ncores++;
    }

    int currentProcessAmount = (master[event.processID]).size();

    if (currentProcessAmount > 1) {
    master[event.processID].erase(master[event.processID].begin()); 
    pq.push(master[event.processID].front()); // push into PQ new event in said process ID
    Event tessing = master[event.processID].front();
    tessing = tessing;
    }
    else if (currentProcessAmount == 1) {
    master[event.processID].clear();
    }
    

}

void ssdRequest(Event & event, 
                priority_queue<Event, vector<Event>, CompareTime> & pq,  
                queue<Event> & ssdQ) {
        
        if (ssd == 1) {
            ssd = 0;
            event.completionTime = event.duration + global;
            event.completionStatus = 'C';
            pq.push(event);
            //cout << "-- Process " << event.processID << " requests SSD access at time " << global << "ms for " << event.duration << endl;
            //cout << "-- Process " << event.device << " will release the SSD at time " << event.completionTime << "ms" << endl;
        }
        else {
            ssdQ.push(event);
            //cout << "-- Process " << event.processID << " is now in SSD Queue" << endl;
        }
}

void ssdRelease(Event & event, // only completion events go in here
            priority_queue<Event, vector<Event>, CompareTime> & pq,
            map<int, vector<Event> > & master, 
            queue<Event> & niQ, 
            queue<Event> & ssdQ) {
    
    //cout << "--SSD completion event for process " << event.processID << "at time " << event.completionTime << "ms" << endl << endl;
    ssd = 1;
    event.interactiveStatus = false;
    if (ssdQ.size() > 0) {
        ssdRequest(ssdQ.front(), pq, ssdQ);
        ssdQ.pop();
    }
    else {
        ssd = 1;
    }

    int currentProcessAmount = (master[event.processID]).size();

    if (currentProcessAmount > 1) {
    master[event.processID].erase(master[event.processID].begin()); 
    pq.push(master[event.processID].front()); // push into PQ new event in said process ID
    }
    else if (currentProcessAmount == 1) {
    master[event.processID].clear();
    }
    

}

void ttyRequest(Event & event, 
            priority_queue<Event, vector<Event>, CompareTime> & pq) {
    
    event.completionTime = event.duration + global;
    event.completionStatus = 'C';
    pq.push(event);
}

void ttyRelease(Event & event, 
                priority_queue<Event, vector<Event>, CompareTime> & pq, 
                map<int, vector<Event> > & master, 
                queue<Event> & iQ) {
    //cout << "TTY completion event for process " << event.processID << "at time " << event.completionTime << "ms" << endl;
    event.interactiveStatus = true;
    //global = event.completionTime;

    int currentProcessAmount = (master[event.processID]).size();

    if (currentProcessAmount > 1) {
    master[event.processID].erase(master[event.processID].begin()); 
    pq.push(master[event.processID].front()); // push into PQ new event in said process ID
    }
    else if (currentProcessAmount == 1) {
    master[event.processID].clear();
    }
    

}


int main() {

/*      DATA INITIATION     */
    

    ncores = 1;

    global = 0;
    ssd = 1;
    totalTime = 0;
    totalSSDAccess = 0;
    totalProcesses = 0;

    priority_queue<Event, vector<Event>, CompareTime> pq; // priorityQ
    map<int, vector<Event> > master; // inputTable
    queue<Event> iQ; // interactive queue
    queue<Event> niQ; // non-interactive queue
    queue<Event> ssdQ; // SSD queue
    map<int, string> processTable;
    map<int, string>::iterator it;
    map<int, vector<Event> >::iterator masterIt;
    vector<DataPair> rawInput;


/*    READING IN FILE */
    string keywordInput;
    string line;
    int timeInput;

    try {
        ifstream inputFile;

            while (getline(std::cin,line)) {
                istringstream strm(line);
                strm >> keywordInput;
                strm >> timeInput;
                DataPair dp;
                dp.keyword = keywordInput;
                dp.time = timeInput;
                rawInput.push_back(dp);
            }
        inputFile.close();
        } catch(exception & e) {
            cout << e.what() << endl;
        }


/*--------------------*/
int index = 0;

/*   PROCESSING RAWINPUT */
    while (index < rawInput.size()) {
        //cout << "index is " << index << endl;
        string k = rawInput[index].keyword;
        int t = rawInput[index].time;
        string k_1 = rawInput[index+1].keyword;
        int t_1 = rawInput[index+1].time;
                //cout << "k is " << k << endl;

        
        if (k.compare("NCORES") == 0) {
            //cout << "I'm in NCORES if block" << endl;
            maxncores = t;
            ncores = t;

        }
        else if (k.compare("END") == 0) {
            break;
        }
        else if (k.compare("START") == 0) {
                        //cout << "I'm in START if block" << endl;

            master[t_1] = vector<Event>(); // start a new process with processID = t_1
                //cout << "t_1 is after creating new vector " << t_1 << endl;

            Event inputEvent;
            inputEvent.device = "START";
            inputEvent.duration = t;
            inputEvent.processID = t_1;
            inputEvent.completionTime = t;
            inputEvent.completionStatus = 'S';

            master[t_1].push_back(inputEvent); // insert in the START event
            index += 2; // move from START to PID to CORE
            string newKeyword = rawInput[index].keyword;
            int newTime = rawInput[index].time;

            //cout << "newKeyword is " << newKeyword << endl;

            //cout << "Before START while block" << endl;
            while (!(newKeyword.compare("START") == 0) & (index < rawInput.size()) & !(newKeyword.compare("END")==0)) {

                newKeyword = rawInput[index].keyword;
                newTime = rawInput[index].time;
                //cout << "In START whle block" << endl;
                Event masterEvent;
                masterEvent.device = newKeyword;
                masterEvent.duration = newTime;
                //cout << "t_1 is " << t_1 << endl;
                masterEvent.processID = t_1;
                masterEvent.completionStatus = 'S';
                masterEvent.interactiveStatus = false;
                //cout << "t_1 is " << t_1 << endl;
                //cout << "masterEvent is: " << masterEvent.device << " | " 
                //<< masterEvent.duration << " | " << masterEvent.processID << endl;
                master[t_1].push_back(masterEvent); // master[0].push_back( {CORE 10} )
                

                index++;
                newKeyword = rawInput[index].keyword;
                newTime = rawInput[index].time;
            }
            index--;
        }
        index++;
    }




/*-----------------------*/
/******************************/
                    

for ( masterIt = master.begin(); masterIt != master.end(); masterIt++ ) {
    pq.push(masterIt->second.at(0));
    //cout << masterIt->second.at(0).processID << " | " << masterIt->second.at(0).device << " | " << masterIt->second.at(0).duration << endl;
}


/*        MASTER WHILE LOOP    */
    
    while (!pq.empty()) {

    /* get event */
        Event newE = pq.top();       
        pq.pop();
        
    /*-----------*/

    /* set glpbal time */
        if (newE.completionStatus == 'S' && newE.device.compare("START") == 0) {
            global = newE.duration;
            cout << "inside S" << endl;
        }
        else if (newE.completionStatus == 'C')      
            global = newE.completionTime;
    /*-------------------*/
        //cout << endl << "-- Global time is " << global << endl;
            int beforePTsize = processTable.size();

    /* if event == START */
        if (newE.device.compare("START") == 0) {
            int currentEventDuration = newE.duration;
            processTable[newE.processID] = "RUNNING";
            int currentNcores = ncores;
            master[newE.processID].erase(master[newE.processID].begin()); // delete the START event
            newE = master[newE.processID].front();
            coreRequest(newE, pq, iQ, niQ, ssdQ);
            totalProcesses++;

            if (currentNcores == 0) 
                processTable[newE.processID] = "READY";


            // print out process table
            int tempCompletion = global + newE.duration;
            cout << "Process " << newE.processID << " starts at time " << currentEventDuration << "ms" << endl;
            cout << "Process Table: " << endl;
            if (beforePTsize == 0) {
                cout << "There are no active processes" << endl << endl;
            }
            else {
            for (it = processTable.begin(); it != processTable.end(); it++ ) {
                if (it->first != newE.processID)
                    cout << "Process " << it->first << " is " << it->second << endl;
            }
            
            }
            cout << endl;
        }
    /*-------------------*/

    /* else if event == CORE */
        else if (newE.device.compare("CORE") == 0) {
            if (newE.completionStatus == 'C') {
                coreRelease(newE, pq, master, iQ, niQ, ssdQ);
                processTable[newE.processID] = "READY";
                 
            }
            else if (newE.completionStatus == 'S') {
                coreRequest(newE, pq, iQ, niQ, ssdQ);
                processTable[newE.processID] = "RUNNING";
                totalCoreTime += newE.duration;

                
            }
        }
    /*-----------------------*/

    /* else if event == SSD */
        else if (newE.device.compare("SSD") == 0) {
            if (newE.completionStatus == 'C') {
                ssdRelease(newE, pq, master, niQ, ssdQ);
                processTable[newE.processID] = "READY";

            }
            else if (newE.completionStatus == 'S') {
                ssdRequest(newE, pq, ssdQ);
                processTable[newE.processID] = "BLOCKED";
                totalSSDAccess++;
                totalSSDTime += newE.duration;
            }
        }
    /*----------------------*/

    /* else if event == TTY */
        else if (newE.device.compare("TTY") == 0) {
            if (newE.completionStatus == 'C') {
                ttyRelease(newE, pq, master, iQ);
                processTable[newE.processID] = "READY";

            }
            else if (newE.completionStatus == 'S') {
                ttyRequest(newE, pq);
                processTable[newE.processID] = "BLOCKED";

            }
        }
    /*----------------------*/

    /* Print out terminations */
    if (master[newE.processID].size() == 0) {
        cout << endl << "Process " << newE.processID << " terminates at time " << newE.completionTime << "ms" << endl;
        processTable[newE.processID] = "TERMINATED";
        cout << "Process Table: " << endl;
        for (it = processTable.begin(); it != processTable.end(); it++ ) {
                cout << "Process " << it->first << " is " << it->second << endl;
            }
        processTable.erase(newE.processID);

        }
    
    } // end of while loop
/********************************/
    
    totalTime = global;
    cout << "\nSUMMARY: " << endl;
    SSDutil = 1.0 * totalSSDTime / totalTime;
    cout << "Total elapsed time: " << totalTime << "ms" << endl;
    cout << "Number of processes that completed: " << totalProcesses << endl;
    avgBusyCore = 1.0 * totalCoreTime / totalTime; 
    cout << "Total number of SSD Accesses: " << totalSSDAccess << endl;
    cout << "Average number of busy cores: " << avgBusyCore << endl;
    cout << "SSD Utiliziation: " << SSDutil << endl;



    return 0;
}