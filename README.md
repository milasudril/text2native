# Text2Native -- Text to binary converter

This tool converts formatted numbers delimited by space into native format. Example:

    echo "1 2" | text2native --type=i32 | hd

will output

    00000000  01 00 00 00 02 00 00 00                           |........|
    00000008

on a little-endian machine.

There is also a inverse command `natve2text` that does the opposite:

    ffmpeg -i dialog.wav -f f32le - | native2text --locale=C --type=f32

This will dump the content of `dialog.wav` into a list of sample values.
