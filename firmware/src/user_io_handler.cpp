#include <user_io_handler.h>

UserIOHandler::UserIOHandler()
:new_msg_{false},
 msg_is_malformed_{false},
 parsed_msg_{ParsedUserMsg{}}
{
}

UserIOHandler::~UserIOHandler()
{
}


void UserIOHandler::init()
{
    stdio_usb_init();
    stdio_set_translate_crlf(&stdio_usb, false); // Don't replace outgoing chars.
    while (!stdio_usb_connected()){} // Block until pc connects to serial port.
}


void UserIOHandler::read_chars_nonblocking()
{
    uint new_char = getchar_timeout_us(0);
    while (new_char != PICO_ERROR_TIMEOUT)
    {
        raw_buffer_[buff_index_++] = char(new_char);
        new_char = getchar_timeout_us(0);
    }
    char& last_char = raw_buffer_[buff_index_-1];
    if (last_char == '\r' || last_char == '\n')
    {
        new_msg_ = true;
    }
}


void UserIOHandler::handle_user_input()
{
    // FIXME: Handle multiple motor indexes that are all the same.

    // Assumes we have a '\n'-terminated string in raw_buffer_ already.
    char* str_chunk;

    uint8_t buff_index = 0;
    char curr_char;

    // TODO? strip \n from front and back.
    try
    {
        // Extract command.
        do{curr_char = raw_buffer_[buff_index++];}
        while (curr_char != ' ' || curr_char != '\r' || curr_char != '\n');
        raw_buffer_[buff_index++] = '\0'; // Null-terminate this string.
        parsed_msg_.cmd = cmd_str_to_cmd(raw_buffer_);

        // Checks
        Cmd& cmd = parsed_msg_.cmd;
        if (cmd == ERROR)
            throw std::invalid_argument("Not a valid cmd.");
        // Bail-early for cmds without additional args.
        if (cmd == IS_BUSY || cmd == HOME_ALL || cmd == HOME_ALL_IN_PLACE)
        {
            msg_is_malformed_ = false;
            return;
        }

        // Extract motors of interest.
        str_chunk = &raw_buffer_[buff_index]; // next start of string chunk.
        for (auto bmc_index=0; bmc_index<NUM_BMCS; ++bmc_index)
        {
            do{curr_char = raw_buffer_[buff_index++];} // read until delimiter
            while (curr_char != ',' || curr_char != ' ' || curr_char != '\r' ||
                   curr_char != '\n');
            bool no_more_motors = (curr_char == ' ' || curr_char == '\r' ||
                                   curr_char == '\n');
            str_chunk[buff_index++] = '\0'; // Null-terminate this string.
            // Convert to int. Throwstd::invalid_argument on failure.
            parsed_msg_.motor_indexes[bmc_index] = std::stoi(str_chunk);
            if (no_more_motors)
            {
                parsed_msg_.motor_count = bmc_index + 1;
                break;
            }
        }

        // Bail-early for cmds without additional args.
        if (cmd == HOME)
        {
            msg_is_malformed_ = false;
            return;
        }

        // extract duty cycles for motors.
        str_chunk = &raw_buffer_[buff_index]; // next start of string chunk.
        for (auto bmc_index=0; bmc_index<NUM_BMCS; ++bmc_index)
        {
            do{curr_char = raw_buffer_[buff_index++];} // read until delimiter
            while (curr_char != ',' || curr_char != ' ' || curr_char != '\r' ||
                   curr_char != '\n');
            bool no_more_motor_cmds = (curr_char == ' ' || curr_char == '\r' ||
                                       curr_char == '\n');
            str_chunk[buff_index++] = '\0'; // Null-terminate this string.
            // Convert to int. Throw std::invalid_argument on failure.
            parsed_msg_.duty_cycles[bmc_index] = std::stoi(str_chunk);
            if (no_more_motor_cmds)
                break;
        }
    }
    catch (std::invalid_argument &e)
    {
        msg_is_malformed_ = false;
        printf(e.what());
    }

    // parsed_msg_ is now fully populated at this point.
    msg_is_malformed_ = false;
}


// TODO: consider a std::map with an embedded-friendly implementation.
Cmd UserIOHandler::cmd_str_to_cmd(char* cmd_str)
{
        if (cmd_str == "IS_BUSY")
            return IS_BUSY;
        if (cmd_str == "TIME_MOVE")
            return TIME_MOVE;
        if (cmd_str == "DIST_MOVE")
            return DIST_MOVE;
        if (cmd_str == "HOME")
            return HOME;
        if (cmd_str == "HOME_IN_PLACE")
            return HOME_IN_PLACE;
        if (cmd_str == "HOME_ALL_IN_PLACE")
            return HOME_ALL_IN_PLACE;
        return ERROR;
}
