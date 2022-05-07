/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*
    
    
    Usage:
      ur3e.exe [read|write ip-address channel(0-7) high|low]
                    Modbus client for universal-robots UR3E

                    (input) command line params (input):
                            [read|write ip-address channel(0-7) high|low]

                    (output) exit status of program:
                            write  -1 -> ERROR   0 -> OK
                            read   -1 -> ERROR   0 -> bit is low    1 -> bit is high

    Examples:
    
        # reading at 127.0.0.1 from channel 0          -> ERROR

        $ ./ur3e read 127.0.0.1 0
            Connecting to 127.0.0.1:502
            Connection failed: No error
    
        $ echo $?
            127
    
        # reading at 127.0.0.1 from channel 0 bit 0x00 -> low
        
        $ ./ur3e read 127.0.0.1 0
            Connecting to 127.0.0.1:502
            [00][01][00][00][00][06][FF][01][00][10][00][01]
            Waiting for a confirmation...
            <00><01><00><00><00><04><FF><01><01><00>

        $ echo $?
            0

        # reading at 127.0.0.1 from channel 0 bit 0x01 -> high

        $ ./ur3e read 127.0.0.1 0
            Connecting to 127.0.0.1:502
            [00][01][00][00][00][06][FF][01][00][10][00][01]
            Waiting for a confirmation...
            <00><01><00><00><00><04><FF><01><01><01>

        $ echo $?
            1

        # writing at 127.0.0.1 to channel 0 bit 0x01 -> ERROR

        $ ./ur3e write 127.0.0.1 0 high
            Connecting to 127.0.0.1:502
            Connection failed: No error

        $ echo $?
            127

        # writing at 127.0.0.1 to channel 0 bit 0x01 -> high

        $ ./ur3e write 127.0.0.1 0 high
            Connecting to 127.0.0.1:502
            [00][01][00][00][00][06][FF][05][00][10][FF][00]
            Waiting for a confirmation...
            <00><01><00><00><00><06><FF><05><00><10><FF><00>
    
        $ echo $?
            0

        # writing at 127.0.0.1 to channel 0 bit 0x00 -> low

        $ ./ur3e write 127.0.0.1 0 low
            Connecting to 127.0.0.1:502
            [00][01][00][00][00][06][FF][05][00][10][00][00]
            Waiting for a confirmation...
            <00><01><00><00><00><06><FF><05><00><10><00><00>

        $ echo $?
            0


    Links:

        https://www.universal-robots.com/articles/ur/interface-communication/modbus-tcp-client-setup/
        https://www.universal-robots.com/articles/ur/interface-communication/modbus-communication-16357/
        https://www.universal-robots.com/articles/ur/interface-communication/modbus-server/
        https://s3-eu-west-1.amazonaws.com/ur-support-site/16377/ModBus%20server%20data.pdf
        
        https://www.modbus.org/docs/PI_MBUS_300.pdf

        https://www.wireshark.org/docs/dfref/m/mbtcp.html
        https://www.linkedin.com/pulse/how-read-modbus-protocol-using-wireshark-matthew-loong

        https://www.vanimpe.eu/2015/12/07/introduction-to-modbus-tcp-traffic/
            In Wireshark I filter the traffic to Modbus only with
                tcp.port == 502

        https://www.linkedin.com/pulse/how-read-modbus-protocol-using-wireshark-matthew-loong

        // Modbus function codes 
        #define MODBUS_FC_READ_COILS                0x01
        #define MODBUS_FC_READ_DISCRETE_INPUTS      0x02
        #define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
        #define MODBUS_FC_READ_INPUT_REGISTERS      0x04
        #define MODBUS_FC_WRITE_SINGLE_COIL         0x05
        #define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06
        #define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
        #define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
        #define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
        #define MODBUS_FC_REPORT_SLAVE_ID           0x11
        #define MODBUS_FC_MASK_WRITE_REGISTER       0x16
        #define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17

        https://control.com/forums/threads/modbus-coil-and-register-question.31155/

        https://www.universal-robots.com/articles/ur/interface-communication/connecting-internal-inputs-and-outputs-io-on-the-robots-controller/

*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#include "ur3e.h"


enum {
    READ_SINGLE_COIL,
    WRITE_SINGLE_COIL
};

void print_usage_and_exit(void);

#define BUG_REPORT(_cond, _format, _args ...) \
    printf("\nLine %d: assertion error for '%s': " _format "\n", __LINE__, # _cond, ## _args)

#define ASSERT_TRUE(_cond, _format, __args...) {  \
    if (_cond) {                                  \
        /*printf("OK\n");*/                       \
    } else {                                      \
        BUG_REPORT(_cond, _format, ## __args);    \
        goto close;                               \
    }                                             \
};

void print_usage_and_exit(void) {
    
    printf("Usage:\n  ur3e.exe [read|write ip-address channel(0-7) high|low] \n");
    printf("\t\tModbus client for universal-robots UR3E\n\n");
    printf("\t\t(input) command line params (input):\n");
    printf("\t\t\t[read|write ip-address channel(0-7) high|low]\n\n");
    printf("\t\t(output) exit status of program:\n");
    printf("\t\t\twrite  -1 -> ERROR   0 -> OK  \n");
    printf("\t\t\tread   -1 -> ERROR   0 -> bit is low    1 -> bit is high  \n\n\n");
    exit(-1);
    
}


int main(int argc, char *argv[]) {
    
    uint8_t *tab_rp_bits = NULL;
    uint16_t *tab_rp_registers = NULL;
    modbus_t *ctx = NULL;
    int nb_points;
    int rc;
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;

    /* modbus interface command line interface */
    int   modbus_cmd = READ_SINGLE_COIL;
    char* modbus_ip_address = NULL;
    int   modbus_channel = 0;
    int   modbus_bit     = FALSE;
    int   modbus_ret_val = 0;

    if (argc >= 4 ) {
        
        if (strcmp(argv[1], "read") == 0) {
            modbus_cmd = READ_SINGLE_COIL;
            
        } else if (strcmp(argv[1], "write") == 0) {
            modbus_cmd = WRITE_SINGLE_COIL;
            
        } else {
            printf("Usage:\n  %s [read|write ip-address channel(0-7) high|low] - Modbus client for universal-robots UR3E\n\n", argv[0]);
            exit(-1);
            
        }
        
        modbus_ip_address = argv[2];
     
        /*   
        #  https://www.universal-robots.com/articles/ur/interface-communication/modbus-server/
        #  https://s3-eu-west-1.amazonaws.com/ur-support-site/16377/ModBus%20server%20data.pdf
        #  test write single coil and read single coil for 
        #  digital output 0 - 7 starting at address 16 for universal robots:
        # 16-31 x x x * * Outputs, bits 0-15 [BBBBBBBBTTxxxxxx] x=undef, T=tool, B=box
        */
        
        modbus_channel = atoi(argv[3]);
        if( modbus_channel < 0 || modbus_channel > 7 ) {
            print_usage_and_exit();
            
        }
        modbus_channel += 16;              // single write coil output starts here at address 16
        
        if( modbus_cmd == WRITE_SINGLE_COIL ) {
            if( argc == 5 ) {

                if (strcmp(argv[4], "high") == 0) {

                    modbus_bit = TRUE;                    
                    
                } else if (strcmp(argv[4], "low") == 0) {

                    modbus_bit = FALSE;                    
                    
                } else {
                    print_usage_and_exit();
                }

            } else {
                print_usage_and_exit();
            }
        } // if( modbus_cmd == WRITE_SINGLE_COIL )
        
    } else {
        print_usage_and_exit();
    } // if (argc >= 4 )
    

    ctx = modbus_new_tcp(modbus_ip_address, 502);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);


    modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the bits */
    nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
    tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    
    /** COIL BITS **/
    /* Single read or write */

    if ( READ_SINGLE_COIL == modbus_cmd ) {

        rc = modbus_read_bits(ctx, modbus_channel, 1, tab_rp_bits);
        ASSERT_TRUE(rc == 1, "FAILED (nb points %d)\n", rc);
        modbus_ret_val = tab_rp_bits[0];

        
    } else if ( WRITE_SINGLE_COIL == modbus_cmd ) {

        rc = modbus_write_bit(ctx, modbus_channel, modbus_bit);
        ASSERT_TRUE(rc == 1, "");

    }
    
    close:
    
    /* Free the memory */
    free(tab_rp_bits);
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return modbus_ret_val;

} // int main(int argc, char *argv[])



