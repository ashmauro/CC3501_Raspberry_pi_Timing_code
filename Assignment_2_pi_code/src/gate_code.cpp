#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <poll.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include<vector>
#include <algorithm>

using namespace std;
using namespace std::chrono;
#define BUF_SIZE 100

//fun fact run ls /dev/tty* on the pi console to see connected ports, looking for ttyACM0/1/2
//TODO pop out window
//TODO i2c colour picker

// gobal vars used in the functions
int gate_id; //used ot assign gate functions
bool start_to_finish = false; // used to switch the function of the starting gate to a finish line

// values used to record timing CURRENTLY used in the internal functions
auto start_gate = high_resolution_clock::now();
auto second_gate = high_resolution_clock::now();
auto third_gate = high_resolution_clock::now();
auto finish_gate = high_resolution_clock::now();

// vars used for timing math
auto sector_1 = duration_cast<milliseconds>(second_gate - start_gate);
auto sector_2 = duration_cast<milliseconds>(third_gate - second_gate);
auto sector_3 = duration_cast<milliseconds>(finish_gate - third_gate);
auto total_time = duration_cast<milliseconds>(finish_gate - start_gate);

// first ports buffer and index
char first_serial_buf [BUF_SIZE];
int first_serial_buf_id = 0; 

// seconds ports buffer and index
char second_serial_buf [BUF_SIZE];
int second_serial_buf_id = 0; 

// third ports buffer and index
char third_serial_buf [BUF_SIZE];
int third_serial_buf_id = 0;


// this function handles how a gate functions, depending on its assigned gate id
bool gate_logic (int gate_id, bool start_to_finish){
    switch (gate_id) 
        {
            case 1: // its the start/finish gate
                if (start_to_finish == true) { // if the gate is in its "finish" state
                    start_to_finish = false;
                    finish_gate = high_resolution_clock::now(); // get finish time
                    sector_3 = duration_cast<milliseconds>(finish_gate - third_gate); // calc the time of sector 3 
                    total_time = duration_cast<milliseconds>(finish_gate - start_gate); // calc final time 
                    printf("Sector 3 Time: %d\n", (int)sector_3.count()); // type cast the var.count to int to use it elswhere otherwise will default to zero.
                    printf("Total Time: %d\n", (int)total_time.count());
                    printf("Race Ended\n");
                }
                else if(start_to_finish == false){ // if the gate is in its "start" state
                    printf("RACE START\n"); 
                    start_gate = high_resolution_clock::now(); // save the starting time
                }
                break;
            case 2:// its the second gate
                second_gate = high_resolution_clock::now(); // get second gate time
                sector_1 = duration_cast<milliseconds>(second_gate - start_gate); //calc the time in sector 1
                printf("Sector 1 Time: %d\n", (int)sector_1.count());

                break;
            case 3: // its the third gate
                start_to_finish = true; //set the start gate to finish
                third_gate = high_resolution_clock::now(); //get the third gate time
                sector_2 = duration_cast<milliseconds>(third_gate - second_gate); // calc the time in sector 2
                printf("Sector 2 Time: %d\n", (int)sector_2.count());
                break;
            default:
                break;

        }
        return start_to_finish; // return the value of start_to_finish
}

// the following 3 buffers could be combined to 1 but i honeslty could figure it out and i wanted to go home so as it stands there are 3
// please note to avoid weird errors with the buffers the 3 ports requires 3 indivigual buffers 

int process_first_buffer(char string_data, size_t bytes_read) // handles the first ports buffer
{
    int gate_id_value; // note that i dont use the noraml gate_id, if it did it would overwrite gate id, when it isnt supposed to
    if (bytes_read != 1)  // idk what this check just rolling with it
    {
        printf("Failed to read from the serial port: \n%s\n", strerror(errno));
    }
    // Detect a newline
    if (string_data == '\n' || string_data == '\r') 
    {
        if (first_serial_buf_id > 0) 
        {
        first_serial_buf[first_serial_buf_id] = 0;
        first_serial_buf_id = 0;
        // buffer should have a full line of text, should be able to then edit the buffer outside of the function in order grabe sent 12c values
        }

    }
    // if no new end line detected assume the buffer isnt done
    else if (first_serial_buf_id < BUF_SIZE-1) 
    {   // -1 to allow room for the NULL
        // Store the result
        first_serial_buf[first_serial_buf_id++] = string_data;
        // test the buffer value to see if it contains an a, b or c, then change gate_id_value accordinly
        if (strcmp("a",first_serial_buf) == 0) 
        {
            gate_id_value = 1;
        }
        else if (strcmp("b",first_serial_buf) == 0)
        {
            gate_id_value = 2;
        }
        else if (strcmp("c",first_serial_buf) == 0)
        {
            gate_id_value = 3;
        }
    }
    return gate_id_value;
}

//essetially the same as the previous except should refer to second buffer
int process_second_buffer(char string_data, size_t bytes_read)
{
    int gate_id_value;
    if (bytes_read != 1) 
    {
        printf("Failed to read from the serial port: \n%s\n", strerror(errno));
    }

    if (string_data == '\n' || string_data == '\r') 
    {
        if (second_serial_buf_id > 0) 
        {
        second_serial_buf[second_serial_buf_id] = 0;
        second_serial_buf_id = 0;
        }

    }
    else if (second_serial_buf_id < BUF_SIZE-1) 
    {
        second_serial_buf[second_serial_buf_id++] = string_data;
        if (strcmp("a",second_serial_buf) == 0) 
        {
            gate_id_value = 1;
        }
        else if (strcmp("b",second_serial_buf) == 0)
        {
            gate_id_value = 2;
        }
        else if (strcmp("c",second_serial_buf) == 0)
        {
            gate_id_value = 3;
        }
    }
    return gate_id_value;
}

// same again except i havent tested this yet and should refer to the third
int process_third_buffer(char string_data, size_t bytes_read)
{
    int gate_id_value;
    if (bytes_read != 1) 
    {
        printf("Failed to read from the serial port: \n%s\n", strerror(errno));
    }
    // Detect a newline
    if (string_data == '\n' || string_data == '\r') 
    {
        if (third_serial_buf_id > 0) 
        {
        third_serial_buf[third_serial_buf_id] = 0;
        third_serial_buf_id = 0;
        /* buf now contains a complete line of text */
        //handle seperating the i2c values here
        }

    }
    else if (third_serial_buf_id < BUF_SIZE-1) 
    {   // -1 to allow room for the NULL
        // Store the result
        second_serial_buf[third_serial_buf_id++] = string_data;
        if (strcmp("a",third_serial_buf) == 0) 
        {
            gate_id_value = 1;
        }
        else if (strcmp("b",third_serial_buf) == 0)
        {
            gate_id_value = 2;
        }
        else if (strcmp("c",third_serial_buf) == 0)
        {
            gate_id_value = 3;
        }
    }
    return gate_id_value;
}

int main(int argc, char *argv[]) 
{
    // not a bug, its a feature :)
    printf("Please configure all gates by moving your hand through each gate :)\n"); 

    // sets values to check if all the gate are configured
    enum gate_state_t {not_configd = 0, configd = 1}; 
    gate_state_t port_ttyACM0_state = not_configd;
    gate_state_t port_ttyACM1_state = not_configd;
    gate_state_t port_ttyACM2_state = not_configd;
    gate_state_t all_ports_state = not_configd;

    // Prepare the pollfd array with the list of file handles to monitor
    // cable length may occasionally determine ttyACM0 order
    int port_ttyACM0 = open("/dev/ttyACM0", O_RDWR); // Blocking I/O
    int port_ttyACM1 = open("/dev/ttyACM1", O_RDWR); // Blocking I/O
    int port_ttyACM2 = open("/dev/ttyACM2", O_RDWR); // Blocking I/O

    // haven't touched this just works
    termios tio; // termios is a struct defined in termios.h
    memset(&tio, 0, sizeof(termios)); // Zero out the tio structure
    tio.c_cflag = CS8|CLOCAL|CREAD; // Select 8 data bits
    tio.c_cc[VMIN] = 1; // Always return at least one character
    cfsetospeed(&tio, B9600); // baud rate for output
    cfsetispeed(&tio, B9600); // baud rate for input
    tcsetattr(port_ttyACM0, TCSANOW, &tio); // Apply these settings
    tcsetattr(port_ttyACM1, TCSANOW, &tio); // Apply these settings
    tcsetattr(port_ttyACM2, TCSANOW, &tio); // Apply these settings

    // sets up the polls to be selected 
    struct pollfd pfds [] = {
        {
            // monitor the start gate
            .fd = port_ttyACM0, 
            .events = POLLIN | POLLERR,
        },
        {
            // monitor the middle gate
            .fd =  port_ttyACM1, //terminal uses stad input
            .events = POLLIN | POLLERR, 
        },
        {
            // monitor the end gate
            .fd =  port_ttyACM2, //terminal uses stad input
            .events = POLLIN | POLLERR, 
        },      

    };

    for (;;) 
    {
        // Wait for events
        poll(pfds, sizeof(pfds)/sizeof(struct pollfd), -1);
        // Check if a a port has sent info arrived

        // tests if all gate are configd, currently only test ptt0/1, replace if test stamtent with commented code to test all
        if ((port_ttyACM0_state == configd) && (port_ttyACM1_state == configd)) // (port_ttyACM0_state == configd) && (port_ttyACM1_state == configd) && (port_ttyACM2_state == configd)
        {
            all_ports_state = configd;
        }

        if (pfds[0].revents) // port tty0 gets an event 
        {
            if (port_ttyACM0_state == not_configd) // checks for configd status
            {
                //not configd
                char string_data; //NEEDED DO NOT MOVE 
                size_t bytes_read = read(port_ttyACM0, &string_data, 1); //NEEDED DO NOT MOVE
                printf("Gate configured\n");
                port_ttyACM0_state = configd; // set to configd
                if (bytes_read == 0) // somthing from the example code i got, just left it
                {
                    continue;
                } 
                int discard_return = process_first_buffer(string_data, bytes_read); // runs function to calibrate buffer, but return nothing 

            }
            else if(all_ports_state == configd)
            {
                //all gates are configd
                char string_data;//NEEDED DO NOT MOVE
                size_t bytes_read = read(port_ttyACM0, &string_data, 1);//NEEDED DO NOT MOVE
                if (bytes_read == 0) 
                {
                    continue;
                } 
                gate_id = process_first_buffer(string_data, bytes_read); // runs function to calibrate and return gate id
            }
        }

        if (pfds[1].revents) 
        { // TODO i2c

            if (port_ttyACM1_state == not_configd)
            {
                char string_data;
                size_t bytes_read = read(port_ttyACM1, &string_data, 1);
                printf("Gate configured\n");
                port_ttyACM1_state = configd;
                
                if (bytes_read == 0) 
                {
                    continue;
                } 
                int discard_return = process_second_buffer(string_data, bytes_read);
            }
            else if(all_ports_state == configd)
            {
                char string_data;
                size_t bytes_read = read(port_ttyACM1, &string_data, 1);
                // char string_data;
                // size_t bytes_read = read(port_ttyACM1, &string_data, 1);
                if (bytes_read == 0) 
                {
                    continue;
                } 
                gate_id = process_second_buffer(string_data, bytes_read);
            }           
        }

        if (pfds[2].revents) 
        { // TODO i2c
            if (port_ttyACM2_state == not_configd)
            {
                char string_data;
                size_t bytes_read = read(port_ttyACM2, &string_data, 1);
                printf("Gate configured\n");
                port_ttyACM1_state = configd;
                
                if (bytes_read == 0) 
                {
                    continue;
                } 
                int discard_return = process_third_buffer(string_data, bytes_read);
            }
            else if(all_ports_state == configd)
            {
                char string_data;
                size_t bytes_read = read(port_ttyACM2, &string_data, 1);
                // char string_data;
                // size_t bytes_read = read(port_ttyACM1, &string_data, 1);
                if (bytes_read == 0) 
                {
                    continue;
                } 
                gate_id = process_third_buffer(string_data, bytes_read);
            }           
        }
        
        // functionality of each gate is here! //TODO TURN INTO FUNCTION
        start_to_finish = gate_logic(gate_id, start_to_finish);

        gate_id = 0;
    }
}


