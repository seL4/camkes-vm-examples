/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */
#include "vchan_testsuite.h"

#include "includes/vmm_manager.h"
#include "includes/vchan_copy.h"
#include "includes/libvchan.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>

static char test_run_arr[VCHANTESTS_MAX_TESTS] = { 0 };
const char testsuite_usage[] = { "sel4_vchan_testsuite ([0..VCHAN_TESTSUITE_MAX_TESTS] )+" };

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
	TESTSUITE: Tests, linux userlevel side
*/

/* Fundamental vchan handshake test */
int vchan_tests_handshake(libvchan_t *ctrl) {
	TDPRINTF(2, "start handshake to component\n");
	int handshake = VCHANTESTS_HANDSHAKE_CODE;

	libvchan_send(ctrl, &handshake, sizeof(int));
    TDPRINTF(2, "wait for ack\n");
	libvchan_wait(ctrl);
    libvchan_recv(ctrl, &handshake, sizeof(int));
    if(handshake != VCHANTESTS_HANDSHAKE_ACK)
    	return -1;

    TDPRINTF(2, "handshake done\n");
	return 0;
}

/*
    vchantests_close_reopen:

*/
int vchantests_close_reopen(libvchan_t *ctrl) {
    int res, ack = 1;

    libvchan_close(ctrl);
    ctrl = libvchan_client_init(50, VCHANTESTS_VCHAN_PORT);
    if(ctrl == NULL) {
        TERROR("close-reopen: failed to create connection")
        return -1;
    }

    res = libvchan_send(ctrl, &ack, sizeof(int));
    if(res != sizeof(int)) {
        TERROR("close-reopen: got %d|%d on send\n", res, (int) sizeof(int));
        return -1;
    }

    res = libvchan_recv(ctrl, &ack, sizeof(int));
    if(res != sizeof(int) || ack != 0) {
        TERROR("close-reopen: got %d|%d on recv\n", res, (int) sizeof(int));
        return -1;
    }

    TDPRINTF(4, "close-reopen: done\n");
    return 0;
}

/*
    vchantests_close:
*/
int vchantests_close(libvchan_t *ctrl) {


    return 0;
}

/*
    vchantests_bigwrite - vchantests_bigwrite_child:
        Open up new vchan and use it to read back file as it is being sent
        Read back data gets placed into a file
*/
static int vchantests_bigwrite_child(libvchan_t *ctrl, size_t sz) {
	int ret = 0, fd;
    char rbuf[VM_BIGWRITE_COMP_BUF_SIZE];

    fd = open(bigwrite_out, 0 | O_CREAT | O_TRUNC | O_RDWR, 0 | S_IRUSR | S_IWUSR);
    if(fd < 0) {
    	TERROR("bigwrite-child: failed to open output file '%s'", bigwrite_out);
    	return EXIT_FAILURE;
    }

	libvchan_t *writeback = libvchan_client_init(50, VM_BIGWRITE_PORT);
	if(writeback == NULL) {
		TERROR("bigwrite-child: failed to initialise readback channel\n");
		return EXIT_FAILURE;
	}

	while(sz > 0) {
    	TDPRINTF(4, "bigwrite (child): want %d;%d bytes from client\n", ret, (int) sz);
		ret = libvchan_read(writeback, rbuf, VM_BIGWRITE_COMP_BUF_SIZE);
		if(ret < 0) {
			TERROR("bigwrite-child: failed to read from readback channel\n");
			return EXIT_FAILURE;
		}
    	TDPRINTF(4, "bigwrite (child): got %d bytes from client\n", (int) ret);

    	if(write(fd, rbuf, ret) < 0) {
    		TERROR("bigwrite-child: error writing to output file\n");
			return EXIT_FAILURE;
    	}

		if(sz < VM_BIGWRITE_COMP_BUF_SIZE)
			sz = 0;
		else
			sz -= ret;
	}

    libvchan_close(writeback);

	return EXIT_SUCCESS;
}

/*
    vchantests_bigwrite:
        Open up a large file, and load in its data
        We then fork a parent and child
            Parent:
                Send file data to server
            Child:
                Readback file data from server and write it to a file
        When child and parent are finished, we compare file and readback file
        Tests simple blocking and multiple active vchan simples
*/
int vchantests_bigwrite(libvchan_t *ctrl) {
    int fd, ret;
    char compare_command[bigwrite_cmd_size];
    pid_t child_pid;
    size_t sz;

    struct stat buf;
    char *wbuf = NULL;

    fd = open(bigwrite_file, O_RDWR);
    if(fd < 0) {
    	TERROR("bigwrite: failed to open test file '%s'\n", bigwrite_file);
    	return -1;
    }

    fstat(fd, &buf);
    sz = (size_t) buf.st_size;
    libvchan_send(ctrl, &sz, sizeof(size_t));

    wbuf = malloc(sz * sizeof(char));
    ret = read(fd, wbuf, sz);
    if(ret < 0) {
    	TERROR("bigwrite: could not read from file\n");
    	return -1;
    }

    child_pid = fork();
    if(child_pid < 0) {
    	TERROR("bigwrite: could not fork thread for testing\n");
    	return -1;
    } else if(child_pid == 0) {
    	int rcode = vchantests_bigwrite_child(ctrl, (size_t) buf.st_size);
	    exit(rcode);
    }

    TDPRINTF(4, "bigwrite (parent): sending %d bytes to client\n", (int) sz);
    ret = libvchan_send(ctrl, wbuf, sz);
    if(ret < 0 || ret != sz)  {
    	TERROR("bigwrite-parent: failed to send data, %d|%d", ret, (int) sz);
    }

    TDPRINTF(4, "bigwrite (parent): wait for child\n");
    if(waitpid(child_pid, &ret, 0) == -1) {
    	TERROR("bigwrite-parent: failed to wait for child");
    	return -1;
    }

    if(WIFEXITED(ret)) {
    	if(WEXITSTATUS(ret) == EXIT_FAILURE) {
    		TERROR("bigwrite-parent: child failed somewhere..\n");
    		return -1;
    	}
    }

    ret = snprintf(compare_command, bigwrite_cmd_size, "diff %s %s", bigwrite_file, bigwrite_out);
    if(ret < 0) {
        TERROR("bigwrite-parent: failed to snprintf compare command\n");
    }

    TDPRINTF(2, "Running '%s' to compare files\n", compare_command);

    /* Check that the files are the same with diff */
    ret = system(compare_command);
    if(ret != EXIT_SUCCESS) {
    	TERROR("bigwrite-parent: original and readback file failed diff\n");
    	return -1;
    }

    TDPRINTF(2, "bigwrite-parent: concluded\n");
    return 0;
}

/*
    vchantests_funnel_verify_buffer:
        Check that buffer that was sent back matches the original data
*/
static int vchantests_funnel_verify_buffer(char *buf, int sz) {
	char c = 0;
	int count;
	int failed = 0;
	for(count = 0; count < sz; count++) {
		if(buf[count] != c) {
			TERROR("funnel: buffer mismatch :expected = %c|actual = %c: \n", c, buf[count]);
			failed = -1;
		}
		c++;
	}
	return failed;
}

/*
    vchantests_funnel:
        Send an entire buffer in chunks
        Wait for that buffer to be sent back (sent back in chunks)
        Primarily testing correct blocking behaviour
*/
int vchantests_funnel(libvchan_t *ctrl) {
	int res, count = 0;
	char *buffer;
	int sz = libvchan_buffer_space(ctrl);
	if(sz <= 0) {
		TERROR("funnel: invalid bspace precondition");
		return -1;
	}

	TDPRINTF(2, "funnel: starting with %d\n", sz);
	libvchan_write(ctrl, &sz, sizeof(int));

	if(sz > 0) {
		buffer = malloc(sz);
		char c = 0;
		while(count < sz) {
			res = libvchan_write(ctrl, &c, sizeof(char));
			if(res < 0)
				TERROR("funnel: failed to perform write");

			buffer[count] = c;
			count++;
			c++;
		}

		TDPRINTF(2, "funnel: waiting for writeback\n");
    	libvchan_recv(ctrl, buffer, sz);
    	if(vchantests_funnel_verify_buffer(buffer, sz) < 0) {
    		TERROR("funnel: failed on buffer getback stage!\n");
    		return -1;
    	}
	} else {
		TERROR("funnel: incorrect precondtion for funnel start\n");
	}

	TDPRINTF(2, "funnel: finished on clientside\n");

	return 0;
}

/*
    vchantests_vm_burst_verify:
		Verify that a given chunk of data is correct
*/
static int vchantests_vm_burst_verify(int key, int *buf, int sz) {
    int i;
    for(i = 0; i < sz; i++) {
        if(buf[i] != (i + key)) {
            TERROR("vm-burst: buffer mismatch :expected at %d = %x|actual = %x: \n", i, i + key, buf[i]);
            return -1;
        }
    }
    return 0;
}

static int valid_key(int key, const int *keylist, int sz) {
    int i;
    for(i = 0; i < sz; i++) {
        if(key == keylist[i])
            return 1;
    }

    return 0;
}

/*
    vchantests_vm_burst:
		Recieve big chunks of data from the component, sleep intermittently
		Check that the chunks recieved are correct
*/
int vchantests_vm_burst(libvchan_t *ctrl) {
	int key, rsize, sleep_count, x, res = 0;
    int total_int_num;
	int buf[VM_BURST_VM_SZ];

    TDPRINTF(2, "vm-burst: start\n");

    for(x = 0; x < VM_BURST_NUM_SENDS; x++) {
        sleep_count = 0;
        res = libvchan_recv(ctrl, &key, sizeof(int));
        if(res < 0) {
            TERROR("vm-burst: failed to get key\n");
            return -1;
        }

        if(!valid_key(key, vm_burst_base_nums, sizeof(vm_burst_base_nums) / sizeof(int))) {
            TERROR("vm-burst: key MISMATCH (got: %x)\n", key);
            return -1;
        }

        TDPRINTF(2, "vm-burst: key = %x\n", key);
        rsize = VM_BURST_CHUNK_INTS * sizeof(int);
        total_int_num = 0;
        TDPRINTF(2, "vm-burst: going to read\n");

        while(rsize > 0) {
            if(sleep_count < VM_BURST_NUM_SLEEPS) {
                sleep_count++;
                sleep(VM_BURST_SLEEP_TIME);
            }

            TDPRINTF(4, "vm-burst: %d bytes remaining\n", rsize);
            int rval = MIN(rsize, sizeof(buf));
            res = libvchan_recv(ctrl, buf, rval);
            TDPRINTF(4, "vm-burst: returned with %d|%d bytes\n", res, rsize);
            if(res != rval) {
                TERROR("vm-burst: did not get correct ret\n");
                return -1;
            }

            rsize -= res;
            if(vchantests_vm_burst_verify(key + total_int_num, buf, (res / sizeof(int))) < 0) {
                TERROR("vm-burst: failed data check %d\n", total_int_num);
            	return -1;
            } else {
                TDPRINTF(2, "vm-burst: passed data check %d\n", total_int_num);
            }
            total_int_num += res / sizeof(int);
        }

        res = VM_BURST_CHECKSUM;
        libvchan_send(ctrl, &res, sizeof(int));
    }

	return 0;
}

/*
    vchantests_prod_cons:
        Simple buffer reading and writing test, similar to packet test
*/
int vchantests_prod_cons(libvchan_t *ctrl) {
	int x, i, t, ret;
	int chunk[PROD_CONS_CHUNK];

	TDPRINTF(2, "prodcons: start\n");
	for(x = 0; x < PROD_CONS_NUM; x++) {
		for(i = 0; i < PROD_CONS_CHUNK; i++) {
			chunk[i] = x + i;
		}
		ret = libvchan_send(ctrl, chunk, PROD_CONS_CHUNK * sizeof(int));
		if(ret != PROD_CONS_CHUNK * sizeof(int)) {
			TERROR("prodcons: failed to write chunk")
			return -1;
		}

		t = libvchan_buffer_space(ctrl);
		TDPRINTF(4, "prodcons: has buffer space left: %d\n", t);
	}

	TDPRINTF(4, "prodcons: wait for ack\n");
	ret = libvchan_recv(ctrl, &x, sizeof(int));
	if(x != 1 || ret != sizeof(int)) {
		TERROR("prodcons: bad ack\n");
		return -1;
	}
	TDPRINTF(2, "prodcons: concluded\n");

	return 0;
}


/*
    vchantests_packet:
		Send a number of packets to the test component, then wait for an ack
		test component checks for incorrect data
*/
int vchantests_packet(libvchan_t *ctrl) {
	size_t sz;
	vchan_packet_t pak;
	int x, i, num_packets = PACKET_TEST_NUM_PACKETS;
	char fnack;

	TDPRINTF(2, "packets: start\n");
	/* Check that buffer data is correct */
	sz = libvchan_data_ready(ctrl);
	if(sz != 0) {
		TERROR("incorrect start packet buffer size (data ready) %d\n", (int) sz);
		return -1;
	}

	sz = libvchan_buffer_space(ctrl);
	if(sz != FILE_DATAPORT_MAX_SIZE) {
		TERROR("error: incorrect start packet buffer size (bspace) %d\n", (int) sz);
		return -1;
	}

	/* Start */
	sz = libvchan_send(ctrl, &num_packets, sizeof(int));
	if(sz < sizeof(int)) {
		TERROR("bad packet send size\n");
		return -1;
	}

	TDPRINTF(4, "packets: send packets\n");

	for(x = 0; x < num_packets; x++) {
		pak.pnum = x;
		for(i = 0; i < 4; i++) {
			pak.datah[i] = i + x;
		}
        pak.guard = PACKET_TEST_GUARD;

		sz = libvchan_send(ctrl, &pak, sizeof(pak));
		if(sz != sizeof(pak)) {
			TERROR("bad packet send size\n");
			return -1;
		}
	}

	TDPRINTF(4, "packets: waiting for ack..\n");

	libvchan_wait(ctrl);
	sz = libvchan_read(ctrl, &fnack, sizeof(char));
	if(sz < sizeof(char) || ! fnack) {
		TERROR("bad ack for packets %d\n", (int) fnack);
		return -1;
	}

	TDPRINTF(2, "packets: finished\n");
	return 0;
}

/*
	TESTSUITE: Program
*/

/* Usage of test binary */
static void usage(int do_exit) {
	printf("%s\n", testsuite_usage);
	printf("Number of tests = %d\n", VCHANTESTS_MAX_TESTS);
	if(do_exit)
		exit(1);
}

/*
    verify_ctrl_correctness:
        Make sure that ctrl has no data to read, or no data is yet to be read
*/
int verify_ctrl_correctness(libvchan_t *ctrl) {
	int dready = libvchan_data_ready(ctrl);
	int bspace = libvchan_buffer_space(ctrl);
	if(bspace != FILE_DATAPORT_MAX_SIZE) {
		TERROR("Bad ctrl buffer after test, got %d (want %d)", bspace, FILE_DATAPORT_MAX_SIZE);
		return -1;
	}

	if(dready != 0) {
		TERROR("Bad ctrl data ready after test, got %d (want %d)", dready, 0);
		return -1;
	}

	return 0;
}

/*
    For each test to be run, test cmd to testsuite camkes component
    	Then perform the test
*/
static int do_tests(libvchan_t *ctrl) {
	int i, errcount = 0, total = 0;
	for(i = 0; i < VCHANTESTS_MAX_TESTS; i++) {
		/* Perform test */
		if(test_run_arr[i]) {
			if(testop_table.tfunc[i] == NULL) {
				TDPRINTF(5, "no test for id %d! Has a test been properly connected?\n", i);
			} else {
                printf("Running test:%d\n", i);
				/* Send test command to vchan component so it knows what to do */
				int tcmd = i;
				libvchan_write(ctrl, &tcmd, sizeof(int));
				/* if non-zero result returned from test, it must have failed */
				tcmd = abs((*testop_table.tfunc[i])(ctrl));
				if(tcmd) {
					printf("Test %d FAILED\n", i);
				} else {
                    printf("Test %d PASSED\n", i);
                }

				errcount += tcmd;
				total++;
			}
		}

		/*
			If the ctrl vchan channel is not correct at this stage, all other tests will fail
				return here to prevent noisy errors
		*/
		if(verify_ctrl_correctness(ctrl) < 0) {
			TERROR("Test %d broke the control vchan channel\n", i);
			return i;
		}
	}

	printf("Testsuite: %d out of %d tests PASSED\n", (total - errcount), total);
	return errcount;
}

/*
    Testsuite entry point
*/
int main(int argc, char **argv) {
	int i, errors, close_code = VCHAN_TESTSUITE_CLOSED;
    if(argc < 2) {
        TERROR("Invalid number of arguments for testsuite, got %d\n", argc);
        usage(1);
    }

	/* Create connection and do sanity test */
	libvchan_t *ctrl = libvchan_client_init(50, VCHANTESTS_VCHAN_PORT);
	if(ctrl == NULL) {
		TERROR("Failed to create control connection");
        return EXIT_FAILURE;
    }

	if(vchan_tests_handshake(ctrl) < 0) {
		TERROR("Failed to perform initial handshake");
        return EXIT_FAILURE;
    }

	/* Set up tests to run from given arguments */
	for(i = 1; i < argc; i++) {
		int anum = atoi(argv[i]);
		if(anum < 0) {
			TDPRINTF(2, "Running all tests\n");
			memset(test_run_arr, 1, sizeof(test_run_arr));
			break;
		} else if(anum < VCHANTESTS_MAX_TESTS) {
			test_run_arr[anum] = 1;
		} else {
			TDPRINTF(2, "Unknown testarg num: %d\n", anum);
		}
	}

	errors = do_tests(ctrl);

	TDPRINTF(4, "closing vchan ctrl\n");
    libvchan_send(ctrl, &close_code, sizeof(int));
	libvchan_close(ctrl);

	if(errors) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
