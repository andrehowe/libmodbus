# License
Test programs of this directory are provided under BSD license (see associated
LICENSE file).


# UR3E notes:

$ ./ur3e
    Usage:
      ur3e.exe [read|write ip-address channel(0-7) high|low]
                    Modbus client for universal-robots UR3E

                    (input) command line params (input):
                            [read|write ip-address channel(0-7) high|low]

                    (output) exit status of program:
                            write  -1 -> ERROR   0 -> OK
                            read   -1 -> ERROR   0 -> bit is low    1 -> bit is high


# UR3E examples:
    
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


# UR3E links:

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



# Legacy libmodbus

# Compilation
After installation, you can use pkg-config to compile these tests.
For example, to compile random-test-server run:

`gcc random-test-server.c -o random-test-server $(pkg-config --libs --cflags libmodbus)`

- `random-test-server` is necessary to launch a server before running
random-test-client. By default, it receives and replies to Modbus query on the
localhost and port 1502.

- `random-test-client` sends many different queries to a large range of
addresses and values to test the communication between the client and the
server.

- `unit-test-server` and `unit-test-client` run a full unit test suite. These
programs are essential to test the Modbus protocol implementation and libmodbus
behavior.

- `bandwidth-server-one`, `bandwidth-server-many-up` and `bandwidth-client`
 return very useful information about the performance of transfer rate between
 the server and the client. `bandwidth-server-one` can only handles one
 connection at once with a client whereas `bandwidth-server-many-up` opens a
 connection for each new clients (with a limit).
