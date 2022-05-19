#ifndef USER_IO_HANDLER_H
#define USER_IO_HANDLER_H
#include <stdint.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <stdexcept>
#include <cstring>

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
    uint8_t motor_indexes[NUM_BMCS];
    uint8_t duty_cycles[NUM_BMCS];
    // Other useful stuff about the message.
    uint8_t motor_count{0};
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
     * \brief convert \n-terminated string in raw_buffer_ to ParsedUserMsg.
     *        Flag msg_is_malformed_ if there is an issue with the string.
     */
    void parse_msg();


    /**
     * \brief inline. retrieve the new message
     */
    // TODO: does this actually return a reference?
    ParsedUserMsg& get_msg(){return parsed_msg_;}


    /**
     * \brief inline. clear the parsed message
     */
    void clear_msg()
    {new_msg_ = false; buff_index_ = 0; msg_is_malformed_ = false;}


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


    /**
     * \brief run state machine based on provided input.
     */
    void update(void);

    // FIXME: make this private.
    static const uint INPUT_BUF_SIZE = 128;
    char raw_buffer_[INPUT_BUF_SIZE];
    uint buff_index_;

private:
    /**
     * \brief convert input string to cmd.
     */
    Cmd cmd_str_to_cmd(char* cmd_str);

    /**
     * \brief tokenizes string input and puts token ptrs in token_array.
     *        Returns the number of tokens extracted or -1 if too many tokens.
     * \param tokens array of pointers to the start of all tokens.
     * \note will not write more than max_tokens into tokens[].
     */
    int extract_tokens(char input[], const char delimiters[], char* tokens[],
                        size_t max_tokens);


    static const uint MAX_TOKENS = 3;
    bool msg_is_malformed_;
    bool new_msg_;
    ParsedUserMsg parsed_msg_;


};
#endif // USER_IO_HANDLER_H
