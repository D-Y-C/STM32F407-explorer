#include <rtthread.h>
#include <string.h>
#include <rtdbg.h>
#include <at_device.h>

static char ssid[64];
static char passwd[64];

#define LOG_TAG "WIFI:"

#define DBG_LVL DBG_LOG

/**
* @brief The usage description of WIFI on rt-Thread
*/
static void usage(void)
{
    rt_kprintf("Usage: wifi -s <ssid> -p <password>\n");
    rt_kprintf("Usage: wifi --off\n");
    rt_kprintf("       wifi --state\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -s           Wifi ssid name\n");
    rt_kprintf("  -p           Wifi password\n");
    rt_kprintf("  --state      Print wifi state\n");
    rt_kprintf("  --help       Print help information\n");
}

/** @brief Wifi conctrl */
static void wifi_contorl(int argc, char** argv)
{
    rt_thread_t tid;

    if (argc == 1 || argc > 5)
    {
        LOG_I("Please check the command you entered!\n");
        goto __usage;
    }
    else {
        struct at_device* dev = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV,"esp0");

        if(dev == RT_NULL)
        {
            LOG_E("Get WIFI device fail");
            return ;
        }

        if (rt_strcmp(argv[1], "--help") == 0){
            goto __usage;
        }
        else if (rt_strcmp(argv[1], "--state") == 0){
            
        }
        else if (rt_strcmp(argv[1], "--off") == 0){
        
            if(RT_EOK == at_device_control(dev,AT_DEVICE_CTRL_NET_DISCONN,NULL))
            {
                LOG_I("AT wifi disconnect success");
            }
            else 
            {
                LOG_I("AT wifi disconnect fail");
            }
           
        }
        else if (rt_strcmp(argv[1], "-s") == 0 && rt_strcmp(argv[3], "-p") == 0){
            if (rt_strlen(argv[2]) > sizeof(ssid))
            {
                LOG_E("The input ssid is too long, max %d bytes!", sizeof(ssid));
                return;
            }

            if (rt_strlen(argv[4]) > sizeof(passwd))
            {
                LOG_E("The input passwd is too long, max %d bytes!", sizeof(passwd));
                return;
            }
            
            rt_memset(ssid, 0x0, sizeof(ssid));
            rt_strncpy(ssid, argv[2], rt_strlen(argv[2]));
            
            rt_memset(passwd, 0x0, sizeof(passwd));
            rt_strncpy(passwd, argv[4], rt_strlen(argv[4]));

            struct at_device_ssid_pwd ssid_pwd;
            ssid_pwd.ssid = ssid;
            ssid_pwd.password= passwd;

            if(RT_EOK == at_device_control(dev,AT_DEVICE_CTRL_SET_WIFI_INFO,(void*)&ssid_pwd))
            {
                LOG_I("Set wifi ssid and password success");
            }
            else 
            {
                LOG_I("Set wifi ssid and password fail");
            }
        }
        else {
            goto __usage;
        }
    }

    return ;
__usage:
    usage();
}

MSH_CMD_EXPORT_ALIAS(wifi_contorl, wifi,
    Wifi contorl. Help: wifi --help);