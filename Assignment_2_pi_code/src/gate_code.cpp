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
#include <iomanip>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace std::chrono;
using namespace cv;
#define BUF_SIZE 100
Mat bgr_img;
Mat display_img;

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

int j = 0;
int driver_index = -1;
float sect1_time;
float sect2_time;
float sect3_time;
float laptime;
bool present_bool = false;
bool driver_bool = false;
bool finished_config = false;
bool update_bool = false;
bool sort_bool = false;
size_t len;

// this function handles how a gate functions, depending on its assigned gate id
bool gate_logic (int gate_id, bool start_to_finish) {
    switch (gate_id) 
        {
            case 1: // its the start/finish gate
                if (start_to_finish == true) { // if the gate is in its "finish" state
                    start_to_finish = false;
                    finish_gate = high_resolution_clock::now(); // get finish time
                    sector_3 = duration_cast<milliseconds>(finish_gate - third_gate); // calc the time of sector 3 
                    total_time = duration_cast<milliseconds>(finish_gate - start_gate); // calc final time 
                    
                    // Set sector times to time
                    sect3_time = (float)sector_3.count() / 1000; // type cast the var.count to int to use it elswhere otherwise will default to zero.
                    laptime = (float)total_time.count() / 1000;
                    
                    printf("Sector 3 Time: %f\n", sect3_time); 
                    printf("Total Time: %f\n", laptime);
                    printf("RACE FINISHED\n");
                    
                    // Enable to retrieve new driver name
                    driver_bool = true;
                    // Enable leaderboard presentation
                    present_bool = true;
                    // Enable sorting of array
                    sort_bool = true;
                }
                else if(start_to_finish == false){ // if the gate is in its "start" state
                    printf("RACE START\n"); 
                    start_gate = high_resolution_clock::now(); // save the starting time
                }
                break;
            case 2:// its the second gate
                second_gate = high_resolution_clock::now(); // get second gate time
                sector_1 = duration_cast<milliseconds>(second_gate - start_gate); //calc the time in sector 1
                
                 // Set sector times to time
                sect1_time = (float)sector_1.count() / 1000;
                printf("Sector 1 Time: %f\n", sect1_time);
               
                break;
            case 3: // its the third gate
                start_to_finish = true; //set the start gate to finish
                third_gate = high_resolution_clock::now(); //get the third gate time
                sector_2 = duration_cast<milliseconds>(third_gate - second_gate); // calc the time in sector 2
                
                // Set sector times to time
                sect2_time = (float)sector_2.count() / 1000;
                printf("Sector 2 Time: %f\n", sect2_time);
                
                break;
            default:
                break;

        }
       
        return start_to_finish; // return the value of start_to_finish
}

// the following 3 buffers could be combined to 1 but i honeslty could figure it out and i wanted to go home so as it stands there are 3
// please note to avoid weird errors with the buffers the 3 ports requires 3 indivigual buffers 

int process_first_buffer(char string_data, size_t bytes_read) {// handles the first ports buffer 
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
int process_second_buffer(char string_data, size_t bytes_read) {
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
int process_third_buffer(char string_data, size_t bytes_read) {
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
        third_serial_buf[third_serial_buf_id++] = string_data;
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

// Create Driver class
class Driver {
   
    public:
    string driver_name;
    float sector1_time;
    float sector2_time;
    float sector3_time;
    float lap_time;
    int index;
    // New driver initliasation
    Driver(string in_driver_name)
        :driver_name(in_driver_name)
        ,sector1_time(0.0f)
        ,sector2_time(0.0f)
        ,sector3_time(0.0f)
        ,lap_time(0.0f)
        {}
    bool operator < (const Driver other_driver) const {return lap_time < other_driver.lap_time;} // sorting
};  

void update_stats(vector<Driver> &drivers, float sect1_time, float sect2_time, float sect3_time, float laptime, int index) {
    
    // Update drivers stats
    drivers[index].sector1_time = sect1_time;
    drivers[index].sector2_time = sect2_time;
    drivers[index].sector3_time = sect3_time;
    drivers[index].lap_time = laptime;

}

void update_name(vector<Driver> &drivers, string name, int index) {
    
    // Update drivers 
    drivers[index].driver_name = name;
    
}

// Function to convert float to specified decimal place
std::string to_string_with_precision(const float a_value, const int n = 6) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

// Present leaderboard function
void present_leaderboard(vector<Driver> drivers, bool present, bool sort_drivers) {
    
    if (present == true) {
        
        // Clone the original image to remove old text
        display_img = bgr_img.clone();
        cv::imshow("Leaderboard", display_img);
        cv::waitKey(1000);
        
        // Sort drivers
        sort(drivers.begin(), drivers.end());
        

        // Put text variables
        int column1_pos = 175;
        int column2_pos = 350;
        int column3_pos = 525;
        int column4_pos = 710;
        int column5_pos = 900;
        int y_pos = 350;

        // Loop the driver array and put text
        int num_drivers = 0;
        for (Driver driver : drivers) {
            if (num_drivers >= 10) { // Only display the top 10 drivers
                break;
            }

            // Convert times to strings with 3 decimal places
            string sect1_str = to_string_with_precision(driver.sector1_time, 3);
            string sect2_str = to_string_with_precision(driver.sector2_time, 3);
            string sect3_str = to_string_with_precision(driver.sector3_time, 3);
            string lapt_str = to_string_with_precision(driver.lap_time, 3);
            
            // Put driver name
            cv::putText(display_img,
                        driver.driver_name,
                        cv::Point(column1_pos, y_pos), //text position
                        cv::FONT_HERSHEY_DUPLEX,
                        1.0, // font scale
                        CV_RGB(255, 255, 255), //font color
                        2);

            // Put sector 1 time
            cv::putText(display_img,
                        sect1_str,
                        cv::Point(column2_pos, y_pos), //text position
                        cv::FONT_HERSHEY_DUPLEX,
                        1.0, // font scale
                        CV_RGB(255, 255, 255), //font color
                        2);
            // Put sector 2 time
            cv::putText(display_img,
                        sect2_str,
                        cv::Point(column3_pos, y_pos), //text position
                        cv::FONT_HERSHEY_DUPLEX,
                        1.0, // font scale
                        CV_RGB(255, 255, 255), //font color
                        2);
            
            // Put sector 3 time
            cv::putText(display_img,
                        sect3_str,
                        cv::Point(column4_pos, y_pos), //text position
                        cv::FONT_HERSHEY_DUPLEX,
                        1.0, // font scale
                        CV_RGB(255, 255, 255), //font color
                        2);
            
            // Put total laptime
            cv::putText(display_img,
                        lapt_str,
                        cv::Point(column5_pos, y_pos), //text position
                        cv::FONT_HERSHEY_DUPLEX,
                        1.0, // font scale
                        CV_RGB(255, 255, 255), //font color
                        2);
            
            y_pos = y_pos + 60; // Iterate y position
            num_drivers++; // iterate displayed drivers
        }
        cv::imshow("Leaderboard", display_img); //show the leaderboard
        
        // Allow openCV to process GUI events
        cv::waitKey(1000);

    }
}

// Functio to ask for next drivers name
void get_name(bool driver_bool) {

    if (driver_bool == true) {
        printf("\nPlease enter next drivers name:\n");
    }
}

int main(int argc, char *argv[])  {
    
    vector<Driver> drivers; // initiliase a vector of the Driver class
    
    // Load the LeaderBoard image
	bgr_img = imread("../leaderboard.jpg");
	if (!bgr_img.data) {
		bgr_img = imread("leaderboard.jpg");
		if (!bgr_img.data) {
			std::cerr << "Failed to load image\n";
			return 1;
		}
	}

    // not a bug, its a feature :)
    cout << "Please configure all gates by moving your hand through each gate :)" << endl;

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
        {
            // monitor the terminal
            .fd = STDIN_FILENO, //terminal uses stad input
            .events = POLLIN | POLLERR, //TODO change these (?)
        },  

    };

    static char user_msg [512];
    // Variables for driver stats
    for (;;) {
        
        // Wait for events
        poll(pfds, sizeof(pfds)/sizeof(struct pollfd), -1);
        // Check if a a port has sent info arrived

        // tests if all gate are configd, currently only test ptt0/1, replace if test stamtent with commented code to test all
        if ((port_ttyACM0_state == configd) && (port_ttyACM1_state == configd) && (port_ttyACM2_state == configd) && (all_ports_state == not_configd)) // (port_ttyACM0_state == configd) && (port_ttyACM1_state == configd) && (port_ttyACM2_state == configd)
        {
            all_ports_state = configd;
            // Since all gates are now configured, ask for first drivers name.
            // This will never be accesed again
            // Get first drivers name
            printf("\nPlease enter drivers name:\n");
            finished_config = true; // change conditional boolean to recieve user message below
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
                port_ttyACM2_state = configd;
                
                if (bytes_read == 0) 
                {
                    continue;
                } 
                int discard_return = process_third_buffer(string_data, bytes_read);
            }
            else if(all_ports_state == configd)
            {
                // printf("tty2\n");
                char string_data;
                size_t bytes_read = read(port_ttyACM2, &string_data, 1);
                if (bytes_read == 0) 
                {
                    continue;
                } 
                gate_id = process_third_buffer(string_data, bytes_read);
            }
        
        }
        
        if (pfds[3].revents) {
            
            if (finished_config == true) {
            
                // Read the incoming message from terminal
                ssize_t bytes_read = read(STDIN_FILENO, user_msg, sizeof(user_msg) - 1); // with room for a trailing null
                
                if (bytes_read < 0) {
                    return 0;
                }
                
                driver_index++; // Update the driver index
                // Make the message null terminated
                user_msg[bytes_read] = 0;
                // Get size of message
                size_t drivername_length = strlen(user_msg);
                // Print welcome message
                cout << "\nGoodluck for your race " << user_msg << endl;
                // Change presentation conditional boolean
                present_bool = true;
                // Conver user input to string
                string recieved_drivername;
                recieved_drivername.assign(user_msg, user_msg + (drivername_length - 1));
                // Initialise a new driver instance
                drivers.push_back(Driver(recieved_drivername));
            }
            present_bool = false; // disable presenting leaderboard
        }
        
        // functionality of each gate is here! //TODO TURN INTO FUNCTION
        start_to_finish = gate_logic(gate_id, start_to_finish);

        // Update stats once driver is finished race
        if (driver_index != -1 ){
            update_stats(drivers, sect1_time, sect2_time, sect3_time, laptime, driver_index); // Update driver stats
        }

        //Present leaderboard
        present_leaderboard(drivers, present_bool, sort_bool);
        // disable presenting leaderboard
        present_bool = false;
        // Cal function to ask for next drivers name
        get_name(driver_bool);
        // disable printing of get_name
        driver_bool = false;

        gate_id = 0;
    }
}


