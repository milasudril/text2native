# Text2Native -- Text to binary converter

This tool converts formatted numbers delimited by space into native format. Example:

    echo "1 2" | text2native --type=i32 | hd

will output

    00000000  01 00 00 00 02 00 00 00                           |........|
    00000008

on a little-endian machine.
