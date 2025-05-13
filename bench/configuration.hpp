#pragma once

#define BUFFSIZE BUFSIZ

class configuration
{
public:
    int id;
    char ipaddr[32];
    int port;
    int duration = 0; // sec

    // master
    int iops = 0; // ops/sec
    int request_interval = 0; // ms
    int threads = 0;
    int log_size = 0; // bytes
    int slaves = 0; // slaves <= 9
    char slave_ipadder[32][10];
    int slave_port[10];

    configuration(char *argv[]);
    void print();
};