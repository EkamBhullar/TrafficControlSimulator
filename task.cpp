#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
using namespace std;
mutex dataMutex;

int numProducerThreads = 3;
int numConsumerThreads = 3;

int producerCount = 0; 
int consumerCount = 0;
int totalRows = 0; 

condition_variable producerCV, consumerCV; 

vector<int> indexVector;
vector<int> trafficLightVector;
vector<int> carCountVector;
vector<string> timeStampVector;

struct TrafficSignal
{
    int index;
    string timeStamp;
    int trafficLightID;
    int numberOfCars;
};

TrafficSignal signalArray[5] = {{0, "", 1, 0}, {0, "", 2, 0}, {0, "", 3, 0}, {0, "", 4, 0},{0, "", 5, 0}};


queue<TrafficSignal> trafficSignalQueue;
bool sortByTrafficCount(TrafficSignal first, TrafficSignal second)
{
    return  (first.numberOfCars > second.numberOfCars);
  
}

void producerFunction()
{
    while (producerCount < totalRows)
    {
        unique_lock<mutex> lock(dataMutex); 

        if (producerCount < totalRows)
        {
            trafficSignalQueue.push(TrafficSignal{indexVector[producerCount], timeStampVector[producerCount], trafficLightVector[producerCount], carCountVector[producerCount]}); // Push into queue
            consumerCV.notify_all(); 
            producerCount++;
        }
        else
        {
            producerCV.wait(lock, [] { return producerCount < totalRows; }); // If count is greater than the number of rows in the dataset, wait
        }

        lock.unlock();
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}
TrafficSignal signal;
void consumerFunction()
{
    while (consumerCount < totalRows)
    {
        unique_lock<mutex> lock(dataMutex); // Lock until processing

        if (!trafficSignalQueue.empty())
        {
            signal = trafficSignalQueue.front();

            if (signal.trafficLightID == 1)
            {
                signalArray[0].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 2)
            {
                signalArray[1].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 3)
            {
                signalArray[2].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 4)
            {
                signalArray[3].numberOfCars += signal.numberOfCars;
            }
            if (signal.trafficLightID == 5)
            {
                signalArray[4].numberOfCars += signal.numberOfCars;
            }

            trafficSignalQueue.pop(); 
            producerCV.notify_all(); 
            consumerCount++;
        }
        else
        {
            consumerCV.wait(lock, [] { return !trafficSignalQueue.empty(); }); 
        }

        if (!trafficSignalQueue.empty() && consumerCount % 60 == 0)
        { 
            sort(signalArray, signalArray + 5, sortByTrafficCount); 
            cout << "Time: " << signal.timeStamp << endl;
            cout << "Traffic Light" << "\t" << "Number of Cars" << endl;
            for (int i = 0; i < 5; ++i)
            {
                cout << signalArray[i].trafficLightID << "\t\t" << signalArray[i].numberOfCars << endl;
            }
        }

        lock.unlock();
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}


int main()
{

  
    ifstream file;

    file.open("traffic_data.txt");

    if (file.is_open())
    {
        string line;
        getline(file, line);

        while (!file.eof())
        {
            string index, timeStamp, trafficLightID, numberOfCars;
            getline(file, index, ',');
            indexVector.push_back(stoi(index));
    
            getline(file, timeStamp, ',');
            timeStampVector.push_back(timeStamp);
            getline(file, trafficLightID, ',');
            trafficLightVector.push_back(stoi(trafficLightID));
    
            getline(file, numberOfCars, '\n');
            carCountVector.push_back(stoi(numberOfCars));
     

            totalRows += 1;
        }
        file.close();
    }
    else
    {
        cout << "Could not open file, try again.";
    }

    thread producers[numProducerThreads];
    thread consumers[numConsumerThreads];

    for (int i = 0; i < numProducerThreads; i++)
        producers[i] = thread(producerFunction);
    for (int i = 0; i < numConsumerThreads; i++)
        consumers[i] = thread(consumerFunction);

    for (int i = 0; i < numProducerThreads; i++)
        producers[i].join();
    for (int i = 0; i < numConsumerThreads; i++)
        consumers[i].join();
}
