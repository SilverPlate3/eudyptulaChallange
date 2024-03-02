#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/textsearch.h>

#define EUDYPTULA_ID "fake_id_123"
struct ts_config *conf;

static void PrintIp(unsigned int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    pr_info("Recived packet from: %d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);  
}

static unsigned int hook_function_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
    
    if (ip_header->protocol == IPPROTO_ICMP) 
    {
        pr_info("ICMP packet\n");
    }

    if(skb_find_text(skb, 0, skb->len, conf) != UINT_MAX)
    {
        unsigned int src_ip = (unsigned int)ip_header->saddr;
        PrintIp(src_ip); 
    }
    
    return NF_ACCEPT;
}

static struct nf_hook_ops nfho_in = {
    .hook = hook_function_in,
    .hooknum = NF_INET_LOCAL_IN,
    .pf = PF_INET,
    .priority = NF_IP_PRI_FIRST,
};


static int netfilter_start(void)
{
    pr_info("Hello\n");

    conf = textsearch_prepare("kmp", EUDYPTULA_ID, strlen(EUDYPTULA_ID), GFP_KERNEL, TS_AUTOLOAD);
    if (IS_ERR(conf)) 
    {
        return PTR_ERR(conf);
    }

    struct net *n;
    for_each_net(n)
    {
        if(nf_register_net_hook(n, &nfho_in) != 0)
            pr_alert("Failed to register net hook\n");
    }

    return 0;
}

static void netfilter_exit(void)
{
    struct net *n;
    for_each_net(n)
    {
        nf_unregister_net_hook(n, &nfho_in);
    }
    textsearch_destroy(conf);
    pr_info("Bye\n");
}

module_init(netfilter_start);
module_exit(netfilter_exit);

MODULE_AUTHOR("Chuckleberryfinn");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");