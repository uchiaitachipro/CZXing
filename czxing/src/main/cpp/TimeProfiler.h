//
// Created by uchiachen on 2020-01-14.
//

#ifndef CZXING_TIMEPROFILER_H
#define CZXING_TIMEPROFILER_H

#include <utility>
#include <memory>
#include <map>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

class TimeProfiler{

public:

    TimeProfiler(){
        _data = make_shared<std::vector<std::map<std::string,long>>>(std::vector<std::map<std::string,long>>(8));
    }

    void Prepare(){
        _currentRecord = make_shared<std::map<std::string,long>>(std::map<std::string,long>());
        isPrepared = true;
    }

    void StartRecord(const std::string& prefix){
        _prefix = std::string(prefix);
        _startTimePoint = high_resolution_clock::now();
        isStart = true;
    }

    void CompleteRecord(){
        if(!isStart){
            return;
        }
        high_resolution_clock::time_point endTime = high_resolution_clock::now();
        milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - _startTimePoint);
        _currentRecord->insert(std::make_pair(_prefix,timeInterval.count()));
        isStart = false;
    }

    void Commit(){
        if (!isPrepared){
            return;
        }
        _data->push_back(*_currentRecord);
        isPrepared = false;
    }

    std::shared_ptr<std::vector<std::map<std::string,long>>> GetRecords(){
        return _data;
    }

private:
    bool isPrepared = false;
    bool isStart = false;
    std::shared_ptr<std::vector<std::map<std::string,long>>> _data;
    std::shared_ptr<std::map<std::string,long>> _currentRecord;
    std::string _prefix;
    high_resolution_clock::time_point _startTimePoint;

};

#endif //CZXING_TIMEPROFILER_H
