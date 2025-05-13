#include "logger.hpp"
#include <syncstream>
#include <iostream>
#include <iomanip>

void logger::request_transaction(int txid, int clientid)
{
    double time = get_time();

    // time, type, clientid, txid,
    std::osyncstream syncout{stream};
    syncout << std::fixed << std::setprecision(4) << time << ",";
    syncout << type_request_transaction << ",";
    syncout << clientid << ",";
    syncout << txid << ",";
    syncout << std::endl;
}

void logger::response_transaction(int txid, int clientid)
{
    double time = get_time();

    // time, type, clientid, txid,
    std::osyncstream syncout{stream};
    syncout << std::fixed << std::setprecision(4) << time << ",";
    syncout << type_response_transaction << ",";
    syncout << clientid << ",";
    syncout << txid << ",";
    syncout << std::endl;
}

void logger::send_message(int size, std::string fromipaddr, int fromport)
{
    double time = get_time();

    // time, type, size, fromipaddr, fromport,
    std::osyncstream syncout{stream};
    syncout << std::fixed << std::setprecision(4) << time << ",";
    syncout << type_send_message << ",";
    syncout << size << ",";
    syncout << fromipaddr << ",";
    syncout << fromport << ",";
    syncout << std::endl;
}

void logger::recv_message(int size, std::string toipaddr, int toport)
{
    double time = get_time();

    // time, type, size, toipaddr, toport,
    std::osyncstream syncout{stream};
    syncout << std::fixed << std::setprecision(4) << time << ",";
    syncout << type_recv_message << ",";
    syncout << size << ",";
    syncout << toipaddr << ",";
    syncout << toport << ",";
    syncout << std::endl;
}

inline double logger::get_time()
{
    using namespace std::chrono;
    system_clock::time_point now = system_clock::now();
    microseconds logtime = duration_cast<milliseconds>(now-start);
    return static_cast<double>(logtime.count()) / 1000000;
}