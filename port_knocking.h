#ifndef __PORT_KNOCKING_H
#define __PORT_KNOCKING_H

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include "common.h"

int port_knocking_init(void);
int port_knocking_clean(void);

#endif