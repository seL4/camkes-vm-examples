/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#define _VAR_STRINGIZE(...) #__VA_ARGS__
#define VAR_STRINGIZE(...) _VAR_STRINGIZE(__VA_ARGS__)

/*
 * Calls macro f with each argument e.g. a,b,c,..
 */
#define __CALL1(f,a) f(a)
#define __CALL2(f,a,b) f(a) f(b)
#define __CALL3(f,a,b,c) f(a) f(b) f(c)
#define __CALL4(f,a,b,c,d) f(a) f(b) f(c) f(d)

#define __CALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __CALL_CONCAT_X(a,b) a##b
#define __CALL_CONCAT(a,b) __CALL_CONCAT_X(a,b)
#define __CALL_NARGS_FROM(...) __CALL_NARGS_X(__VA_ARGS__,8,7,6,5,4,3,2,1,)
#define __CALL_DISP_FROM(f, b,...) __CALL_CONCAT(b,__CALL_NARGS_FROM(__VA_ARGS__))(f, __VA_ARGS__)
#define __CALL(f, args...) __CALL_DISP_FROM(f, __CALL, args)

#define VM_COMPONENT_DEF(num) \
    component VM vm##num; \

#define VM_COMPONENT_CONNECTIONS_DEF(num) \
    connection seL4RPCDataport fs##num(from vm##num.fs, to fserv.fs_ctrl); \
    connection seL4GlobalAsynch notify_ready_vm##num(from vm##num.notification_ready_connector, to vm##num.notification_ready); \

#define VM_GENERAL_COMPOSITION_DEF() \
    component FileServer fserv; \

#define VM_COMPOSITION_DEF(num) \
    VM_COMPONENT_DEF(num) \
    VM_COMPONENT_CONNECTIONS_DEF(num) \

#define VM_GENERAL_CONFIGURATION_DEF() \
    fserv.heap_size = 165536; \

#define VM_CONFIGURATION_DEF(num) \
    vm##num.fs_attributes = VAR_STRINGIZE(num); \
    vm##num.fs_shmem_size = 0x1000; \
    vm##num.asid_pool = true; \
    vm##num.simple = true; \
    vm##num.cnode_size_bits = 23; \
    vm##num.simple_untyped24_pool = 12; \
    vm##num.base_prio = 100; \
    vm##num._priority = 101; \
    vm##num.sem_value = 0; \
    vm##num.notification_ready_global_endpoint = VAR_STRINGIZE(vm##num); \
    vm##num.notification_ready_connector_global_endpoint = VAR_STRINGIZE(vm##num); \

#define VM_VIRTUAL_SERIAL_COMPONENTS_DEF() \
    component SerialServer serial; \
    component TimeServer time_server; \
    connection seL4TimeServer serialserver_timer(from serial.timeout, to time_server.the_timer); \

#define PER_VM_VIRTUAL_SERIAL_CONNECTIONS_DEF(num) \
    connection seL4RPCCall serial_vm##num(from vm##num.putchar, to serial.processed_putchar); \
    connection seL4SerialServer serial_input_vm##num(from vm##num.serial_getchar, to serial.getchar);

#define VM_VIRTUAL_SERIAL_COMPOSITION_DEF(vm_ids...) \
    VM_VIRTUAL_SERIAL_COMPONENTS_DEF() \
    __CALL(PER_VM_VIRTUAL_SERIAL_CONNECTIONS_DEF, vm_ids) \

#define VM_VIRTUAL_SERIAL_GENERAL_CONFIGURATION_DEF() \
    time_server.putchar_attributes = 0; \
    time_server.timers_per_client = 1; \
    time_server.priority = 255; \
    time_server.simple = true; \
    serial.timeout_attributes = 1; \
    serial.serial_wait_global_endpoint = "serial"; \
    serial.self_global_endpoint = "serial"; \

#define PER_VM_VIRTUAL_SERIAL_CONFIGURATION_DEF(num) \
    vm##num.putchar_attributes = VAR_STRINGIZE(num); \
    vm##num.serial_getchar_global_endpoint = VAR_STRINGIZE(vm##num); \
    vm##num.serial_getchar_badge = "4"; \
    vm##num.serial_getchar_attributes = VAR_STRINGIZE(num); \
    vm##num.serial_getchar_shmem_size = 0x1000; \

#define VM_VIRTUAL_SERIAL_CONFIGURATION_DEF(vm_ids...) \
    VM_VIRTUAL_SERIAL_GENERAL_CONFIGURATION_DEF() \
    __CALL(PER_VM_VIRTUAL_SERIAL_CONFIGURATION_DEF, vm_ids) \

