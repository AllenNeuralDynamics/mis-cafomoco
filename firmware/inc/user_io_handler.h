#ifndef USER_IO_HANDLER_H
#define USER_IO_HANDLER_H
#include <stdint.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <stdexcept>

#define NUM_BMCS (4)

enum Cmd
{
    IS_BUSY = 0,
    TIME_MOVE = 1,
    DIST_MOVE = 2,
    HOME = 3,
    HOME_IN_PLACE = 4,
    HOME_ALL = 5,
    HOME_ALL_IN_PLACE = 6,
    ERROR
};


// User should be able to invoke:
//    "TIME_MOVE 2,3 500,500"
// Result would move motors 2 and 3 for 500 [ms] each.
struct ParsedUserMsg
{
    Cmd cmd;
    // args
    int8_t motor_indexes[NUM_BMCS] {0, 1, 2, 3}; // -1 for invalid input.
    int8_t duty_cycles[NUM_BMCS] {0, 0, 0, 0}; // -1 for invalid input.
    // Other useful stuff about the message.
    uint8_t motor_count{4};
};


class UserIOHandler
{
public:
    /**
     * \brief constructor. Setup usb serial connection.
     */
    UserIOHandler();


    /**
     * \brief destructor.
     */
    ~UserIOHandler();


    /**
     * \brief connect to pc as usb serial. Block until connection is made.
     */
    void init();

    /**
     * \brief read in chars until none are present. Flag if a full msg arrived.
     * \note: this fn would not be needed and we could separate io behavior
     *        altogether if pico sdk provided access to the serial buffer
     *        directly and told us how many characters were in it.
     */
    void read_chars_nonblocking();


    /**
     * \brief handle user input and dispatch it.
     * \param input_str string contents terminated with "\r\n"
     */
    void handle_user_input();


    /**
     * \brief inline. retrieve the new message
     */
    ParsedUserMsg get_msg(){return parsed_msg_;}


    /**
     * \brief inline. clear the parsed message
     */
    void clear_msg(){new_msg_ = false; buff_index_ = 0;}


    /**
     * \brief inline. getter. True if new message has arrived.
     *        false otherwise.
     */
    bool new_msg(){return new_msg_;}


    /**
     * \brief inline. getter. True if new message has malformed contents.
     *        false otherwise.
     */
    bool msg_is_malformed(){return msg_is_malformed_;}


private:
    /**
     * \brief convert input string to cmd.
     */
    Cmd cmd_str_to_cmd(char* cmd_str);

    static const uint INPUT_BUF_SIZE = 128;

    char raw_buffer_[INPUT_BUF_SIZE];
    uint buff_index_;
    bool msg_is_malformed_;
    bool new_msg_;
    ParsedUserMsg parsed_msg_;

};
#endif // USER_IO_HANDLER_H
