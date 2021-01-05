/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-29     supperthomas first version
 *
 */

#include <rtthread.h>
#include <rtdevice.h>

#define DK_BOARD_LED_1  13
#define DK_BOARD_LED_2  14

#define FS_PARTITION_NAME  "filesystem"


#define LOG_TAG     "main"     // 该模块对应的标签。不定义时，默认：NO_TAG
#define LOG_LVL     LOG_LVL_DBG   // 该模块对应的日志输出级别。不定义时，默认：调试级别
#include <ulog.h>



#include "fal.h"
#include "dfs_file.h"

#include "flashdb.h"



static uint32_t boot_count = 0;
static time_t boot_time[10] = {0, 1, 2, 3};
/* default KV nodes */
static struct fdb_default_kv_node default_kv_table[] = {
        {"username", "armink", 0}, /* string KV */
        {"password", "123456", 0}, /* string KV */
        {"boot_count", &boot_count, sizeof(boot_count)}, /* int type KV */
        {"boot_time", &boot_time, sizeof(boot_time)},    /* int array type KV */
};
/* KVDB object */
static struct fdb_kvdb kvdb = { 0 };
/* TSDB object */
struct fdb_tsdb tsdb = { 0 };
/* counts for simulated timestamp */
static int counts = 0;

extern void kvdb_basic_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_string_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_blob_sample(fdb_kvdb_t kvdb);
extern void tsdb_sample(fdb_tsdb_t tsdb);

static fdb_time_t get_time(void)
{
    /* Using the counts instead of timestamp.
     * Please change this function to return RTC time.
     */
    return ++counts;
}


int main(void)
{
    fal_init();
    ulog_init();
    
    struct rt_device *flash_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);

    if (flash_dev == NULL)
    {
        LOG_E("Can't create a mtd device on '%s' partition.\n", FS_PARTITION_NAME);
    }
    else
    {
        LOG_I("Create a mtd device on the %s partition of flash successful.\n", FS_PARTITION_NAME);
    }

    if(rt_device_find(FS_PARTITION_NAME) != RT_NULL)
    {
        //

        if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == RT_EOK)
        {
            LOG_I("onchip lfs filesystem mount to '/'\n");
        }
        else
        {
            LOG_E("onchip lfs filesystem mount to '/' failed!\n");
            LOG_D("formating this fartion...\n");
            dfs_mkfs("lfs", FS_PARTITION_NAME);
            LOG_I("reboot now...");
            rt_hw_cpu_reset();
        }
    }
    else
    {
        LOG_E("find filesystem portion failed\r\n");
    }

    fdb_err_t result;
    //easyflash_init();
    //fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_LOCK, lock);
    //fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_UNLOCK, unlock);
    /* Time series database initialization
        *
        *       &tsdb: database object
        *       "log": database name
        * "fdb_tsdb1": The flash partition name base on FAL. Please make sure it's in FAL partition table.
        *              Please change to YOUR partition name.
        *    get_time: The get current timestamp function.
        *         128: maximum length of each log
        *        NULL: The user data if you need, now is empty.
        */
    result = fdb_tsdb_init(&tsdb, "log", "tv", get_time, 128, NULL);
    /* read last saved time for simulated timestamp */
    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_GET_LAST_TIME, &counts);

    if (result != FDB_NO_ERR) {
        return -1;
    }
    
    //result = fdb_kvdb_init()

    /* run TSDB sample */
    //tsdb_sample(&tsdb);

    int count = 1; 
    rt_pin_mode(DK_BOARD_LED_1, PIN_MODE_OUTPUT);
    
    while (count++)
    {    
        rt_pin_write(DK_BOARD_LED_1, PIN_HIGH);
        rt_thread_mdelay(500);
        
        rt_pin_write(DK_BOARD_LED_1, PIN_LOW);
        rt_thread_mdelay(500);                    
    }
    return RT_EOK;
}

