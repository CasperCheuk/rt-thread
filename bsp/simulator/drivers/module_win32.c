/*
 * File      : module.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-02-26     prife        first version for win32
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtm.h>

#ifdef RT_USING_MODULE
static void rt_module_init_object_container(struct rt_module *module)
{
    RT_ASSERT(module != RT_NULL);

    /* initialize object container - thread */
    rt_list_init(&(module->module_object[RT_Object_Class_Thread].object_list));
    module->module_object[RT_Object_Class_Thread].object_size = sizeof(struct rt_thread);
    module->module_object[RT_Object_Class_Thread].type = RT_Object_Class_Thread;

#ifdef RT_USING_SEMAPHORE
    /* initialize object container - semaphore */
    rt_list_init(&(module->module_object[RT_Object_Class_Semaphore].object_list));
    module->module_object[RT_Object_Class_Semaphore].object_size = sizeof(struct rt_semaphore);
    module->module_object[RT_Object_Class_Semaphore].type = RT_Object_Class_Semaphore;
#endif

#ifdef RT_USING_MUTEX
    /* initialize object container - mutex */
    rt_list_init(&(module->module_object[RT_Object_Class_Mutex].object_list));
    module->module_object[RT_Object_Class_Mutex].object_size = sizeof(struct rt_mutex);
    module->module_object[RT_Object_Class_Mutex].type = RT_Object_Class_Mutex;
#endif

#ifdef RT_USING_EVENT
    /* initialize object container - event */
    rt_list_init(&(module->module_object[RT_Object_Class_Event].object_list));
    module->module_object[RT_Object_Class_Event].object_size = sizeof(struct rt_event);
    module->module_object[RT_Object_Class_Event].type = RT_Object_Class_Event;
#endif

#ifdef RT_USING_MAILBOX
    /* initialize object container - mailbox */
    rt_list_init(&(module->module_object[RT_Object_Class_MailBox].object_list));
    module->module_object[RT_Object_Class_MailBox].object_size = sizeof(struct rt_mailbox);
    module->module_object[RT_Object_Class_MailBox].type = RT_Object_Class_MailBox;
#endif

#ifdef RT_USING_MESSAGEQUEUE
    /* initialize object container - message queue */
    rt_list_init(&(module->module_object[RT_Object_Class_MessageQueue].object_list));
    module->module_object[RT_Object_Class_MessageQueue].object_size = sizeof(struct rt_messagequeue);
    module->module_object[RT_Object_Class_MessageQueue].type = RT_Object_Class_MessageQueue;
#endif

#ifdef RT_USING_MEMHEAP
    /* initialize object container - memory heap */
    rt_list_init(&(module->module_object[RT_Object_Class_MemHeap].object_list));
    module->module_object[RT_Object_Class_MemHeap].object_size = sizeof(struct rt_memheap);
    module->module_object[RT_Object_Class_MemHeap].type = RT_Object_Class_MemHeap;
#endif

#ifdef RT_USING_MEMPOOL
    /* initialize object container - memory pool */
    rt_list_init(&(module->module_object[RT_Object_Class_MemPool].object_list));
    module->module_object[RT_Object_Class_MemPool].object_size = sizeof(struct rt_mempool);
    module->module_object[RT_Object_Class_MemPool].type = RT_Object_Class_MemPool;
#endif

#ifdef RT_USING_DEVICE
    /* initialize object container - device */
    rt_list_init(&(module->module_object[RT_Object_Class_Device].object_list));
    module->module_object[RT_Object_Class_Device].object_size = sizeof(struct rt_device);
    module->module_object[RT_Object_Class_Device].type = RT_Object_Class_Device;
#endif

    /* initialize object container - timer */
    rt_list_init(&(module->module_object[RT_Object_Class_Timer].object_list));
    module->module_object[RT_Object_Class_Timer].object_size = sizeof(struct rt_timer);
    module->module_object[RT_Object_Class_Timer].type = RT_Object_Class_Timer;
}

#ifdef RT_USING_HOOK
static void (*rt_module_load_hook)(rt_module_t module);
static void (*rt_module_unload_hook)(rt_module_t module);

/**
 * @addtogroup Hook
 */

/*@{*/

/**
 * This function will set a hook function, which will be invoked when module
 * be loaded to system.
 *
 * @param hook the hook function
 */
void rt_module_load_sethook(void (*hook)(rt_module_t module))
{
    rt_module_load_hook = hook;
}

/**
 * This function will set a hook function, which will be invoked when module
 * be unloaded from system.
 *
 * @param hook the hook function
 */
void rt_module_unload_sethook(void (*hook)(rt_module_t module))
{
    rt_module_unload_hook = hook;
}

/*@}*/
#endif

/**
 * @ingroup SystemInit
 *
 * This function will initialize system module
 */
void rt_system_module_init(void)
{
}

/**
 * This function will return self module object
 *
 * @return the self module object
 */
rt_module_t rt_module_self(void)
{
    rt_thread_t tid;

    tid = rt_thread_self();
    if (tid == RT_NULL)
        return RT_NULL;

    /* return current module */
    return (rt_module_t)tid->module_id;
}

/**
 * This function will find the specified module.
 *
 * @param name the name of module finding
 *
 * @return the module
 */
rt_module_t rt_module_find(const char *name)
{
    struct rt_object_information *information;
    struct rt_object *object;
    struct rt_list_node *node;

    extern struct rt_object_information rt_object_container[];

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* enter critical */
    rt_enter_critical();

    /* try to find device object */
    information = &rt_object_container[RT_Object_Class_Module];
    for (node = information->object_list.next;
        node != &(information->object_list);
        node = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            rt_exit_critical();

            return (rt_module_t)object;
        }
    }

    /* leave critical */
    rt_exit_critical();

    /* not found */
    return RT_NULL;
}

#ifdef RT_USING_DFS
#include <windows.h>
#include <dfs_posix.h>
extern char * dfs_win32_dirdup(char * path);
static char* _module_name(const char *path)
{
    const char *first, *end, *ptr;
    char *name;
    int size;

    ptr   = (char *)path;
    first = ptr;
    end   = path + rt_strlen(path);

    while (*ptr != '\0')
    {
        if (*ptr == '/')
            first = ptr + 1;
        if (*ptr == '.')
            end = ptr - 1;

        ptr ++;
    }

    size = end - first + 1;
    name = rt_malloc(size);
    rt_strncpy(name, first, size);
    name[size] = '\0';

    return name;
}

typedef int (*appentry_t)(void);
/**
 * This function will load a module from a file
 *
 * @param path the full path of application module
 *
 * @return the module object
 */
rt_module_t rt_module_open(const char *path)
{
    struct dfs_filesystem *fs;
    appentry_t fptr;
    HINSTANCE hinstlib;
    int len;
    char * winpath;
    rt_module_t module;
    char * name;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* check parameters */
    RT_ASSERT(path != RT_NULL);

    /* app module should only in DFS_WIN32 */
    fs = dfs_filesystem_lookup(path);
    if ((fs == RT_NULL) || (strcmp(fs->ops->name,"wdir") != 0))
    {
        rt_kprintf("invalid path: %s\n", path);
        return RT_NULL;
    }

    /* change path */
    len = strlen(path+1);
    if ((winpath = dfs_win32_dirdup((char *)path)) == RT_NULL)
    {
        rt_kprintf("out of memory, exit", path);
        return RT_NULL;
    }

    hinstlib = LoadLibrary(winpath);
    if (hinstlib == NULL)
    {
        rt_kprintf("error: unable to open %s\n", winpath);
        return RT_NULL;
    }

    fptr = (appentry_t)GetProcAddress(hinstlib, "main");
    if (fptr == NULL)
    {
        rt_kprintf("error: unable to find function in %s\n", winpath);
        FreeLibrary(hinstlib);
        return RT_NULL;
    }

    /* get the name of the module */
    name = _module_name(path);

    /* allocate module */
    module = (struct rt_module *)rt_object_allocate(RT_Object_Class_Module,
        name);
    if (!module)
        return RT_NULL;

    module->nref = 0;
    module->module_entry = fptr;

    /* init module object container */
    rt_module_init_object_container(module);

    /* increase module reference count */
    module->nref ++;

    if (module->module_entry != 0)
    {
        rt_uint32_t *stack_size;
        rt_uint8_t  *priority;

#ifdef RT_USING_SLAB
        /* init module memory allocator */
        module->mem_list = RT_NULL;

        /* create page array */
        module->page_array = 
            (void *)rt_malloc(PAGE_COUNT_MAX * sizeof(struct rt_page_info));
        module->page_cnt = 0;
#endif

        /* get the main thread stack size */
        module->stack_size = 2048;
        module->thread_priority = RT_THREAD_PRIORITY_MAX - 2;

        /* create module thread */
        module->module_thread =
            rt_thread_create(name,
            (void(*)(void *))module->module_entry,
            RT_NULL,
            module->stack_size,
            module->thread_priority,
            10);

        RT_DEBUG_LOG(RT_DEBUG_MODULE, ("thread entry 0x%x\n",
            module->module_entry));

        /* set module id */
        module->module_thread->module_id = (void *)module;
        module->parent.flag = RT_MODULE_FLAG_WITHENTRY;

        /* startup module thread */
        rt_thread_startup(module->module_thread);
    }
    else
    {
        /* without entry point */
        module->parent.flag |= RT_MODULE_FLAG_WITHOUTENTRY;
    }

#ifdef RT_USING_HOOK
    if (rt_module_load_hook != RT_NULL)
    {
        rt_module_load_hook(module);
    }
#endif

    rt_free(name);
    return module;
    /* FreeLibrary(hinstlib); */
}

#if defined(RT_USING_FINSH)
#include <finsh.h>

FINSH_FUNCTION_EXPORT_ALIAS(rt_module_open, exec, exec module from a file);
#endif

#endif

rt_err_t rt_module_destroy(rt_module_t module)
{
    return 0;
}
#endif