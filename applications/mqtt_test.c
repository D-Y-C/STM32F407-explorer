
/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @LastEditTime: 2020-06-17 14:35:29
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "mqttclient.h"

#ifndef LOG_TAG
#else
#define DBG_TAG LOG_TAG
#endif /* LOG_TAG */

#ifdef DRV_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

/** IP setting */
static char ip_address_str[64];
static char port_str[32];

/** Access */
static char user_name_str[64];
static char passwd_str[64];
static char cid[64] = {0};

/** Kawaii mqtt client */
static mqtt_client_t *client = NULL;
static bool is_running = false;
static bool request_stop = false;

/** publish */
/** subscribe */
static char sub_topic_name[32];
static bool sub_is_running = false;
static uint32_t sub_rec_cnt = 0;

/** @brief Subscribe receive message callback
 *  @param[in] client Client instance
 *  @param[in] msg Receive message
 */
static void sub_topic_handle(void *client, message_data_t *msg)
{
//    (void)client;
//    LOG_I("-----------------------------------------------------------------------------------");
//    LOG_I("%s:%d %s()...\ntopic: %s\nmessage:%s, total recv : %u", __FILE__, __LINE__, __FUNCTION__, msg->topic_name, (char *)msg->message->payload, sub_rec_cnt++);
//    LOG_I("-----------------------------------------------------------------------------------");
    sub_rec_cnt ++;
}

/** @brief Publish message by mqtt client
 *  @param[in] client Client instance
 *  @param[in] pub_name Mqtt publish name
 *  @param[in] msg Message that will publish
 *  @return 0 if success
 */
static int ka_mqtt_publish(char *pub_name, char *data)
{
    mqtt_message_t msg;

    /* client is ready*/
    if (client == NULL || is_running == false)
    {
        LOG_W("Client is not running, can't publish message");
        return -1;
    }

    /* clear message */
    memset(&msg, 0, sizeof(msg));

    /* copy data and set qoi */
    msg.qos = QOS0;
    msg.payload = (void *)data;

    /* low level function */
    mqtt_publish(client, pub_name, &msg);

    return 0;
}

/** @brief Publish message by mqtt client
 *  @param[in] client Client instance
 *  @param[in] topic_name Mqtt topic name
 *  @return 0 if success
 */
static int ka_mqtt_sub_start(const char *topic_name)
{
    /* client is ready */
    if (client == NULL || is_running == false || sub_is_running == true)
    {
        LOG_W("Client is not running or sub is running, can't register subscribe");
        return -1;
    }

    LOG_I("kawaii mqtt subscribe start, topic : %s", topic_name);
    /* register subscribe */
    if (mqtt_subscribe(client, topic_name, QOS0, sub_topic_handle))
    {
        return -1;
    }
    sub_is_running = true;

    return 0;
}

static int ka_mqtt_sub_stop(char *topic_name)
{
    /* client is ready */
    if (client == NULL || is_running == false || sub_is_running == false)
    {
        LOG_W("Client or subscribe is not running, can't register subscribe");
        return -1;
    }

    sub_is_running = false;
    return mqtt_unsubscribe(client, topic_name);
}

/** @brief Stop kawaii mqtt client
 *
 */
static int stop_ka_mqtt_client()
{
    if (client == NULL || is_running == false)
        return -1;

    is_running = false;
    request_stop = true;
    mqtt_disconnect(client);
    mqtt_release(client);
    client = NULL;
}

static void mqtt_client_thread()
{
    while (1)
    {
        if (client)
        {
            /*
               if(mqtt_keep_alive(client) != KAWAII_MQTT_SUCCESS_ERROR)
                {
                    break;
                }
            */
            char data[128];
            sprintf(data, "sub count : %u", sub_rec_cnt);
            rt_kprintf("%s\n",data);
        //    ka_mqtt_publish("rtt_pub", data);
        }

        if (request_stop)
        {
            break;
        }

        rt_thread_mdelay(1000);
    }
}

/** @brief Start kawaii mqtt client
 *  @return 0 if success
 */
static int start_ka_mqtt_client()
{
    if (is_running == true)
    {
        LOG_W("kawaii mqtt is running");
        return -1;
    }

    client = mqtt_lease();

    rt_snprintf(cid, sizeof(cid), "rtthread%d", rt_tick_get());

    mqtt_set_host(client, ip_address_str);
    mqtt_set_port(client, port_str);
    mqtt_set_user_name(client, user_name_str);
    mqtt_set_password(client, passwd_str);
    mqtt_set_client_id(client, cid);
    mqtt_set_clean_session(client, 1);

    LOG_I("The ID of the Kawaii client is: %s ", cid);

    if (mqtt_connect(client))
    {
        LOG_E("MQTT start fail");
        return -1;
    }

    // mqtt_subscribe(client, "rtt_sub", QOS0, sub_topic_handle);

    is_running = true;
    request_stop = false;

    /* start background thread */
    rt_thread_t tid;

    tid = rt_thread_create("kawaii_mqtt_client", mqtt_client_thread, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid == RT_NULL)
    {
        stop_ka_mqtt_client();
        return -2;
    }

    rt_thread_startup(tid);
    return 0;
}

/*************************************************************************
 * Command line parse
 ************************************************************************/

/**
 * @brief The usage description of ka mqtt client
 */
static void usage(void)
{
    rt_kprintf("Usage: ka_mqtt -i <ip> -p <port>\n");
    rt_kprintf("       ka_mqtt --state\n");
    rt_kprintf("       ka_mqtt --stop\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -i           IP address\n");
    rt_kprintf("  -p           Port\n");
    rt_kprintf("  --state      Print kawaii-mqtt state\n");
    rt_kprintf("  --help       Print help information\n");
}

/**
 * @brief kawaii MQTT server thread entry
 */
static void mqtt_client(int argc, char **argv)
{

    if (argc == 1 || argc > 5)
    {
        LOG_I("Please check the command you entered!\n");
        goto __usage;
    }
    else
    {
        if (rt_strcmp(argv[1], "--help") == 0)
        {
            goto __usage;
        }
        else if (rt_strcmp(argv[1], "--state") == 0)
        {
            if (is_running)
            {
                LOG_I("kawaii mqtt running");
            }
            else
            {
                LOG_I("kawaii mqtt stop");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            stop_ka_mqtt_client();
        }
        else if (rt_strcmp(argv[1], "-i") == 0 && rt_strcmp(argv[3], "-p") == 0)
        {
            if (rt_strlen(argv[2]) > sizeof(ip_address_str))
            {
                LOG_E("The ip address d is too long, max %d bytes!", sizeof(ip_address_str));
                return;
            }

            if (rt_strlen(argv[4]) > sizeof(port_str))
            {
                LOG_E("The input port is too long, max %d bytes!", sizeof(port_str));
                return;
            }

            rt_memset(ip_address_str, 0x0, sizeof(ip_address_str));
            rt_strncpy(ip_address_str, argv[2], rt_strlen(argv[2]));

            rt_memset(port_str, 0x0, sizeof(port_str));
            rt_strncpy(port_str, argv[4], rt_strlen(argv[4]));

            rt_memset(user_name_str, 0x0, sizeof(user_name_str));
            rt_memset(passwd_str, 0x0, sizeof(passwd_str));

            /* start mqtt client */
            if (start_ka_mqtt_client() != 0)
            {
                LOG_E("start ka mqtt client fail");
            }
        }
        else
        {
            goto __usage;
        }
    }

    return;
__usage:
    usage();
}

/**
 * @brief The usage description of mqtt publisher
 */
static void pub_usage(void)
{
    rt_kprintf("Usage: ka_mqtt_pub -n <pub name> -m <message>\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -t           Topic name\n");
    rt_kprintf("  -m           Message\n");
    rt_kprintf("  --help       Print help information\n");
}

/**
 * @brief The usage description of mqtt publisher
 */
static void mqtt_pub(int argc, char **argv)
{
    if (argc == 1 || argc > 5)
    {
        LOG_I("Please check the command you entered!\n");
        goto __usage;
    }
    else
    {
        if (rt_strcmp(argv[1], "--help") == 0)
        {
            goto __usage;
        }
        else if (rt_strcmp(argv[1], "-t") == 0 && rt_strcmp(argv[3], "-m") == 0)
        {
            if (0 != ka_mqtt_publish(argv[2], argv[4]))
            {
                LOG_W("MQTT publish fail");
            }
        }
        else
        {
            goto __usage;
        }
    }

    return;
__usage:
    pub_usage();
}

/**
 * @brief The usage description of ,qtt subscribe
 */
static void sub_usage(void)
{
    rt_kprintf("Usage: ka_mqtt_sub -t <topic name> \n");
    rt_kprintf("       ka_mqtt_sub --stop\n");
    rt_kprintf("\n");
    rt_kprintf("Miscellaneous:\n");
    rt_kprintf("  -t           Topic name\n");
    rt_kprintf("  --stop       Stop mqtt subscribe listen\n");
    rt_kprintf("  --help       Print help information\n");
}

static void mqtt_sub(int argc, char **argv)
{
    if (argc == 1 || argc > 3)
    {
        LOG_I("Please check the command you entered!\n");
        goto __usage;
    }
    else
    {
        if (rt_strcmp(argv[1], "--help") == 0)
        {
            goto __usage;
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            if(argc < 3)
            {
                goto __usage;
            }

            if (0 != ka_mqtt_sub_stop(argv[2]))
            {
                LOG_W("MQTT unsubscribe fail");
            }
        }
        else if (rt_strcmp(argv[1], "-t") == 0)
        {
            if (rt_strlen(argv[2]) > sizeof(sub_topic_name))
            {
                LOG_E("The topic name is too long, max %d bytes!", sizeof(sub_topic_name));
                return;
            }

            rt_memset(sub_topic_name, 0x0, sizeof(sub_topic_name));
            rt_strncpy(sub_topic_name, argv[2], rt_strlen(argv[2]));

            if (0 != ka_mqtt_sub_start(sub_topic_name))
            {
                LOG_W("MQTT subscribe fail");
            }
        }
        else
        {
            goto __usage;
        }
    }

    return;
__usage:
    sub_usage();
}

MSH_CMD_EXPORT(mqtt_client, Kawaii MQTT client test program);
MSH_CMD_EXPORT(mqtt_pub, Publish mqtt message);
MSH_CMD_EXPORT(mqtt_sub, Listen mqtt message);
