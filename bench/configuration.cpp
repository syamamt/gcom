#include "configuration.hpp"
#include <iostream>
#include <string.h>

configuration::configuration(char *argv[])
{
    id = atoi(argv[1]);
    strncpy(ipaddr, argv[2], strlen(argv[2]));
    ipaddr[strlen(argv[2])] = '\0';
    port = atoi(argv[3]);
    duration = atoi(argv[4]);

    // only master
    if (id == 1)
    {
        iops = atoi(argv[5]);
        request_interval = 1000/iops;
        threads = atoi(argv[6]);
        log_size = atoi(argv[7]);
        slaves = atoi(argv[8]);

        for (int slave = 0; slave < slaves; slave++)
        {
            int idx = 9 + slave*2;
            strncpy(slave_ipadder[slave], argv[idx], strlen(argv[idx]));
            slave_ipadder[slave][strlen(argv[idx])] = '\0';
            slave_port[slave] = atoi(argv[idx+1]);
        }
    }
}

void configuration::print()
{
    fprintf(stdout, "---------server_config---------\n");
    fprintf(stdout, "server id: %d\n", id);
    fprintf(stdout, "ip address: %s\n", ipaddr);
    fprintf(stdout, "port: %d\n", port);
    fprintf(stdout, "run duration: %d\n", duration);

    // only master
    if (id == 1)
    {
        fprintf(stdout, "traffic: %d [ops/sec]\n", iops);
        fprintf(stdout, "number of client threads: %d\n", threads);
        fprintf(stdout, "send log size: %d [bytes]\n", log_size);
        fprintf(stdout, "number of slaves: %d\n", slaves);
        for (int slave = 0; slave < slaves; slave++) {
            fprintf(stdout, "slave: ip address: %s, port: %d\n", slave_ipadder[slave], slave_port[slave]);
        }
    }
    fprintf(stdout, "-------------------------------\n");
}