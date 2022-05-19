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
    // TODO: convert to do-while.
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
    // Null terminate what we received for direct buffer printing.
    raw_buffer_[buff_index_] = '\0';
}


void UserIOHandler::parse_msg()
{
    // March through the char array sequentially while interpretting chunks
    // of it as a command with arguments.

    // Safety guard to avoid parsing an unfinished string.
    if (!new_msg())
        return;

    // TODO: Handle multiple motor indexes that are all the same.
    char* str_chunk;

    uint8_t buff_index = 0;
    char curr_char = raw_buffer_[buff_index];

    // TODO? strip \n from front and back.

    // Strip leading whitespace and other chars that could land in front.
    while (curr_char == ' ' || curr_char == '\n')
    {curr_char = raw_buffer_[++buff_index];}

    // Extract command.
    str_chunk = &raw_buffer_[buff_index];
    while (curr_char != ' ' && curr_char != '\r' && curr_char != '\n')
    {curr_char = raw_buffer_[++buff_index];}
    // Null-terminate this chunk.
    raw_buffer_[buff_index] = '\0';
    // Save cmd.
    parsed_msg_.cmd = cmd_str_to_cmd(str_chunk);

    // CMD Checks.
    Cmd& cmd = parsed_msg_.cmd;
    if (cmd == ERROR)
    {
        msg_is_malformed_ = true;
        return;
    }
    // Bail-early for cmds without additional args.
    if (cmd == IS_BUSY || cmd == HOME_ALL || cmd == HOME_ALL_IN_PLACE)
        return;

    // Strip whitespace between commands. (Skip added null-termination char.)
    curr_char = raw_buffer_[++buff_index];
    while (curr_char == ' ')
    {curr_char = raw_buffer_[++buff_index];}

    // Extract motors of interest.
    parsed_msg_.motor_count = 0;
    for (auto bmc_index=0; bmc_index<NUM_BMCS; ++bmc_index)
    {
        str_chunk = &raw_buffer_[buff_index]; // next start of string chunk.
        while (curr_char != ',' && curr_char != ' ' && curr_char != '\r' &&
               curr_char != '\n')
        {//printf("\'%c\' (%d)\r\n", curr_char, curr_char);
         curr_char = raw_buffer_[++buff_index];} // read until delimiter
        raw_buffer_[buff_index] = '\0'; // Null-terminate this chunk.
        // Convert to int and save. Throw std::invalid_argument on failure.
        parsed_msg_.motor_indexes[bmc_index] = std::stoi(str_chunk);
        parsed_msg_.motor_count += 1;
        // Check if no more motors specified. (Motors are delimited by ',')
        curr_char = raw_buffer_[++buff_index];
        if (curr_char == ' ' || curr_char == '\r' || curr_char == '\n' ||
            curr_char == '\0')
            break;
    }

    printf("Done parsing msg.\r\n");
    return;

    // Bail-early for cmds without additional args.
    if (cmd == HOME)
        return;

    // Strip whitespace between args.
    curr_char = raw_buffer_[++buff_index];
    while (curr_char != ' ');
    {curr_char = raw_buffer_[++buff_index];}

    // extract duty cycles for motors.
    str_chunk = &raw_buffer_[buff_index]; // next start of string chunk.
    for (auto bmc_index=0; bmc_index<NUM_BMCS; ++bmc_index)
    {
        while (curr_char != ',' && curr_char != ' ' && curr_char != '\r' &&
               curr_char != '\n')
        {curr_char = raw_buffer_[++buff_index];} // read until delimiter
        raw_buffer_[buff_index] = '\0'; // Null-terminate this string.
        // Convert to int and save. Throw std::invalid_argument on failure.
        parsed_msg_.duty_cycles[bmc_index] = std::stoi(str_chunk);
        curr_char = raw_buffer_[++buff_index];
        // Check if no more args specified. (Motors args are delimited by ',')
        if (curr_char == ' ' || curr_char == '\r' || curr_char == '\n')
            break;
    }

    // parsed_msg_ is now fully populated at this point.
    msg_is_malformed_ = false;
}


// TODO: consider a std::map with an embedded-friendly implementation.
Cmd UserIOHandler::cmd_str_to_cmd(char* cmd_str)
{
        if (std::strcmp(cmd_str, "IS_BUSY") == 0)
            return IS_BUSY;
        if (std::strcmp(cmd_str, "TIME_MOVE") == 0)
            return TIME_MOVE;
        if (std::strcmp(cmd_str, "DIST_MOVE") ==  0)
            return DIST_MOVE;
        if (std::strcmp(cmd_str, "HOME") == 0)
            return HOME;
        if (std::strcmp(cmd_str, "HOME_IN_PLACE") == 0)
            return HOME_IN_PLACE;
        if (std::strcmp(cmd_str, "HOME_ALL_IN_PLACE") == 0)
            return HOME_ALL_IN_PLACE;
        return ERROR;
}
